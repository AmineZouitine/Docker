CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Werror -Wextra -Wvla -g
LDFLAGS=-lcap -lseccomp -lcurl -ljansson -larchive 

OBJS=$(addprefix src/, io_utils.o json_utils.o curl_utils.o request_informations.o arguments.o cgroups.o main.o capabilities.o rootfs.o seccomp_filter.o oci_json_handler.o)

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
