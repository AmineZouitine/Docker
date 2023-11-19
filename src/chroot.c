#define _GNU_SOURCE
#include "chroot.h"

#include <err.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "io_utils.h"
#define OLD_ROOT_NAME "old_root"

void do_chroot(const char *path)
{
    if (chroot(path) == -1)
        err(1, "Unable to chroot on this path: %s", path);

    if (chdir("/") == -1)
        err(1, "Unable to chdir in / on this path: %s", path);
}

void do_pivot_root(const char *path)
{
    if (mount(path, path, NULL, MS_BIND | MS_REC, NULL) == -1)
        err(1, "Failed to bind mount new_root");

    if (mount("none", "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1)
        err(1, "Failed to make / private");

    char *old_root_path = create_path(path, OLD_ROOT_NAME);

    mkdir(old_root_path, 0755);

    if (syscall(SYS_pivot_root, path, old_root_path))
        err(1, "pivot_root failed");

    if (chdir("/") == -1)
        err(1, "chdir to new root failed");

    free(old_root_path);

    if (umount2("/old_root", MNT_DETACH) == -1)
        err(1, "Failed to umount old root");

    if (rmdir("/old_root") == -1)
        err(1, "Failed to remove old root directory");
}
