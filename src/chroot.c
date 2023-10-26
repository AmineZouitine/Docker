#define _GNU_SOURCE
#include "chroot.h"

#include <err.h>
#include <unistd.h>

void do_chroot(const char *path)
{
    if (chroot(path) == -1)
        err(1, "Unable to chroot on this path: %s", path);

    if (chdir("/") == -1)
        err(1, "Unable to chdir in / on this path: %s", path);
}
