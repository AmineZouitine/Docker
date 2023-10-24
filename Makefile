CC=gcc
CFLAGS=-std=c99 -pedantic -Werror -Wall -Wextra -Wvla -fsanitize=address -g
LDFLAGS=-fsanitize=address

OBJS=$(addprefix src/, cgroups_manager.o main.o)

BIN=mymoulette

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ -fsanitize=address

run: $(BIN)
	./$(BIN)
	
clean:
	$(RM) $(OBJS) $(BIN)

.PHONY: clean all
