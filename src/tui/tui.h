#ifndef TUI_H
#define TUI_H

#define NO_COLOR A_NORMAL
#define BOLD A_BOLD
#define GREEN COLOR_PAIR(1)
#define RED COLOR_PAIR(2)
#define BLUE COLOR_PAIR(3)
#define YELLOW COLOR_PAIR(4)

#include <ncurses.h>
#include "../buffer/buffer.h"

void draw_display_panel();
void draw_file_name();
void draw_number_lines(Buffer *buffer);
void draw_page_number(Buffer *buffer);
void draw_buffer(Buffer *buffer, attr_t attr);
void display_char(int y, int x, wchar_t character, attr_t attr);

#endif
