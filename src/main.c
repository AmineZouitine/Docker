#include <err.h>
#include <linux/capability.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "capabilities.h"
#include "cgroups.h"
#include "chroot.h"
#include "seccomp.h"
#include "seccomp_filter.h"
#include "oci_json_handler.h"

int main(int argc, char **argv)
{
    get_url_to_image_tarball("library/alpine", "latest");
    if (argc != 3)
        err(1, "Usage: %s <chroot_path> <program_to_run>\n", argv[0]);

    char *new_rootfs = argv[1];
    char *program_to_run = argv[2];

    create_cgroup();

    set_capability(CAP_NET_RAW);

    pid_t pid = fork();
    if (pid < 0)
        err(1, "Failed to fork");
    else if (pid == 0)
    {
        pid_t current_pid = getpid();
        add_process_to_cgroup(current_pid);

        do_chroot(new_rootfs);
        create_seccomp_filter();

        execvp(program_to_run, &program_to_run);
        err(1, "Failed to lauch %s program", program_to_run);
    }

    int status;
    waitpid(pid, &status, 0);
}
