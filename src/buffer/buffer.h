#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <wchar.h>

#include "../gamestate/gamestate.h"

typedef struct Range {
  int start;
  int end;
} Range;

typedef struct Buffer {
  int page_number;
  int current_cu_pointer;
  int offset;
  int size;
  Range lines_range;
  wchar_t *vect_buff;
} Buffer;

typedef struct NodeBuffer {
  struct Buffer *buffer;
  struct NodeBuffer *next;
  struct NodeBuffer *previous;
} NodeBuffer;

Buffer create_buffer(FILE *file);
NodeBuffer *create_buffer_node(FILE *file);
void set_pages(NodeBuffer **pages, FILE *file);
void previous_buffer(NodeBuffer **pages);
void next_buffer(NodeBuffer **pages);

#endif
