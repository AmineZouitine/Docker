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

int main(int argc, char **argv)
{
    if (argc < 2)
        err(1, "Usage: %s <program_to_run> [args...]\n", argv[0]);

    create_cgroup();
    set_capability(CAP_NET_RAW);

    pid_t pid = fork();
    if (pid < 0)
        err(1, "Failed to fork");
    else if (pid == 0)
    {
        pid_t current_pid = getpid();
        add_process_to_cgroup(current_pid);

        execvp(argv[1], &argv[1]);
        err(1, "Failed to lauch %s program", argv[1]);
    }

    int status;
    waitpid(pid, &status, 0);
}
