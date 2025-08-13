#include <stdlib.h>
#include <ncurses.h>

#include "endgame.h"
#include "../file/file.h"
#include "../timer/timer.h"

void check_end_game(FILE *file, NodeBuffer **pages) {
  if ((*pages)->buffer->page_number == file_info.number_of_buffers &&
      (*pages)->buffer->current_cu_pointer == (*pages)->buffer->size - 1) {

    stop_timer();
    exit_game(0, file, pages);
  }
}

void free_pages(NodeBuffer **pages) {
  NodeBuffer *current = *pages;

  while (current != NULL) {
    NodeBuffer *next = current->next;
    free(current->buffer->vect_buff);
    free(current->buffer);
    free(current);
    current = next;
  }

  *pages = NULL;
}

void exit_game(int exit_status, FILE *file_path, NodeBuffer **pages) {
  endwin(); // make sure to close the window
  close_file(file_path);
  if (pages != NULL)
    free_pages(pages);
  exit(exit_status);
}
