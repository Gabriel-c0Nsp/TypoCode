#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

// TODO: Implement MakeFile
int main() {
  initscr();

  printw("qualquer coisa pra testar se isso aqui estah funcionando!");

  int y_cursor_pos = 0;
  int x_cursor_pos = 0;
  int i = 0;

  move(y_cursor_pos, x_cursor_pos);

  while (mvinch(y_cursor_pos, x_cursor_pos) == getch()) {
    i++;
    x_cursor_pos++;
    move(y_cursor_pos, x_cursor_pos);
    mvprintw(20, 20, "certo");
    move(y_cursor_pos, x_cursor_pos);
  }

  endwin();

  return 0;
}
