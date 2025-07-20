#define NCURSES_WIDECHAR 1

#include <locale.h>
#include <ncurses.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

typedef struct Buffer {
  int current_cu_pointer;
  int offset;
  int size;
  wchar_t *vect_buff;
} Buffer;

// log/debug functions
void logtf(const char *fmt, ...);

// file related operations
FILE *open_file(char *argv);
void close_file(FILE *file);

// buffer related operations
int file_char_number(FILE *file);
Buffer create_buffer(FILE *file);
void draw_buffer(Buffer *buffer);
void display_char(int y, int x, wchar_t character, attr_t attr);

// user input related operations
wchar_t get_user_input(FILE *file, Buffer *buffer);
void handle_del_key(FILE *file, Buffer *buffer);
void handle_bs_key(Buffer *buffer);
void handle_enter_key(Buffer *buffer);
void handle_space_key(wchar_t user_input, Buffer *buffer);
void handle_wrong_key(wchar_t user_input, Buffer *buffer);
void handle_right_key(wchar_t user_input, Buffer *buffer);
void handle_input(wchar_t user_input, FILE *file, Buffer *buffer);

void exit_game(int exit_status, FILE *file_path, Buffer *buffer);

// cursor position global variables
int y_cursor_pos = 0;
int x_cursor_pos = 0;

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, ""); // important so the widechar ncurses can work

  if (argc < 2) {
    printf("You should specify a file!\n");
    exit(1);
  }

  FILE *file = open_file(argv[1]);

  initscr();

  if (!has_colors()) {
    endwin();
    fprintf(stderr,
            "Your terminal emulator must support colors to play this game!\n");
    exit(1);
  }

  start_color();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);

  cbreak();
  noecho();

  Buffer buffer = create_buffer(file);

  draw_buffer(&buffer);

  while (1) {
    handle_input(get_user_input(file, &buffer), file, &buffer);
  }

  close_file(file);

  return 0;
}

void logtf(const char *fmt, ...) {
  FILE *log_file = fopen("log.txt", "a");
  if (!log_file) {
    printf(
        "Failed to open log file: log.txt\nAborting for security reasons...\n");
    exit(1);
  }

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  fprintf(log_file, "[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);

  va_list args;
  va_start(args, fmt);
  vfprintf(log_file, fmt, args);
  va_end(args);
  fclose(log_file);
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
    if (file_char == '\t')
      char_number += 2;
    else
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
      if ((wchar_t)file_char == L'\t') {
        buffer.vect_buff[i] = L' ';
        buffer.vect_buff[i + 1] = L' ';
        i++;
      } else {
        buffer.vect_buff[i] = (wchar_t)file_char;
      }
    }
  }
  buffer.vect_buff[buffer.size] = L'\0';

  return buffer;
}

void draw_buffer(Buffer *buffer) {
  // TODO: Make 8 lines padding so it loooks better (4 at the top and 4 at the
  // bottom)
  // TODO: Draw line numbers
  // TODO: Implement pagination
  // NOTE: Remember to use LINES and COLS variables

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

void display_char(int y, int x, wchar_t character, attr_t attr) {
  attron(attr);
  cchar_t display_char;
  setcchar(&display_char, &character, 0, 0, NULL);

  mvadd_wch(y, x, &display_char);
  attroff(attr);

  move(y, x);
  refresh();
}

wchar_t get_user_input(FILE *file, Buffer *buffer) {
  wint_t user_input;
  int result = get_wch(&user_input);

  if (result == ERR) {
    fprintf(stderr, "Some error occurred while typing\n");
    exit_game(1, file, buffer);
  }

  wchar_t result_char = (wchar_t)user_input;
  return result_char;
}

void handle_del_key(FILE *file, Buffer *buffer) { exit_game(0, file, buffer); }

void handle_bs_key(Buffer *buffer) {
  if (buffer->current_cu_pointer || buffer->offset) {
    if (buffer->offset) {
      buffer->offset--;

      x_cursor_pos--;
      if (x_cursor_pos < 0)
        x_cursor_pos = 0;

      display_char(
          y_cursor_pos, x_cursor_pos,
          buffer->vect_buff[buffer->current_cu_pointer + buffer->offset],
          A_NORMAL);

    } else if (!buffer->offset && buffer->current_cu_pointer) {
      int i = buffer->current_cu_pointer;
      int temp_counter = 0;

      do {
        i--;
        temp_counter++;

        if (buffer->vect_buff[i] == L'\n') {
          buffer->current_cu_pointer -= temp_counter;
          int j = buffer->current_cu_pointer - 1;
          x_cursor_pos = 0;

          while (j >= 0 && buffer->vect_buff[j] != L'\n') {
            x_cursor_pos++;
            j--;
          }

          y_cursor_pos--;
          move(y_cursor_pos, x_cursor_pos);

          return;
        }
      } while (buffer->vect_buff[i] == L' ' && buffer->vect_buff[i] != L'\n');

      buffer->current_cu_pointer--;
      x_cursor_pos--;
      if (x_cursor_pos < 0)
        x_cursor_pos = 0;

      display_char(
          y_cursor_pos, x_cursor_pos,
          buffer->vect_buff[buffer->current_cu_pointer + buffer->offset],
          A_NORMAL);
    }
  }
}

void handle_enter_key(Buffer *buffer) {
  wchar_t buffer_cu_char = buffer->vect_buff[buffer->current_cu_pointer];

  if (buffer_cu_char == L'\n' && !buffer->offset) {
    y_cursor_pos++;
    x_cursor_pos = 0;

    do {
      buffer->current_cu_pointer++;
      buffer_cu_char = buffer->vect_buff[buffer->current_cu_pointer];
      if (buffer_cu_char == L' ')
        x_cursor_pos++;
    } while (buffer_cu_char == L' ');

    move(y_cursor_pos, x_cursor_pos);
  } else if (buffer_cu_char != L'\n') {
    display_char(y_cursor_pos, x_cursor_pos, '_', COLOR_PAIR(2));
    buffer->offset++;
    x_cursor_pos++;
    move(y_cursor_pos, x_cursor_pos);
  }
}

void handle_space_key(wchar_t user_input, Buffer *buffer) {
  wchar_t buffer_cu_char = buffer->vect_buff[buffer->current_cu_pointer];

  if (user_input != buffer_cu_char && buffer_cu_char == L'\n') {
    if (!buffer->offset) {
      display_char(y_cursor_pos, x_cursor_pos, '_', COLOR_PAIR(2));
      x_cursor_pos++;
      move(y_cursor_pos, x_cursor_pos);
    }

    buffer->offset++;
    if (buffer->offset > 1) buffer->offset = 1;
  } else if (user_input != buffer_cu_char) {
    display_char(y_cursor_pos, x_cursor_pos, '_', COLOR_PAIR(2));
    buffer->offset++;
    x_cursor_pos++;
    move(y_cursor_pos, x_cursor_pos);
  } else {
    buffer->current_cu_pointer++;
    x_cursor_pos++;
    move(y_cursor_pos, x_cursor_pos);
  }
}

void handle_wrong_key(wchar_t user_input, Buffer *buffer) {
  // FIXME: It should output wrong key when \n is misstyped
  buffer->offset++;

  if (buffer->offset)
    display_char(y_cursor_pos, x_cursor_pos, user_input, COLOR_PAIR(2));
  else
    display_char(y_cursor_pos, x_cursor_pos, user_input, COLOR_PAIR(1));

  x_cursor_pos++;
  move(y_cursor_pos, x_cursor_pos);
}

void handle_right_key(wchar_t user_input, Buffer *buffer) {
  if (buffer->offset)
    display_char(y_cursor_pos, x_cursor_pos, user_input, COLOR_PAIR(2));
  else
    display_char(y_cursor_pos, x_cursor_pos, user_input, COLOR_PAIR(1));

  buffer->current_cu_pointer++;
  x_cursor_pos++;
  move(y_cursor_pos, x_cursor_pos);
}

void handle_input(wchar_t user_input, FILE *file, Buffer *buffer) {
  wchar_t buffer_cu_char = buffer->vect_buff[buffer->current_cu_pointer];

  if (user_input == 27) { // ESC
    handle_del_key(file, buffer);
  } else if (user_input == 127) { // BACKSPACE
    handle_bs_key(buffer);
  } else if (user_input == '\n') {
    handle_enter_key(buffer);
  } else if (user_input == ' ') {
    handle_space_key(user_input, buffer);
  } else if (user_input != buffer_cu_char && buffer_cu_char != L'\n') {
    handle_wrong_key(user_input, buffer);
  } else if (user_input == buffer_cu_char) {
    handle_right_key(user_input, buffer);
  }

  logtf("%d\n", buffer->offset);
  logtf("user_input: %lc\n", user_input);
  logtf("buffer_char: %lc\n", buffer_cu_char);
}

void exit_game(int exit_status, FILE *file_path, Buffer *buffer) {
  endwin();
  close_file(file_path);
  free(buffer->vect_buff);
  exit(exit_status);
}
