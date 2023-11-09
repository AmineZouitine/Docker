#define _POSIX_C_SOURCE 200809L
#include "json_utils.h"

#include <err.h>
#include <string.h>

json_t *init_json(char *http_response)
{
    json_t *root = NULL;
    json_error_t error;

    root = json_loads(http_response, 0, &error);

    if (!root)
        err(1, "Unable to load JSON : %s", error.text);

    if (!json_is_object(root))
    {
        json_decref(root);
        err(1, "The http response in not a json\n");
    }

    return root;
}

char *get_json_element(json_t *root, char *element_name)
{
    json_t *token = json_object_get(root, element_name);
    if (!json_is_string(token))
    {
        json_decref(root);
        err(1, "The token is not a string");
    }

    const char *element_value = json_string_value(token);
    return strdup(element_value);
}
