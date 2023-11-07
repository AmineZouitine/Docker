#define _POSIX_C_SOURCE 200809L
#include "arguments.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <err.h>

#define USAGE_MESSAGE "Usage: %s <chroot_path> <program_to_run>\n"


static bool is_image_option(char *argument)
{
    return strcmp(argument, "-v") == 0;
}

static struct arguments_datas *init_arguments_datas(void)
{
    struct arguments_datas *arguments_datas = calloc(1, sizeof(struct arguments_datas));

    if (!arguments_datas)
        err(1, "Unable to allocate memory for arguments data");

    return arguments_datas;
}


struct arguments_datas *get_arguments(int argc, char **argv)
{
    struct arguments_datas *arguments_datas = init_arguments_datas();

    bool image_argument_found = false;
    bool program_name_found = false;
    size_t argc = 0;
    for (size_t i = 1; argv[i]; i++)
    {
        if (is_image_option(argv[i]))
        {
            if (!argv[i + 1] || image_argument_found)
            {
                free(arguments_datas);
                err(1, USAGE_MESSAGE, argv[0]);
            }
            arguments_datas->oci_image = argv[i + 1];
            image_argument_found = true;
            i++;
        }
        else
        {
            if (!program_name_found)
            {
                arguments_datas->program.program_name = argv[i];
                program_name_found = true;
            }
            else
                arguments_datas->program.program_arguments[argc++] = argv[i];
        }
    }

    if (!image_argument_found || program_name_found)
    {
        free(arguments_datas);
        err(1, USAGE_MESSAGE, argv[0]);
    }

    return arguments_datas;
}
