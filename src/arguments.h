#pragma once

struct program {
    char *program_name;
    char **program_arguments;
};

struct arguments_datas{
    char *oci_image;
    struct program program;
};


struct arguments_datas *get_arguments(char **argv);
