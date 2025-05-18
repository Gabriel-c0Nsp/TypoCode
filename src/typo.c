#define NCURSES_WIDECHAR 1

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <locale.h>
#include <ncurses.h>
#include <wchar.h>

typedef struct Buffer {
  int current_cu_pointer;
  wchar_t *vect_buff;
} Buffer;

FILE *open_file(char *argv);
void close_file(FILE *file_path);

void store_file_buffer(Buffer buffer, FILE file_path);

void display_buffer(Buffer buffer);

int y_cursor_pos = 0;
int x_cursor_pos = 0;

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "pt_BR");

  FILE *file_path = open_file(argv[1]);

  // TODO: Create a buffer
  store_file_buffer(&buffer, file_path);

  initscr();
  cbreak();
  noecho();

  display_buffer(buffer);

  getch();
  endwin();
  close_file(file_path);

  return 0;
}

FILE *open_file(char *argv) {
  FILE *file_path;

  if ((file_path = fopen(argv, "r")) == NULL) {
    printf("Não foi posspivel abrir o arquivo!\n");
    exit(1);
  }

  return file_path;
}

void close_file(FILE *file_path) { fclose(file_path); }

void store_file_buffer(Buffer buffer, FILE file_path) {
  // TODO: transverse through a file and store it in a buffer
}

void display_buffer(Buffer buffer) {
  // TODO: loop over the buffer vector and display it on the screen
}
