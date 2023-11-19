#define _POSIX_C_SOURCE 200809L
#include "arguments.h"

#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "oci_json_handler.h"

static bool is_image_option(char *argument)
{
    return strcmp(argument, "-I") == 0;
}

static void handle_help_option(int exit_code)
{
    printf("MyMoulette, the students' nightmare, now highly secured\n");
    printf("Usage: ./mymoulette <-I docker-img|rootfs-path> moulette_prog "
           "[moulette_arg [...]]\n");
    printf("rootfs-path is the path to the directory containing the new rootfs "
           "(exclusive with -I option)\n");
    printf("docker-img is an image available on hub.docker.com (exclusive with "
           "rootfs-path)\n");
    printf("moulette_prog will be the first program to be launched, must "
           "already be in the environment\n");
    exit(exit_code);
}
static bool is_help_option(char *argument) {
    return strcmp(argument, "-h") == 0;
}

static char **init_args(void)
{
    char **argv = calloc(1, sizeof(char *));
    if (!argv)
        err(1, "Unable to allocate memory for arguments data");

    argv[0] = NULL;
    return argv;
}

static void add_new_data(char ***argv, char *new_data, size_t *current_size)
{
    (*argv)[*current_size - 1] = new_data;
    char **temp = realloc(*argv, ++(*current_size) * sizeof(char *));
    if (!temp)
    {
        free(*argv);
        err(1, "Unable to reallocate memory for arguments data");
    }
    *argv = temp;
    (*argv)[*current_size - 1] = NULL;
}

static void parse_oci_image(char *oci_image, char ***argv, size_t *current_size)
{
    char *saveptr = NULL;
    char *delim = ":";

    char *image = strtok_r(oci_image, delim, &saveptr);
    char *tag = strtok_r(NULL, delim, &saveptr);

    char *new_rootfs = get_url_to_image_tarball(image, tag);

    add_new_data(argv, new_rootfs, current_size);
}

static void check_help_option(char **argv)
{
    for (size_t i = 1; argv[i]; i++)
    {
        if (is_help_option(argv[i]))
            handle_help_option(0);
    }
}

char **get_arguments(char **argv)
{
    check_help_option(argv);
    char **new_argv = init_args();
    size_t current_size = 1;

    bool image_argument_found = false;
    bool program_name_found = false;

    for (size_t i = 1; argv[i]; i++)
    {
        if (is_image_option(argv[i]))
        {
            if (!argv[i + 1] || image_argument_found)
            {
                free(new_argv);
                handle_help_option(1);
            }
            parse_oci_image(argv[++i], &new_argv, &current_size);
            image_argument_found = true;
        }
        else
        {
            program_name_found = true;
            add_new_data(&new_argv, argv[i], &current_size);
        }
    }

    if (!image_argument_found || !program_name_found)
    {
        free(new_argv);
        handle_help_option(1);
    }

    return new_argv;
}
