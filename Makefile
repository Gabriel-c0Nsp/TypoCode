CFLAGS = -lncurses -o
CC = gcc

all: make

make: src/typo.c
	@$(CC) src/typo.c $(CFLAGS) src/typo
	@src/typo

clean:
	rm -f src/typo
