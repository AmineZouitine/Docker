#include "cgroups.h"

#include <err.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "io_utils.h"

#define CGROUP_NAME "moulinette"
#define BASE_PATH "/sys/fs/cgroup"

#define MAX_MEMORY 1073741823
#define MEMORY_MAX_FILE_NAME "memory.max"

#define CPUSET_CPUS_FILE_NAME "cpuset.cpus"
#define CPUSET_CPUS_VALUE 0

#define MAX_PID 100
#define PID_MAX_FILE_NAME "pids.max"

#define PROC_FILE_NAME "cgroup.procs"

// Create and mount cgroupv2 if it don't exist
static void mount_cgroupv2_if_needed(void)
{
    if (access(BASE_PATH, F_OK) == -1)
    {
        if (mkdir(BASE_PATH, 0755) == -1)
            err(1, "Unable to create directory at %s", BASE_PATH);
        if (system("mount -t cgroup2 none " BASE_PATH) != 0)
            err(1, "Unable to mount cgroupv2 at %s", BASE_PATH);
    }
}

// set value to specifique cgroup file
static void set_cgroup_attribute(char *cgroup_path, char *attribute_name,
                                 long value)
{
    char *attribute_path = create_path(cgroup_path, attribute_name);
    FILE *file = fopen(attribute_path, "w");
    free(attribute_path);

    if (!file)
    {
        free(cgroup_path);
        err(1, "Unable to set attribute: %s in %s", attribute_name,
            attribute_name);
    }

    fprintf(file, "%ld\n", value);
    fclose(file);
}

void create_cgroup(void)
{
    mount_cgroupv2_if_needed();
    char *cgroup_path = create_path(BASE_PATH, CGROUP_NAME);

    create_directory(cgroup_path);

    set_cgroup_attribute(cgroup_path, MEMORY_MAX_FILE_NAME, MAX_MEMORY);
    set_cgroup_attribute(cgroup_path, CPUSET_CPUS_FILE_NAME, CPUSET_CPUS_VALUE);
    set_cgroup_attribute(cgroup_path, PID_MAX_FILE_NAME, MAX_PID);

    free(cgroup_path);
}

// simple wrapper to set_cgroup_attributee
void add_process_to_cgroup(pid_t pid)
{
    char *cgroup_path = create_path(BASE_PATH, CGROUP_NAME);
    set_cgroup_attribute(cgroup_path, PROC_FILE_NAME, pid);
    free(cgroup_path);
}
