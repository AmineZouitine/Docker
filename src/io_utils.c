#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include "io_utils.h"

#include <archive.h>
#include <archive_entry.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

char *create_tmp_folder(void)
{
    char *template = strdup("/tmp/exampleXXXXXX");

    if (!mkdtemp(template))
        err(1, "Unable to create tmp directory");

    return template;
}

FILE *open_path(char *path)
{
    FILE *fd = fopen(path, "ab");

    if (!fd)
        err(1, "failed to open file");

    printf("New path %s\n", path);
    return fd;
}

char *get_target_command(char *src, char *dest)
{
    size_t new_path_size = strlen("tar xvzf ") + strlen(src) + 1 + strlen("-C")
        + 1 + strlen(dest) + 1;
    char *new_path = malloc(sizeof(char) * new_path_size);

    sprintf(new_path, "tar xf %s -C %s", src, dest);

    return new_path;
}

char *create_path(const char *base_path, const char *expended)
{
    const size_t len_base_path = strlen(base_path);
    const size_t len_expended = strlen(expended);

    const size_t path_memory_size =
        sizeof(char) * len_base_path + len_expended + 2;
    char *path = malloc(path_memory_size);

    snprintf(path, path_memory_size, "%s/%s", base_path, expended);

    return path;
}

bool create_directory(char *directory_path)
{
    struct stat st;

    if (stat(directory_path, &st) == -1)
    {
        if (mkdir(directory_path, 0755) == -1)
        {
            err(1, "Unable to create directory %s", directory_path);
        }
        return false;
    }
    return true;
}

static int copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    while (true)
    {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
        {
            return ARCHIVE_OK;
        }
        if (r < ARCHIVE_OK)
        {
            return r;
        }
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK)
        {
            fprintf(stderr, "%s\n", archive_error_string(aw));
            return r;
        }
    }
}

int extract_tar(const char *tar_path, const char *output_directory)
{
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int result;

    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    a = archive_read_new();
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);
    archive_read_support_format_tar(a);
    archive_read_support_filter_gzip(a);

    if ((result = archive_read_open_filename(a, tar_path, 10240)))
    {
        return result;
    }

    while (true)
    {
        result = archive_read_next_header(a, &entry);
        if (result == ARCHIVE_EOF)
        {
            break;
        }
        if (result < ARCHIVE_OK)
        {
            fprintf(stderr, "%s\n", archive_error_string(a));
        }
        if (result < ARCHIVE_WARN)
        {
            return result;
        }

        const char *currentFile = archive_entry_pathname(entry);
        char *fullPath =
            malloc(strlen(output_directory) + strlen(currentFile) + 2);
        strcpy(fullPath, output_directory);
        strcat(fullPath, "/");
        strcat(fullPath, currentFile);
        archive_entry_set_pathname(entry, fullPath);

        result = archive_write_header(ext, entry);
        if (result < ARCHIVE_OK)
        {
            fprintf(stderr, "%s\n", archive_error_string(ext));
        }
        else if (archive_entry_size(entry) > 0)
        {
            result = copy_data(a, ext);
            if (result < ARCHIVE_OK)
            {
                fprintf(stderr, "%s\n", archive_error_string(ext));
            }
            if (result < ARCHIVE_WARN)
            {
                return result;
            }
        }
        free(fullPath);
        archive_write_finish_entry(ext);
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    return ARCHIVE_OK;
}

void mount_sysfs()
{
    if (mount("sysfs", "/sys", "sysfs", 0, NULL) == -1)
    {
        err(1, "Error mounting sysfs");
    }
}

void mount_tmpfs()
{
    if (mount("tmpfs", "/tmp", "tmpfs", 0, NULL) == -1)
    {
        err(1, "Error mounting tmpfs");
    }
}

void mount_procfs()
{
    if (mount("proc", "/proc", "proc", 0, NULL) == -1)
    {
        err(1, "Error mounting procfs");
    }
}

static void generate_random_hostname(char *hostname, size_t length)
{
    const char *charset =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    srand(time(NULL));
    if (length)
    {
        --length;
        for (size_t n = 0; n < length; n++)
        {
            int key = rand() % (int)(strlen(charset));
            hostname[n] = charset[key];
        }
        hostname[length] = '\0';
    }
}

void set_container_hostname()
{
    char hostname[12];
    generate_random_hostname(hostname, sizeof(hostname));
    sethostname(hostname, strlen(hostname));
}
