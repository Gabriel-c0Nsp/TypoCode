CFLAGS = -lncursesw -lm -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600 -Wall -Wextra -o
CC = gcc

all: make

make: src/typo.c
	@$(CC) src/typo.c $(CFLAGS) src/typo

clean:
	rm -f src/typo
