#include "seccomp_filter.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "seccomp.h"
#define NUMBER_OF_BLOCKED_SYSCALLS 3

const char *blocked_syscalls[NUMBER_OF_BLOCKED_SYSCALLS] = { "nfsservctl",
                                                             "personality",
                                                             "pivot_root" };

static void enable_all_syscall(scmp_filter_ctx *ctx)
{
    *ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (*ctx == NULL)
    {
        err(1, "Enable to do seccomp_init");
    }
}

static void disable_blocked_syscalls(scmp_filter_ctx *ctx)
{
    for (size_t i = 0; i < NUMBER_OF_BLOCKED_SYSCALLS; i++)
    {
        int syscall_id = seccomp_syscall_resolve_name(blocked_syscalls[i]);
        if (syscall_id == __NR_SCMP_ERROR)
        {
            seccomp_release(*ctx);
            err(1, "Unable to find syscall id of this: %s\n",
                blocked_syscalls[i]);
        }

        if (seccomp_rule_add(*ctx, SCMP_ACT_ERRNO(EPERM), syscall_id, 0) < 0)
        {
            seccomp_release(*ctx);
            err(1, "Unable to do seccomp_rule_add");
        }
    }
}

static void load_syscall_filter_to_kernel(scmp_filter_ctx *ctx)
{
    if (seccomp_load(*ctx) < 0)
    {
        seccomp_release(*ctx);
        err(1, "Unable to do seccomp_load");
    }
}

void create_seccomp_filter(void)
{
    scmp_filter_ctx ctx;

    enable_all_syscall(&ctx);
    disable_blocked_syscalls(&ctx);
    load_syscall_filter_to_kernel(&ctx);

    seccomp_release(ctx);
}
