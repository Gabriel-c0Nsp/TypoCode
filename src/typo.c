#define NCURSES_WIDECHAR 1

#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>

#include "endgame/endgame.h"
#include "file/file.h"
#include "input/input.h"
#include "tui/tui.h"

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, ""); // important so the widechar ncurses can work

  if (argc < 2) {
    fprintf(stderr, "You should specify a file!\n");
    exit(1);
  }

  char *file_name = argv[1];
  file_name = extract_file_name(file_name);

  FILE *file = open_file(argv[1]);

  initscr(); // initialize a window

  if (!has_colors()) {
    endwin();
    fprintf(stderr,
            "Your terminal emulator must support colors to play this game!\n");
    exit_game(1, file, NULL);
  }

  start_color();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);

  cbreak();
  noecho();

  file_info = get_file_information(&file_info, file, file_name);
  NodeBuffer *pages = NULL;
  set_pages(&pages, file);

  if (pages == NULL) {
    endwin();
    fprintf(stderr, "Something went wrong while allocating memory for "
                    "buffers!\nAborting...\n");
    exit_game(1, file, &pages);
  }

  draw_buffer(pages->buffer, NO_COLOR);
  while (1) {
    handle_input(get_user_input(file, &pages), file, &pages);
  }

  close_file(file);

  return 0;
}
