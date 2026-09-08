CC = gcc
CFLAGS = -Wall -Wextra -g
LDLIBS = -lpthread

all: main

main: main.c memory.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
clean:
	rm -f main *.o