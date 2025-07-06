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
  if (argc == 1) {
    printf("You should specify a file!\n");
    exit(1);
  }

  FILE *file = open_file(argv[1]);

  define_locale(argv[2]);

  Buffer buffer = create_buffer(file);

  initscr();
  cbreak();
  noecho();

  draw_buffer(&buffer);

  while (true) {
    get_user_input(file, &buffer);
  }

  close_file(file);

  return 0;
}

void define_locale(char *locale) {
  if (locale != NULL) {
    setlocale(LC_ALL, locale);

    return;
  }

  setlocale(LC_ALL, "UTF-8");
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
  wchar_t file_char = '0';

  while (file_char != EOF) {
    file_char = getc(file);

    if (file_char != EOF)
      char_number++;
  }

  rewind(file); // reseting file pointer

  return char_number - 1; // ignores unwanted character at the end of the file
}

Buffer create_buffer(FILE *file) {
  int char_number = file_char_number(file); // gets the number of
                                            // characters inside a file
  wchar_t file_char;

  // initializing
  Buffer buffer;
  buffer.current_cu_pointer = 0;
  buffer.size = char_number;
  buffer.vect_buff = calloc(buffer.size + 1, sizeof(wchar_t));

  // alocating the file information inside the buffer vector
  for (int i = 0; i <= buffer.size; i++) {
    file_char = getc(file);
    buffer.vect_buff[i] = file_char;
  }

  return buffer;
}

void draw_buffer(Buffer *buffer) {
  // clear(); // NOTE: comented for debug purposes
  move(0, 0);

  int x_pos = 0;
  int y_pos = 0;

  for (int i = 0; i <= buffer->size; i++) {
    mvaddch(y_pos, x_pos, buffer->vect_buff[i]);

    // incrementing position
    x_pos++;

    if (buffer->vect_buff[i] == '\n') {
      y_pos++;
      x_pos = 0;
    }
  }

  move(y_cursor_pos, x_cursor_pos); // going back to the original position
}

wchar_t get_user_input(FILE *file, Buffer *buffer) {
  wchar_t user_input = getch();

  if (user_input == 27)
    exit_game(0, file, buffer);

  // NOTE: This don't include accentes
  if (user_input >= 32 && user_input <= 125) {
    mvaddch(y_cursor_pos, x_cursor_pos, user_input);
    refresh();
    x_cursor_pos++;
  } else if (user_input == 127) {
    // TODO: Handle user input backspace key
  } else if (user_input == '\n') {
    // TODO: Handle user input '\n' case
  }

  return user_input;
}

void exit_game(int exit_status, FILE *file_path, Buffer *buffer) {
  endwin();
  close_file(file_path);
  free(buffer->vect_buff);

  exit(exit_status);
}
