#pragma once

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

void free_request_data(struct request_informations *request_informations);
