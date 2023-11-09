#define _POSIX_C_SOURCE 200809L
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <jansson.h>

#define SIMPLE_ACCEPT_HEADER "Accept: application/vnd.docker.distribution.manifest.list.v2+json"
#define BASE_MEDIATYPE_ACCEPT_HEADERS "Accept:"
#define BASE_TOKEN_PATH "https://auth.docker.io/token?service=registry.docker.io&scope=repository:"
#define BASE_MANIFEST_PATH "https://registry-1.docker.io/v2/"
#define BASE_AUTHORIZATION "Authorization: Bearer"

struct request_headers
{
    char *authorization;
    char *accept;
};

struct request_datas
{
    char *token;
    char *mediatype;
    char *mnfst_dgst;
    char *cnf_digest;
    char *layer_digest;
};

struct request_informations
{
    struct request_headers headers;
    struct request_datas request_datas;
};

struct request_informations request_informations = {0};

char *http_response = NULL;


static void free_request_data(void){
    free(request_informations.request_datas.token);
    free(request_informations.request_datas.mediatype);
    free(request_informations.request_datas.mnfst_dgst);
    free(request_informations.request_datas.cnf_digest);
    free(request_informations.request_datas.layer_digest);
    free(request_informations.headers.authorization);
    free(request_informations.headers.accept);
}

static void reset_http_response(void)
{
    free(http_response);
    http_response = NULL;
}

static void add_authentification_header(struct curl_slist **headers)
{
    *headers = curl_slist_append(*headers, request_informations.headers.authorization);
}

static void add_accept_header(struct curl_slist **headers)
{
    *headers = curl_slist_append(*headers, request_informations.headers.accept);
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    char **response_ptr = (char **)userp;
    size_t current_size = *response_ptr ? strlen(*response_ptr) : 0;

    char *new_str = realloc(*response_ptr, current_size + real_size + 1);
    if (new_str == NULL) {
        return 0;
    }

    memcpy(new_str + current_size, contents, real_size);
    new_str[current_size + real_size] = '\0';

    *response_ptr = new_str;

    return real_size;
}

static void request_and_save_response(CURL *curl, struct curl_slist *headers, char *url)
{
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    if (headers)
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        free(url);
        err(1, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
}

static json_t *init_json()
{
    json_t *root = NULL;
    json_error_t error;

    root = json_loads(http_response, 0, &error);

    if (!root)
        err(1, "Unable to load JSON : %s", error.text);

    if (!json_is_object(root)) {
        json_decref(root);
        err(1, "The http response in not a json\n");
    }

    return root;
}

static char *get_json_element(json_t *root, char *element_name)
{
    json_t *token = json_object_get(root, element_name);
    if (!json_is_string(token)) {
        json_decref(root);
        err(1, "The token is not a string");
    }

    const char *element_value = json_string_value(token);
    return strdup(element_value);
}


void get_docker_authentication_token(CURL *curl, const char *image_name)
{
    char *token_url = calloc(strlen(BASE_TOKEN_PATH) + strlen(image_name) + 6, sizeof(char));
    sprintf(token_url, "%s%s:pull", BASE_TOKEN_PATH, image_name);

    request_and_save_response(curl, NULL, token_url);
    free(token_url);


    json_t *root = init_json();
    char *token = get_json_element(root, "token");

    json_decref(root);
    reset_http_response();
    request_informations.request_datas.token = token;
}


static void get_os_and_arch_manifest(CURL *curl, struct curl_slist **headers, const char *image_name, const char *tag_name)
{
    add_authentification_header(headers);
    add_accept_header(headers);

    size_t total_size = strlen(BASE_MANIFEST_PATH) + strlen(image_name) + 1 + strlen("manifests") + 1 + strlen(tag_name) + 1;
    char *manifest_url = calloc(total_size, sizeof(char));
    sprintf(manifest_url, "%s%s/manifests/%s", BASE_MANIFEST_PATH, image_name, tag_name);

    request_and_save_response(curl, *headers, manifest_url);
    free(manifest_url);


    json_t *root = init_json();
    json_t *manifest_list = json_object_get(root, "manifests");

    json_t *manifest_entry;
    size_t index;
    json_array_foreach(manifest_list, index, manifest_entry) {
        json_t *platform = json_object_get(manifest_entry, "platform");
        char const *os = json_string_value(json_object_get(platform, "os"));
        char const *architecture = json_string_value(json_object_get(platform, "architecture"));

        if (strcmp(os, "linux") == 0 && strcmp(architecture, "amd64") == 0) {
            request_informations.request_datas.mediatype = get_json_element(manifest_entry, "mediaType");
            request_informations.request_datas.mnfst_dgst = get_json_element(manifest_entry, "digest");
            break;
        }
    }

    json_decref(root);
    reset_http_response();
}


static void get_image_manifest(CURL *curl, struct curl_slist **headers, const char *image_name)
{
    add_authentification_header(headers);
    add_accept_header(headers);

    size_t total_size = strlen(BASE_MANIFEST_PATH) + strlen(image_name) + 1 + strlen("manifests") + 1 + strlen(request_informations.request_datas.mnfst_dgst) + 1;
    char *manifest_url = calloc(total_size, sizeof(char));
    sprintf(manifest_url, "%s%s/manifests/%s", BASE_MANIFEST_PATH, image_name, request_informations.request_datas.mnfst_dgst);

    request_and_save_response(curl, *headers, manifest_url);
    free(manifest_url);


    json_t *root = init_json();
    json_t *layers = json_object_get(root, "layers"); 
    json_t *first_layer = json_array_get(layers, 0);


    request_informations.request_datas.layer_digest = get_json_element(first_layer, "digest");


    json_decref(root);
    reset_http_response();
    curl_slist_free_all(*headers);
    *headers = NULL;
}


static void download_file(CURL *curl, struct curl_slist **headers, const char *image_name)
{
    add_authentification_header(headers);

    size_t total_size = strlen(BASE_MANIFEST_PATH) + strlen(image_name) + 1 + strlen("blobs") + 1 + strlen(request_informations.request_datas.layer_digest) + 1;
    char *download_url = calloc(total_size, sizeof(char));
    sprintf(download_url, "%s%s/blobs/%s", BASE_MANIFEST_PATH, image_name, request_informations.request_datas.layer_digest);
    request_and_save_response(curl, *headers, download_url);
    free(download_url);

    reset_http_response();
    curl_slist_free_all(*headers);
    *headers = NULL;
}


static CURL *init_curl(void)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        err(1, "Unable to init curl");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    return curl;
}

static CURL *curl_download_setup(FILE* fd)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        err(1, "Unable to init curl");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    return curl;
}

static void set_authorization_header(void)
{
    if (!request_informations.request_datas.token)
        err(1, "Token is needed to set autorization");

    size_t base_authorization_path_len = strlen(BASE_AUTHORIZATION);
    size_t token_len = strlen(request_informations.request_datas.token);

    size_t total_size = base_authorization_path_len + token_len + 2;

    request_informations.headers.authorization = malloc(sizeof(char *) * total_size);

    if (!request_informations.headers.authorization)
        err(1, "Failed to allocate memory for authorization");

    sprintf(request_informations.headers.authorization, "%s %s", BASE_AUTHORIZATION, request_informations.request_datas.token);

}

static void set_simple_accept_header(void)
{
    request_informations.headers.accept = strdup(SIMPLE_ACCEPT_HEADER);
}

static void set_mediatype_accept_header(void)
{
    if (!request_informations.request_datas.mediatype)
        err(1, "Token is needed to set accept token header");

    size_t base_token_accept_path_len = strlen(BASE_AUTHORIZATION);
    size_t token_len = strlen(request_informations.request_datas.mediatype);

    size_t total_size = base_token_accept_path_len + token_len + 2;

    request_informations.headers.accept = malloc(sizeof(char *) * total_size);

    if (!request_informations.headers.accept)
        err(1, "Failed to allocate memory for accept header");

    sprintf(request_informations.headers.accept, "%s %s", BASE_MEDIATYPE_ACCEPT_HEADERS, request_informations.request_datas.mediatype);
}

static char *get_random_path_folder(void)
{
    char template[] = "/tmp/exampleXXXXXX";

    if (!mkdtemp(template))
        err(1, "Unable to create tmp directory");
    
    size_t new_path_size = strlen(template) + strlen("layer.tar.gz") + 2;
    char *new_path = malloc(sizeof(char) * new_path_size);

    sprintf(new_path, "%s/%s", template, "layer.tar.gz");

    return new_path;
}


static FILE *open_path(char *path)
{
    FILE *fd = fopen(path, "ab");

    if (!fd)
        err(1, "failed to open file");

    printf("New path %s\n", path);
    free(path);

    return fd;
}

static char *get_target_command(char *src, char *dest)
{
    size_t new_path_size = strlen("tar xvzf ") + strlen(src) + 1 + strlen("-C") + 1 + strlen(dest) + 1;
    char *new_path = malloc(sizeof(char) * new_path_size);

    sprintf(new_path, "tar xvzf %s -C %s", src, dest);

    return new_path;
}

char *get_url_to_image_tarball(const char *image_name, const char *tag_name)
{
    CURL *curl = init_curl();
    struct curl_slist *headers = NULL;
    get_docker_authentication_token(curl, image_name);
    set_authorization_header();
    set_simple_accept_header();
    get_os_and_arch_manifest(curl, &headers, image_name, tag_name);
    free(request_informations.headers.accept);
    set_mediatype_accept_header();
    get_image_manifest(curl, &headers, image_name);
    curl_easy_cleanup(curl);

    char *path_new_dir = get_random_path_folder();
    FILE *new_dir = open_path(path_new_dir);

    curl = curl_download_setup(new_dir);

    download_file(curl, &headers, image_name);

    char *new_rootfs = get_random_path_folder();
    
    char *tar_command = get_target_command(path_new_dir, new_rootfs);
    system(tar_command);
    free(tar_command);
    fclose(new_dir);
    free_request_data();

    return new_rootfs;
}
