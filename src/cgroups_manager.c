#include "cgroups_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <err.h>
#include <sys/stat.h>
#include <sys/types.h>

#define CGROUP_NAME "moulinette"
#define BASE_PATH "/sys/fs/cgroup"

#define MAX_MEMORY 1073741823
#define MEMORY_MAX_FILE_NAME "memory.max"

#define CPUSET_CPUS_FILE_NAME "cpuset.cpus"
#define CPUSET_CPUS_VALUE 0

#define MAX_PID 100
#define PID_MAX_FILE_NAME "pids.max"


// Create a dynamic allocated path from a base_path and a extended element (could be a directory name or a file name)
static char *create_path(const char *base_path, const char *expented)
{
    const size_t len_cgroup_name = strlen(base_path);
    const size_t len_base_path = strlen(expented);

    const size_t path_memory_size = sizeof(char)* len_base_path + len_cgroup_name + 2;
    char *path = malloc(path_memory_size);

    snprintf(path, path_memory_size, "%s/%s", base_path, expented);

    return path;
}

// Create the cgroup folder in the right path if not exist
static void create_cgroup_directory(char *cgroup_path)
{
    struct stat st;

    if (stat(cgroup_path, &st) == -1)
    {
        if (mkdir(cgroup_path, 0755) == -1)
        {
            free(cgroup_path);
            err(1, "Unable to create directory %s in %s", CGROUP_NAME, cgroup_path);
        }
    }
}

// set value to specifique cgroup file
static void set_cgroup_attribute(char *cgroup_path, char *attribute_name, long value)
{
    char *attribute_path = create_path(cgroup_path, attribute_name);
    FILE *file = fopen(attribute_path, "w");
    free(attribute_path);

    if (!file)
    {
        free(cgroup_path);
        err(1, "Unable to set attribute: %s in %s", attribute_name, attribute_name);
    }
     
    fprintf(file, "%ld\n", value);
    fclose(file);
}


void create_cgroup(void)
{
    char *cgroup_path = create_path(BASE_PATH, CGROUP_NAME);
    printf("%s\n", cgroup_path);

    create_cgroup_directory(cgroup_path);

    set_cgroup_attribute(cgroup_path, MEMORY_MAX_FILE_NAME, MAX_MEMORY);
    set_cgroup_attribute(cgroup_path, CPUSET_CPUS_FILE_NAME, CPUSET_CPUS_VALUE);
    set_cgroup_attribute(cgroup_path, PID_MAX_FILE_NAME, MAX_PID);

    free(cgroup_path);
}

