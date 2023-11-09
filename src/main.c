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
#include "arguments.h"

int main(int argc, char **argv)
{
    struct arguments_datas *arguments_datas = get_arguments(argv);

    char *new_rootfs = get_url_to_image_tarball(arguments_datas->oci_image.image_name, arguments_datas->oci_image.tag);

    printf(new_rootfs);
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

        execvp(argv[arguments_datas->program_name_index], &argv[arguments_datas->program_name_index]);
        err(1, "Failed to lauch %s program", argv[arguments_datas->program_name_index]);
    }

    int status;
    waitpid(pid, &status, 0);
}
