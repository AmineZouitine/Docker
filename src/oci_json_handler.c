#define _POSIX_C_SOURCE 200809L
#include <curl/curl.h>
#include <err.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "curl_utils.h"
#include "io_utils.h"
#include "json_utils.h"
#include "request_informations.h"

#define SIMPLE_ACCEPT_HEADER                                                   \
    "Accept: application/vnd.docker.distribution.manifest.list.v2+json"
#define BASE_MEDIATYPE_ACCEPT_HEADERS "Accept:"
#define BASE_TOKEN_PATH                                                        \
    "https://auth.docker.io/"                                                  \
    "token?service=registry.docker.io&scope=repository:"
#define BASE_MANIFEST_PATH "https://registry-1.docker.io/v2/"
#define BASE_AUTHORIZATION "Authorization: Bearer"
#define OCI_FILE_NAME "oci.tar.gz"
#define ROOTFS "rootfs"

struct request_informations request_informations = { 0 };
char *http_response = NULL;

static void reset_http_response(void)
{
    free(http_response);
    http_response = NULL;
}

void get_docker_authentication_token(CURL *curl, const char *image_name)
{
    char *token_url =
        calloc(strlen(BASE_TOKEN_PATH) + strlen(image_name) + 6, sizeof(char));
    sprintf(token_url, "%s%s:pull", BASE_TOKEN_PATH, image_name);

    request_and_save_response(curl, NULL, token_url);
    free(token_url);

    json_t *root = init_json(http_response);
    char *token = get_json_element(root, "token");

    json_decref(root);
    reset_http_response();
    request_informations.request_datas.token = token;
}

static void get_os_and_arch_manifest(CURL *curl, struct curl_slist **headers,
                                     const char *image_name,
                                     const char *tag_name)
{
    add_authentification_header(headers, &request_informations);
    add_accept_header(headers, &request_informations);

    tag_name = *tag_name == '\0' ? "latest" : tag_name;
    size_t total_size = strlen(BASE_MANIFEST_PATH) + strlen(image_name) + 1
        + strlen("manifests") + 1 + strlen(tag_name) + 1;
    char *manifest_url = calloc(total_size, sizeof(char));
    sprintf(manifest_url, "%s%s/manifests/%s", BASE_MANIFEST_PATH, image_name,
            tag_name);

    request_and_save_response(curl, *headers, manifest_url);
    free(manifest_url);

    json_t *root = init_json(http_response);
    json_t *manifest_list = json_object_get(root, "manifests");

    json_t *manifest_entry;
    size_t index;
    json_array_foreach(manifest_list, index, manifest_entry)
    {
        json_t *platform = json_object_get(manifest_entry, "platform");
        char const *os = json_string_value(json_object_get(platform, "os"));
        char const *architecture =
            json_string_value(json_object_get(platform, "architecture"));

        if (strcmp(os, "linux") == 0 && strcmp(architecture, "amd64") == 0)
        {
            request_informations.request_datas.mediatype =
                get_json_element(manifest_entry, "mediaType");
            request_informations.request_datas.mnfst_dgst =
                get_json_element(manifest_entry, "digest");
            break;
        }
    }

    json_decref(root);
    reset_http_response();
}

static void get_image_manifest(CURL *curl, struct curl_slist **headers,
                               const char *image_name)
{
    add_authentification_header(headers, &request_informations);
    add_accept_header(headers, &request_informations);

    size_t total_size = strlen(BASE_MANIFEST_PATH) + strlen(image_name) + 1
        + strlen("manifests") + 1
        + strlen(request_informations.request_datas.mnfst_dgst) + 1;
    char *manifest_url = calloc(total_size, sizeof(char));
    sprintf(manifest_url, "%s%s/manifests/%s", BASE_MANIFEST_PATH, image_name,
            request_informations.request_datas.mnfst_dgst);

    request_and_save_response(curl, *headers, manifest_url);
    free(manifest_url);

    json_t *root = init_json(http_response);
    json_t *layers = json_object_get(root, "layers");
    json_t *first_layer = json_array_get(layers, 0);

    request_informations.request_datas.layer_digest =
        get_json_element(first_layer, "digest");

    json_decref(root);
    reset_http_response();
    curl_slist_free_all(*headers);
    *headers = NULL;
}

static void download_file(CURL *curl, struct curl_slist **headers,
                          const char *image_name)
{
    add_authentification_header(headers, &request_informations);

    size_t total_size = strlen(BASE_MANIFEST_PATH) + strlen(image_name) + 1
        + strlen("blobs") + 1
        + strlen(request_informations.request_datas.layer_digest) + 1;
    char *download_url = calloc(total_size, sizeof(char));
    sprintf(download_url, "%s%s/blobs/%s", BASE_MANIFEST_PATH, image_name,
            request_informations.request_datas.layer_digest);
    request_and_save_response(curl, *headers, download_url);
    free(download_url);

    reset_http_response();
    curl_slist_free_all(*headers);
    *headers = NULL;
}

static void set_authorization_header(void)
{
    if (!request_informations.request_datas.token)
        err(1, "Token is needed to set autorization");

    size_t base_authorization_path_len = strlen(BASE_AUTHORIZATION);
    size_t token_len = strlen(request_informations.request_datas.token);

    size_t total_size = base_authorization_path_len + token_len + 2;

    request_informations.headers.authorization =
        malloc(sizeof(char *) * total_size);

    if (!request_informations.headers.authorization)
        err(1, "Failed to allocate memory for authorization");

    sprintf(request_informations.headers.authorization, "%s %s",
            BASE_AUTHORIZATION, request_informations.request_datas.token);
}

static void set_simple_accept_header(void)
{
    request_informations.headers.accept = strdup(SIMPLE_ACCEPT_HEADER);
}

static void set_mediatype_accept_header(void)
{
    if (!request_informations.request_datas.mediatype)
        err(1, "Mediatype is needed to set accept mediatype header");

    size_t base_token_accept_path_len = strlen(BASE_AUTHORIZATION);
    size_t token_len = strlen(request_informations.request_datas.mediatype);

    size_t total_size = base_token_accept_path_len + token_len + 2;

    request_informations.headers.accept = malloc(sizeof(char *) * total_size);

    if (!request_informations.headers.accept)
        err(1, "Failed to allocate memory for accept header");

    sprintf(request_informations.headers.accept, "%s %s",
            BASE_MEDIATYPE_ACCEPT_HEADERS,
            request_informations.request_datas.mediatype);
}

char *get_url_to_image_tarball(const char *image_name, const char *tag_name)
{
    CURL *curl = init_curl(&http_response);
    struct curl_slist *headers = NULL;
    get_docker_authentication_token(curl, image_name);
    set_authorization_header();
    set_simple_accept_header();
    get_os_and_arch_manifest(curl, &headers, image_name, tag_name);
    free(request_informations.headers.accept);
    set_mediatype_accept_header();
    get_image_manifest(curl, &headers, image_name);
    curl_easy_cleanup(curl);

    char *tmp_folder_path = create_tmp_folder();
    char *download_path = create_path(tmp_folder_path, OCI_FILE_NAME);
    FILE *new_dir = open_path(download_path, "wb");
    curl = curl_download_setup(new_dir);
    download_file(curl, &headers, image_name);
    char *rootfs_directory_path = create_path(tmp_folder_path, ROOTFS);
    create_directory(rootfs_directory_path);

    extract_tar(download_path, rootfs_directory_path);

    fclose(new_dir);
    free_request_data(&request_informations);
    free(tmp_folder_path);
    return rootfs_directory_path;
}
