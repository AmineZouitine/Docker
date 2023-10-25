CC=gcc
CFLAGS=-std=c99 -pedantic -Werror -Wall -Wextra -Wvla -g -fsanitize=address

OBJS=$(addprefix src/, cgroups_manager.o main.o capabilities_manager.o)

BIN=mymoulette

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $^ -o $@ -lcap -fsanitize=address
	sudo setcap 'cap_sys_admin,cap_dac_override,cap_net_raw+ep' mymoulette

run: $(BIN)
	./$(BIN)

fmt:
	clang-format -i src/*.c
	clang-format -i src/*.h
	
clean:
	$(RM) $(OBJS) $(BIN)

.PHONY: clean all
