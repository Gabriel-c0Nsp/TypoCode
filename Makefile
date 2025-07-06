CFLAGS = -lncursesw -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600 -Wall -Wextra -o
CC = gcc

all: make

make: src/typo.c
	@$(CC) src/typo.c $(CFLAGS) src/typo
	@src/typo

file1: src/typo.c
	@$(CC) src/typo.c $(CFLAGS) src/typo
	@src/typo src/ArquivoTeste.java pt_BR.UTF-8

file2: src/typo.c
	@$(CC) src/typo.c $(CFLAGS) src/typo
	@src/typo src/arquivo_teste.lua

clean:
	rm -f src/typo
