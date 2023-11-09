#pragma once

#include <jansson.h>

json_t *init_json(char *http_response);

char *get_json_element(json_t *root, char *element_name);
