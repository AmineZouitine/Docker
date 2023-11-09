#pragma once

#include <stddef.h>

struct oci_image 
{
    char *image_name;
    char *tag;
};

struct arguments_datas{
    struct oci_image oci_image;
    size_t program_name_index;
};


struct arguments_datas *get_arguments(char **argv);
