#include <ncurses.h>
#include <stdlib.h>

#include "buffer.h"
#include "../endgame/endgame.h"
#include "../file/file.h"

Buffer create_buffer(FILE *file) {
  int buffer_capacity = LINES - Y_PADDING;
  int current_lines = 0;
  int char_count = 0;
  static int range_line_counter = 0;

  Buffer buffer;
  buffer.lines_range.start = range_line_counter + 1;
  buffer.page_number = 0;
  buffer.current_cu_pointer = 0;
  buffer.offset = 0;

  long start_pos = ftell(file);

  // count how many characters the buffer needs to able to store
  wint_t file_char;

  while (current_lines < buffer_capacity &&
         (file_char = fgetwc(file)) != WEOF) {
    if ((wchar_t)file_char == L'\t') {
      char_count += 2; // one tab char later will become two spaces char
    } else {
      char_count++;
    }

    if ((wchar_t)file_char == L'\n') {
      current_lines++;
      range_line_counter++;
    }
  }

  buffer.size = char_count;
  buffer.lines_range.end = range_line_counter;
  buffer.vect_buff = calloc(buffer.size + 1, sizeof(wchar_t));

  if (buffer.vect_buff == NULL) {
    endwin();
    fprintf(stderr, "Couldn't allocate memory for the buffer\nAborting...\n");
    exit_game(1, file, NULL);
  }

  fseek(file, start_pos, SEEK_SET);

  current_lines = 0;
  int i = 0;

  while (current_lines < buffer_capacity &&
         (file_char = fgetwc(file)) != WEOF && i < buffer.size) {

    if ((wchar_t)file_char == L'\t') {
      buffer.vect_buff[i++] = L' ';
      if (i < buffer.size) {
        buffer.vect_buff[i++] = L' ';
      }
    } else {
      buffer.vect_buff[i++] = (wchar_t)file_char;
    }

    if ((wchar_t)file_char == L'\n') {
      current_lines++;
    }
  }

  // ensure each buffer ends with a \0 character
  if (i <= buffer.size) {
    buffer.vect_buff[i] = L'\0';
    buffer.size = i;
  }

  return buffer;
}

NodeBuffer *create_buffer_node(FILE *file) {
  NodeBuffer *new_node = malloc(sizeof(NodeBuffer));

  if (new_node == NULL) {
    endwin();
    fprintf(stderr, "Couldn't allocate memory for a new node in NodeBuffer "
                    "doubly-linked list!\nAborting...\n");
    exit_game(1, file, NULL);
  }

  Buffer *new_buffer = malloc(sizeof(Buffer));
  if (new_buffer == NULL) {
    endwin();
    fprintf(stderr, "Couldn't allocate memory for a new Buffer\nAborting...\n");
    exit_game(1, file, NULL);
  }

  *new_buffer = create_buffer(file);
  new_node->buffer = new_buffer;
  new_node->next = NULL;
  new_node->previous = NULL;

  return new_node;
}

void set_pages(NodeBuffer **pages, FILE *file) {
  int current_number_of_buffers = 0;

  while (current_number_of_buffers < file_info.number_of_buffers) {
    NodeBuffer *new_node = create_buffer_node(file);

    // add a node at the end of the doubly linked list
    if (*pages == NULL) {
      *pages = new_node;
    } else {
      NodeBuffer *temp = *pages;
      while (temp->next != NULL) {
        temp = temp->next;
      }
      temp->next = new_node;
      new_node->previous = temp;
    }

    current_number_of_buffers++;
    new_node->buffer->page_number = current_number_of_buffers;

    if ((unsigned int)feof(file) == WEOF) {
      break;
    }
  }
}

void previous_buffer(NodeBuffer **pages) {
  if ((*pages)->previous != NULL) {
    (*pages)->buffer->current_cu_pointer = 0;
    (*pages)->buffer->offset = 0;
    x_cursor_pos = X_PADDING;
    *pages = (*pages)->previous;
  }

  return;
}

void next_buffer(NodeBuffer **pages) {
  if ((*pages)->next != NULL)
    *pages = (*pages)->next;

  return;
}
