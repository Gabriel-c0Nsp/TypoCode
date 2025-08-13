#define NCURSES_WIDECHAR 1

#include <ncurses.h>
#include <string.h>

#include "../file/file.h"
#include "tui.h"

void draw_display_panel() {
  for (int i = 0; i <= COLS; i++) {
    if (i == X_PADDING - 2) {
      display_char(0, i, L'┬', NO_COLOR);
      display_char(LINES - 3, i, L'┼', NO_COLOR);
    } else {
      display_char(0, i, L'─', NO_COLOR);
      display_char(LINES - 3, i, L'─', NO_COLOR);
    }

    if (i == X_PADDING - 2) {
      display_char(2, i, L'┼', NO_COLOR);
      display_char(LINES - 1, i, L'┴', NO_COLOR);
    } else {
      display_char(2, i, L'─', NO_COLOR);
      display_char(LINES - 1, i, L'─', NO_COLOR);
    }
  }

  for (int i = 1; i <= LINES; i++) {
    if (i == 2 || i == LINES - 1 || i == LINES - 3)
      continue;
    display_char(i, X_PADDING - 2, L'│', NO_COLOR);
  }
}

void draw_file_name() {
  attron(BLUE);
  mvaddstr(1, X_PADDING, "File: ");
  attroff(BLUE);
  attron(BOLD);
  mvaddstr(1, X_PADDING + 6, file_info.file_name);
  attroff(BOLD);
}

void draw_number_lines(Buffer *buffer) {
  int current_line = Y_PADDING / 2;
  for (int i = buffer->lines_range.start; i <= buffer->lines_range.end; i++) {
    attron(YELLOW);
    move(current_line, 0);
    current_line++;
    printw("%8d", i);
    attroff(YELLOW);
  }
}

void draw_page_number(Buffer *buffer) {
  int number_size = str_bytes_num(buffer->page_number);
  char current_page_number[number_size];
  sprintf(current_page_number, "%d", buffer->page_number);

  number_size = str_bytes_num(file_info.number_of_buffers);
  char total_page_number[number_size];
  sprintf(total_page_number, "%d", file_info.number_of_buffers);

  attron(BLUE);
  mvaddstr(LINES - 2, X_PADDING, "Page: ");
  attroff(BLUE);
  attron(BOLD);
  mvaddstr(LINES - 2, X_PADDING + 6, current_page_number);
  mvaddstr(LINES - 2, X_PADDING + 6 + strlen(current_page_number), "/");
  mvaddstr(LINES - 2, X_PADDING + 7 + strlen(current_page_number),
           total_page_number);
  attroff(BOLD);
}

void draw_buffer(Buffer *buffer, attr_t attr) {
  clear();
  move(Y_PADDING, X_PADDING);

  int x_pos = X_PADDING;
  int y_pos = Y_PADDING / 2;

  attron(attr);
  for (int i = 0; i < buffer->size; i++) {
    wchar_t file_char = buffer->vect_buff[i];

    cchar_t ch;
    setcchar(&ch, &file_char, 0, 0, NULL);

    mvadd_wch(y_pos, x_pos, &ch);
    x_pos++;

    if (file_char == L'\n') {
      y_pos++;
      x_pos = X_PADDING;
    }
  }
  attroff(attr);

  draw_number_lines(buffer);
  draw_display_panel();
  draw_file_name();
  draw_page_number(buffer);

  y_cursor_pos = Y_PADDING / 2;
  x_cursor_pos = X_PADDING;
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
