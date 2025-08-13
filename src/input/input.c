#define NCURSES_WIDECHAR 1

#include <ncurses.h>

#include "../endgame/endgame.h"
#include "../timer/timer.h"
#include "../tui/tui.h"
#include "input.h"

wchar_t get_user_input(FILE *file, NodeBuffer **pages) {
  wint_t user_input;
  int result = get_wch(&user_input);

  if (result == ERR) {
    endwin();
    fprintf(stderr, "Some error occurred while typing\n");
    exit_game(1, file, pages);
  }

  wchar_t result_char = (wchar_t)user_input;
  return result_char;
}

void handle_del_key(FILE *file, NodeBuffer **pages) {
  exit_game(0, file, pages);
}

// clean up this function later
void handle_bs_key(NodeBuffer **pages) {
  wchar_t buffer_cu_char =
      (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer];

  // ignore tabs when deleting the first characters
  if ((*pages)->buffer->page_number > 1 && y_cursor_pos == Y_PADDING / 2) {
    int i = (*pages)->buffer->current_cu_pointer - 1;
    if (i < 0)
      i = 0;

    while ((i && (*pages)->buffer->vect_buff[i] == L' ') ||
           (i && (*pages)->buffer->vect_buff[i] == L'\n'))
      i--;

    if (!i) {
      previous_buffer(pages);

      (*pages)->buffer->current_cu_pointer = (*pages)->buffer->size;
      (*pages)->buffer->offset = 0;

      draw_buffer((*pages)->buffer, GREEN);

      y_cursor_pos = Y_PADDING / 2;
      x_cursor_pos = X_PADDING;

      // find the last cursor position in terms of x
      for (int i = 0; i < (*pages)->buffer->size; i++) {
        if ((*pages)->buffer->vect_buff[i] == L'\n' &&
            (*pages)->buffer->vect_buff[i + 1] != L'\0') {
          y_cursor_pos++;
          x_cursor_pos = X_PADDING;
        } else {
          x_cursor_pos++;
        }
      }

      x_cursor_pos--;
      (*pages)->buffer->current_cu_pointer--;

      // position the cursor at the end of the previous buffer
      move(y_cursor_pos, x_cursor_pos);

      return;
    }
  }

  if ((*pages)->buffer->current_cu_pointer || (*pages)->buffer->offset) {
    if ((*pages)->buffer->offset) {
      (*pages)->buffer->offset--;

      // deletes all extra characters after the end of a line
      if (buffer_cu_char == L'\n') {
        display_char(y_cursor_pos, x_cursor_pos, L' ', NO_COLOR);
        x_cursor_pos--;
        display_char(y_cursor_pos, x_cursor_pos, L' ', NO_COLOR);
      } else {

        x_cursor_pos--;
        if (x_cursor_pos < X_PADDING)
          x_cursor_pos = X_PADDING;

        display_char(
            y_cursor_pos, x_cursor_pos,
            (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer +
                                        (*pages)->buffer->offset],
            NO_COLOR);
      }
    } else if (!(*pages)->buffer->offset &&
               (*pages)->buffer->current_cu_pointer) {
      int i = (*pages)->buffer->current_cu_pointer;
      int temp_counter = 0;

      do { // handles backspacing into the previous last line character without
           // deleting extra spaces
        i--;
        temp_counter++;

        if ((*pages)->buffer->vect_buff[i] == L'\n') {
          (*pages)->buffer->current_cu_pointer -= temp_counter;
          int j = (*pages)->buffer->current_cu_pointer - 1;
          x_cursor_pos = X_PADDING;

          while (j >= 0 && (*pages)->buffer->vect_buff[j] != L'\n') {
            x_cursor_pos++;
            j--;
          }

          y_cursor_pos--;
          move(y_cursor_pos, x_cursor_pos);

          return;
        }
      } while ((*pages)->buffer->vect_buff[i] == L' ' &&
               (*pages)->buffer->vect_buff[i] != L'\n');

      (*pages)->buffer->current_cu_pointer--;
      x_cursor_pos--;
      if (x_cursor_pos < X_PADDING)
        x_cursor_pos = X_PADDING;

      display_char(
          y_cursor_pos, x_cursor_pos,
          (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer +
                                      (*pages)->buffer->offset],
          NO_COLOR);
    }
  }
}

void handle_enter_key(NodeBuffer **pages) {
  wchar_t buffer_cu_char =
      (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer];

  // check if reached the end of the buffer
  if ((*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer + 1] ==
      L'\0') {
    next_buffer(pages);
    draw_buffer((*pages)->buffer, NO_COLOR);

    y_cursor_pos = Y_PADDING / 2;
    x_cursor_pos = X_PADDING;

    do {
      if ((*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer] ==
          L' ')
        (*pages)->buffer->current_cu_pointer++;
      x_cursor_pos++;
    } while (
        (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer] ==
        L' ');

    move(y_cursor_pos, x_cursor_pos);

    return;
  }

  if (buffer_cu_char == L'\n' && !(*pages)->buffer->offset) { // end of the line
    y_cursor_pos++;
    x_cursor_pos = X_PADDING;

    do {
      (*pages)->buffer->current_cu_pointer++;
      buffer_cu_char =
          (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer];
      if (buffer_cu_char == L' ')
        x_cursor_pos++;
    } while (buffer_cu_char == L' ');

    move(y_cursor_pos, x_cursor_pos);
  } else if (buffer_cu_char !=
             L'\n') { // wrong character at the middle of the line
    display_char(y_cursor_pos, x_cursor_pos, '_', RED);
    (*pages)->buffer->offset++;
    x_cursor_pos++;
    move(y_cursor_pos, x_cursor_pos);
  }
}

void handle_space_key(wchar_t user_input, Buffer *buffer) {
  wchar_t buffer_cu_char = buffer->vect_buff[buffer->current_cu_pointer];

  if (user_input != buffer_cu_char && buffer_cu_char == L'\n') {
    if (!buffer->offset) {
      display_char(y_cursor_pos, x_cursor_pos, '_', RED);
      x_cursor_pos++;
      move(y_cursor_pos, x_cursor_pos);
    }

    buffer->offset++;
    if (buffer->offset > 1)
      buffer->offset = 1;
  } else if (user_input != buffer_cu_char) {
    display_char(y_cursor_pos, x_cursor_pos, '_', RED);
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
  wchar_t buffer_cu_char = buffer->vect_buff[buffer->current_cu_pointer];

  if (buffer_cu_char == L'\n') {
    if (!buffer->offset) {
      buffer->offset++;
      display_char(y_cursor_pos, x_cursor_pos, user_input, RED);
      x_cursor_pos++;
      move(y_cursor_pos, x_cursor_pos);
    } else {
      display_char(y_cursor_pos, x_cursor_pos, user_input, RED);
    }
  } else {
    buffer->offset++;

    if (buffer->offset)
      display_char(y_cursor_pos, x_cursor_pos, user_input, RED);
    else
      display_char(y_cursor_pos, x_cursor_pos, user_input, GREEN);

    x_cursor_pos++;
    move(y_cursor_pos, x_cursor_pos);
  }
}

void handle_right_key(wchar_t user_input, Buffer *buffer) {
  // prevent for displaying green characters after red ones
  if (buffer->offset)
    display_char(y_cursor_pos, x_cursor_pos, user_input, RED);
  else
    display_char(y_cursor_pos, x_cursor_pos, user_input, GREEN);

  buffer->current_cu_pointer++;
  x_cursor_pos++;
  move(y_cursor_pos, x_cursor_pos);
}

void handle_input(wchar_t user_input, FILE *file, NodeBuffer **pages) {
  if (!started_test)
    start_timer();

  wchar_t buffer_cu_char =
      (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer];

  if (user_input == 27) { // ESC
    handle_del_key(file, pages);
  } else if (user_input == 127) { // BACKSPACE
    handle_bs_key(pages);
  } else if (user_input == '\n') {
    handle_enter_key(pages);
  } else if (user_input == ' ') {
    handle_space_key(user_input, (*pages)->buffer);
  } else if (user_input != buffer_cu_char) {
    handle_wrong_key(user_input, (*pages)->buffer);
  } else if (user_input == buffer_cu_char) {
    handle_right_key(user_input, (*pages)->buffer);
    check_end_game(file, pages);
  }
}
