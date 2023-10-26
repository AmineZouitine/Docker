CC=gcc
CFLAGS=-std=c99 -pedantic -Werror -Wall -Wextra -Wvla -g -fsanitize=address
LDFLAGS=-fsanitize=address -lcap -lseccomp

OBJS=$(addprefix src/, cgroups.o main.o capabilities.o chroot.o seccomp_filter.o)

BIN=mymoulette

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) 
	sudo setcap 'cap_sys_chroot,cap_sys_admin,cap_dac_override,cap_net_raw+ep' mymoulette

run: $(BIN)
	./$(BIN)

fmt:
	clang-format -i src/*.c
	clang-format -i src/*.h
	
clean:
	$(RM) $(OBJS) $(BIN)

.PHONY: clean all
