#include "namespace.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "err.h"
#include "io_utils.h"

#define PROC_PATH "/proc"
#define UID_PATH "uid_map"
#define GID_PATH "gid_map"
#define SETGROUPS_PATH "setgroups"
#define SETGROUPS_VALUE "deny"
#define ROOT_ID "0 1000 1"
#define DEFAULT_ID 0

void reset_uid_gid(void)
{
    if (setgid(DEFAULT_ID) == -1)
        err(1, "setgid failed");
    if (setuid(DEFAULT_ID) == -1)
        err(1, "setuid failed");
}

static char *int_to_string(int number)
{
    int length = snprintf(NULL, 0, "%d", number) + 1;
    char *str = malloc(length);
    if (!str)
    {
        err(1, "Unable to allocate memory");
    }

    snprintf(str, length, "%d", number);
    return str;
}

static void write_mapping(char *path, const char *mapping)
{
    FILE *file = open_path(path, "w");

    fprintf(file, "%s", mapping);
    if (fclose(file) == -1)
    {
        err(1, "Failed to close path %s", path);
    }
}

static char *create_id_value(int value)
{
    char *value_str = int_to_string(value);

    char *id = calloc(1, sizeof(char) * (strlen(value_str) + 5));

    sprintf(id, "0 %s 1", value_str);

    free(value_str);
    return id;
}

void setup_uid_gid_mappind(int uid, int gid)
{
    char *default_path = create_path(PROC_PATH, "self");

    char *uid_path = create_path(default_path, UID_PATH);

    char *uid_value = create_id_value(uid);
    write_mapping(uid_path, uid_value);
    free(uid_value);

    char *setgroups_path = create_path(default_path, SETGROUPS_PATH);
    write_mapping(setgroups_path, SETGROUPS_VALUE);

    char *gid_path = create_path(default_path, GID_PATH);
    char *gid_value = create_id_value(gid);
    write_mapping(gid_path, gid_value);
    free(gid_value);

    free(gid_path);
    free(default_path);
    free(uid_path);
    free(setgroups_path);
}
