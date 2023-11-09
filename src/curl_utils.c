#include "curl_utils.h"

#include <err.h>
#include <stdlib.h>
#include <string.h>

void add_authentification_header(
    struct curl_slist **headers,
    struct request_informations *request_informations)
{
    *headers = curl_slist_append(*headers,
                                 request_informations->headers.authorization);
}

void add_accept_header(struct curl_slist **headers,
                       struct request_informations *request_informations)
{
    *headers =
        curl_slist_append(*headers, request_informations->headers.accept);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t real_size = size * nmemb;
    char **response_ptr = (char **)userp;
    size_t current_size = *response_ptr ? strlen(*response_ptr) : 0;

    char *new_str = realloc(*response_ptr, current_size + real_size + 1);
    if (new_str == NULL)
    {
        return 0;
    }

    memcpy(new_str + current_size, contents, real_size);
    new_str[current_size + real_size] = '\0';

    *response_ptr = new_str;

    return real_size;
}

void request_and_save_response(CURL *curl, struct curl_slist *headers,
                               char *url)
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

CURL *init_curl(char **http_response)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        err(1, "Unable to init curl");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, http_response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    return curl;
}

CURL *curl_download_setup(FILE *fd)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        err(1, "Unable to init curl");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    return curl;
}
