CC = gcc
CFLAGS = -Wall -Wextra -lncursesw -lm -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600
INCLUDE_DIRS = -Isrc/buffer -Isrc/endgame -Isrc/file -Isrc/gamestate -Isrc/input -Isrc/log -Isrc/timer -Isrc/tui -Isrc/args

OBJ = src/args/args.o src/buffer/buffer.o src/endgame/endgame.o src/file/file.o src/gamestate/gamestate.o \
      src/input/input.o src/log/log.o src/timer/timer.o src/tui/tui.o src/typo.o

BIN = src/typo

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(CFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

install: all
	sudo cp src/typo /usr/local/bin

# clean (objects)
clean:
	rm -f $(OBJ)

# clean (objects + binary)
distclean: clean
	rm -f $(BIN)
