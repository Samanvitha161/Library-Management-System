CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2

all: library.exe

library.exe: library.c
	$(CC) $(CFLAGS) library.c -o library.exe

clean:
	rm -f library.exe

run: library.exe
	./library.exe

.PHONY: all clean run
