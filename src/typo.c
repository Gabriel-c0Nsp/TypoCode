#define NCURSES_WIDECHAR 1

#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

typedef struct Buffer {
  int current_cu_pointer;
  int size;
  wchar_t *vect_buff;
} Buffer;

void define_locale(char *locale);

// file related operations
FILE *open_file(char *argv);
void close_file(FILE *file);

// buffer related operations
int file_char_number(FILE *file);
Buffer create_buffer(FILE *file);
void draw_buffer(Buffer *buffer);

wchar_t get_user_input(FILE *file, Buffer *buffer);

void exit_game(int exit_status, FILE *file_path, Buffer *buffer);

// cursor position global variables
int y_cursor_pos = 0;
int x_cursor_pos = 0;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("You should specify a file!\n");
    exit(1);
  }

  FILE *file = open_file(argv[1]);

  if (argc >= 3)
    define_locale(argv[2]);
  else
    define_locale(NULL);

  initscr();
  cbreak();
  noecho();

  setlocale(LC_ALL, ""); // important so the widechar ncurses can work

  Buffer buffer = create_buffer(file);

  draw_buffer(&buffer);

  while (1) {
    get_user_input(file, &buffer);
  }

  close_file(file);

  return 0;
}

void define_locale(char *locale) {
  if (locale) {
    if (setlocale(LC_ALL, locale) == NULL) {
      // using the default locale if locale passed is not valid
      setlocale(LC_ALL, "");
    }
    return;
  }
  setlocale(LC_ALL, "");
}

FILE *open_file(char *argv) {
  FILE *file;

  if ((file = fopen(argv, "r")) == NULL) {
    printf("couldn't open file\nAborting the program...\n");
    exit(1);
  }

  return file;
}

void close_file(FILE *file) { fclose(file); }

int file_char_number(FILE *file) {
  int char_number = 0;
  wint_t file_char;

  while ((file_char = fgetwc(file)) != WEOF) {
    char_number++;
  }

  rewind(file);
  return char_number;
}

Buffer create_buffer(FILE *file) {
  int char_number = file_char_number(file);

  Buffer buffer;
  buffer.current_cu_pointer = 0;
  buffer.size = char_number;
  buffer.vect_buff = calloc(buffer.size + 1, sizeof(wchar_t));

  for (int i = 0; i < buffer.size; i++) {
    wint_t file_char = fgetwc(file);
    if (file_char == WEOF) {
      buffer.vect_buff[i] = L'\0';
    } else {
      buffer.vect_buff[i] = (wchar_t)file_char;
    }
  }
  buffer.vect_buff[buffer.size] = L'\0';

  return buffer;
}

void draw_buffer(Buffer *buffer) {
  clear();
  move(0, 0);

  int x_pos = 0;
  int y_pos = 0;

  for (int i = 0; i < buffer->size; i++) {
    wchar_t file_char = buffer->vect_buff[i];

    cchar_t ch;
    setcchar(&ch, &file_char, 0, 0, NULL);

    mvadd_wch(y_pos, x_pos, &ch);

    x_pos++;

    if (file_char == L'\n') {
      y_pos++;
      x_pos = 0;
    }
  }

  move(y_cursor_pos, x_cursor_pos);
  refresh();
}

wchar_t get_user_input(FILE *file, Buffer *buffer) {
  wint_t user_input;
  int result = get_wch(&user_input);

  if (result == ERR) {
    fprintf(stderr, "Some error occurred while typing\n");
    exit_game(1, file, buffer);
  }

  if (user_input == 27) { // ESC
    exit_game(0, file, buffer);
  }

  wchar_t result_char = (wchar_t)user_input;
  cchar_t display_char;
  setcchar(&display_char, &result_char, 0, 0, NULL);

  mvadd_wch(y_cursor_pos, x_cursor_pos, &display_char);
  refresh();

  // handle_input(); TODO: Implement
  x_cursor_pos++; // only for now

  return result_char;
}

void exit_game(int exit_status, FILE *file_path, Buffer *buffer) {
  endwin();
  close_file(file_path);
  free(buffer->vect_buff);
  exit(exit_status);
}
