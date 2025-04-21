#define NCURSES_WIDECHAR 1

#include <curses.h>
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

FILE *read_file(char *argv);
void close_file(FILE *file_path);

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "pt_BR");

  // TODO: Try this one later
  // setlocale(LC_ALL, "UTF-8");

  FILE *file_path = read_file(argv[1]);

  initscr();
  cbreak();
  noecho();

  getch();
  endwin();
  close_file(file_path);

  return 0;
}

FILE *read_file(char *argv) {
  FILE *file_path;

  if ((file_path = fopen(argv, "r")) == NULL) {
    printf("Não foi posspivel abrir o arquivo!\n");
    exit(1);
  }

  return file_path;
}

void close_file(FILE *file_path) {
  fclose(file_path);
  printf("file closed\n");
}
