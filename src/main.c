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

int main(__attribute__((unused)) int argc, char **argv)
{
    struct arguments_datas *arguments_datas = get_arguments(argv);
    printf("%s\n", arguments_datas->oci_image);
    return 0;
    get_url_to_image_tarball("library/alpine", "latest");
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
