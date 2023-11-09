#pragma once

#include <curl/curl.h>

#include "request_informations.h"

void add_authentification_header(
    struct curl_slist **headers,
    struct request_informations *request_informations);

void add_accept_header(struct curl_slist **headers,
                       struct request_informations *request_informations);

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);

void request_and_save_response(CURL *curl, struct curl_slist *headers,
                               char *url);

CURL *init_curl(char **http_response);

CURL *curl_download_setup(FILE *fd);
