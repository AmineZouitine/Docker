#include "capabilities_manager.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <sys/prctl.h>

void set_capability(cap_value_t cap)
{
    cap_t caps = cap_get_proc();
    if (!caps)
        err(1, "Failed to get capabilities");

    if (cap_set_flag(caps, CAP_INHERITABLE, 1, &cap, CAP_SET) == -1
        || cap_set_flag(caps, CAP_PERMITTED, 1, &cap, CAP_SET) == -1)
    {
        cap_free(caps);
        err(1, "Failed to set capability");
    }

    if (cap_set_proc(caps) == -1)
    {
        cap_free(caps);
        err(1, "Failed to set capabilities");
    }

    prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0);
    prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, CAP_NET_RAW, 0, 0);

    cap_free(caps);
}
