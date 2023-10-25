#pragma once

#include <sys/types.h>

void create_cgroup(void);
void add_process_to_cgroup(pid_t pid);
