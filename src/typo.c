#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

int main() {
  initscr();
  cbreak();
  noecho();

  int number_of_lines = 9;

  int y_cursor_pos = 0;
  int x_cursor_pos = 0;

  int y_padding = 3;
  int x_padding = 3;

  int y, x;

  mvaddstr(0, x_padding, "qualquer coisa pra testar se isso aqui funciona!");

  move(y_cursor_pos, x_cursor_pos);

  for (int i = 0; i <= number_of_lines; i++) {
    mvprintw(i, 0, "%d", i);
  }

  move(0, x_padding);

  while (true) {
    char input = getch();

    char value = mvinch(y_cursor_pos, x_cursor_pos + x_padding) & A_CHARTEXT;

    addch(input);

    // Debug purposes
    mvaddstr(22, 20, &input);
    mvaddstr(23, 20, &value);

    move(y_cursor_pos, x_cursor_pos + x_padding);

    if (input != value) {
      mvaddstr(20, 20, "errado");
      move(y_cursor_pos, x_cursor_pos + x_padding);
    } else if (input == value) {
      mvaddstr(20, 20, "certo");
      move(y_cursor_pos, x_cursor_pos + x_padding);
    }

    x_cursor_pos++;
    move(y_cursor_pos, x_cursor_pos + x_padding);

    if (input == '0')
      break;
  }

  endwin();

  return 0;
}
