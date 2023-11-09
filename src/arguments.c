#define _POSIX_C_SOURCE 200809L
#include "arguments.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <err.h>

#define USAGE_MESSAGE "Usage: %s <chroot_path> <program_to_run>\n"


static bool is_image_option(char *argument)
{
    return strcmp(argument, "-I") == 0;
}

static struct arguments_datas *init_arguments_datas(void)
{
    struct arguments_datas *arguments_datas = calloc(1, sizeof(struct arguments_datas));

    if (!arguments_datas)
        err(1, "Unable to allocate memory for arguments data");

    return arguments_datas;
}

static void parse_oci_image(char *oci_image, struct arguments_datas *arguments_datas)
{
    char *saveptr = NULL;
    char delim = ":";

    char *token = strtok_r(oci_image, delim, &saveptr);

    arguments_datas->oci_image.image_name = token;
 

    token = strtok_r(NULL, delim, &saveptr);

    arguments_datas->oci_image.tag = !token ? "" : token;
}


static struct arguments_datas *get_arguments(char **argv)
{

    struct arguments_datas *arguments_datas = init_arguments_datas();
    bool image_argument_found = false;
    bool program_name_found = false;

    for (size_t i = 1; argv[i]; i++)
    {
        if (is_image_option(argv[i]))
        {
            if (!argv[i + 1] || image_argument_found)
            {
                free(arguments_datas);
                err(1, USAGE_MESSAGE, argv[0]);
            }
            parse_oci_image(argv[i++], arguments_datas);
            image_argument_found = true;
        }
        else
        {
            arguments_datas->program_name_index = i;
            program_name_found = true;
            break;
        }
    }

    if (!image_argument_found || !program_name_found)
    {
        free(arguments_datas);
        err(1, USAGE_MESSAGE, argv[0]);
    }

    return arguments_datas;
}  
