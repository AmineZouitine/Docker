#define _GNU_SOURCE
#include <err.h>
#include <linux/capability.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "arguments.h"
#include "capabilities.h"
#include "cgroups.h"
#include "chroot.h"
#include "oci_json_handler.h"
#include "seccomp.h"
#include "seccomp_filter.h"
#include "io_utils.h"
#include <sched.h>


#define STACK_SIZE (1024 * 1024)

int child_func(void *arg) {
    char **argv = (char **)arg;

    do_chroot(argv[0]);

    mount_procfs();
    mount_sysfs();
    mount_tmpfs();
    create_seccomp_filter();
    set_container_hostname();

    execvp(argv[1],
           &argv[1]);
    err(1, "Failed to launch program");
}

int main(__attribute__((unused)) int argc, char **argv) {

    char **new_argv = get_arguments(argv);

    create_cgroup();
    set_capability(CAP_NET_RAW);

    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        err(1, "Failed to allocate stack for the cloned process");
    }

    int clone_flags = CLONE_NEWCGROUP | CLONE_NEWIPC | CLONE_NEWNS | 
                      CLONE_NEWNET | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD;
    pid_t pid = clone(child_func, stack + STACK_SIZE, clone_flags, new_argv);
    if (pid == -1) {
        err(1, "Failed to clone");
    }

    add_process_to_cgroup(pid);

    int status;
    waitpid(pid, &status, 0);
    free(stack);
}
