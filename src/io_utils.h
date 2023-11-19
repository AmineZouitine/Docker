#pragma once

#include <stdbool.h>
#include <stdio.h>

char *create_tmp_folder(void);

FILE *open_path(char *path);

char *get_target_command(char *src, char *dest);

char *create_path(const char *base_path, const char *expended);

bool create_directory(char *directory_path);

int extract_tar(const char *tar_path, const char *output_directory);

void mount_sysfs(void);

void mount_tmpfs(void);

void mount_procfs(void);

void set_container_hostname();
