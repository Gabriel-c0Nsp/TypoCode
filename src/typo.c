#define NCURSES_WIDECHAR 1

#include <curses.h>
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

// file related operations
FILE *open_file(char *argv);
void close_file(FILE *file);

// buffer related operations
int file_char_number(FILE *file);
Buffer *create_buffer(FILE *file);
void draw_buffer(Buffer *buffer);

// cursor position global variables
int y_cursor_pos = 0;
int x_cursor_pos = 0;

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "pt_BR");

  FILE *file = open_file(argv[1]);

  Buffer *buffer = create_buffer(file);

	// debuging purposes
	/* printf("%d", buffer->current_cu_pointer); */
	/* printf("%d", buffer->size); */
	/* printf("%c", buffer->vect_buff[8]); */

  /* initscr(); */
  /* cbreak(); */
  /* noecho(); */
  /**/
  /* draw_buffer(buffer); */
  /* getch(); */

  close_file(file);

  return 0;
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
  wchar_t file_char;

  do {
    file_char = getc(file);
    char_number++;
  } while (file_char != EOF);

  char_number -= 2; // necessary because of the basic stream pointers to the
                    // beginning/end of the file

  return char_number;
}

// FIXME: broke after becoming a pointer
Buffer *create_buffer(FILE *file) {
  int char_number = file_char_number(file);
  wchar_t file_char;

  // initializing
  Buffer *buffer;
  buffer->current_cu_pointer = 0;
  buffer->size = char_number;
  buffer->vect_buff = calloc(char_number, sizeof(wchar_t));

  rewind(file); // reseting file pointer

  for (int i = 0; i <= char_number; i++) {
    file_char = getc(file);
		printf("%c", file_char);
    buffer->vect_buff[i] = file_char;
  }

	// debuging purposes
  /* for (int i = 0; i <= char_number; i++) { */
  /* printf("%c", buffer->vect_buff[i]); */
  /* } */

  return buffer;
}
void draw_buffer(Buffer *buffer) {
  // TODO: Implement
  clear(); // TODO: extract this one to a funciton like start_game()
  move(0, 0);

  int x_pos = 0;
  int y_pos = 0;

  Buffer *temp = buffer;

  /* for (int i = 0; i <= buffer->size; i++) { */
  /*   mvaddch(y_pos, x_pos, buffer->vect_buff[i]); */
  /**/
  /*   // incrementing position */
  /*   x_pos++; */
  /*   if (buffer->vect_buff[i] == '\n') { */
  /*     y_pos++; */
  /*     x_pos = 0; */
  /*   } */
  /* } */

  move(y_cursor_pos, x_cursor_pos); // going back to the original position
}
