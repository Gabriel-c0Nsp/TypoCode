#define NCURSES_WIDECHAR 1

#include <locale.h>
#include <ncurses.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

#define NO_COLOR A_NORMAL
#define GREEN COLOR_PAIR(1)
#define RED COLOR_PAIR(2)

#define PADDING 6

typedef struct Buffer {
  int page_number;
  int current_cu_pointer;
  int offset;
  int size;
  wchar_t *vect_buff;
} Buffer;

typedef struct NodeBuffer {
  struct Buffer *buffer;
  struct NodeBuffer *next;
  struct NodeBuffer *previous;
} NodeBuffer;

typedef struct FileInformation {
  int number_of_characters;
  int number_of_lines;
  int number_of_buffers;
} FileInformation;

// log/debug functions
void logtf(const char *fmt, ...);

// file related operations
FILE *open_file(char *argv);
void close_file(FILE *file);

// buffer related operations
FileInformation get_file_information(FileInformation *file_info, FILE *file);
int file_char_number(FILE *file);
Buffer create_buffer(FILE *file);
NodeBuffer *create_buffer_node(FILE *file);
void set_pages(NodeBuffer **pages, FILE *file, FileInformation *file_info);
void previous_buffer(NodeBuffer **pages);
void next_buffer(NodeBuffer **pages);

void draw_buffer(Buffer *buffer, attr_t attr);
void display_char(int y, int x, wchar_t character, attr_t attr);

// user input related operations
wchar_t get_user_input(FILE *file, NodeBuffer **pages);
void handle_del_key(FILE *file, NodeBuffer **pages);
void handle_bs_key(NodeBuffer **pages);
void handle_enter_key(NodeBuffer **pages);
void handle_space_key(wchar_t user_input, Buffer *buffer);
void handle_wrong_key(wchar_t user_input, Buffer *buffer);
void handle_right_key(wchar_t user_input, Buffer *buffer);
void handle_input(wchar_t user_input, FILE *file, NodeBuffer **pages);

void free_pages(NodeBuffer **pages);
void exit_game(int exit_status, FILE *file_path, NodeBuffer **pages);

// cursor position global variables
int y_cursor_pos = PADDING;
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

  FileInformation file_info;

  file_info = get_file_information(&file_info, file);
  NodeBuffer *pages = NULL;
  set_pages(&pages, file, &file_info);

  if (pages == NULL) {
    fprintf(stderr, "Something went wrong while allocating memory for "
                    "buffers!\nAborting...\n");
    endwin();
    exit(1);
  }

  draw_buffer(pages->buffer, NO_COLOR);
  while (1) {
    handle_input(get_user_input(file, &pages), file, &pages);
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

FileInformation get_file_information(FileInformation *file_info, FILE *file) {
  int number_of_characters = 0;
  wint_t file_char;
  int number_of_lines = 1;

  while ((file_char = fgetwc(file)) != WEOF) {
    if (file_char == '\t') {
      number_of_characters += 2; // tab will become two spaces
    } else {
      number_of_characters++;
    }

    if (file_char == '\n') {
      number_of_lines++;
    }
  }

  // if the file is empty
  if (number_of_characters == 0) {
    number_of_lines = 0;
    // TODO: Exit game, there's nothing to do!
  }

  file_info->number_of_characters = number_of_characters;
  file_info->number_of_lines = number_of_lines;

  int lines_per_buffer = LINES - PADDING;
  if (lines_per_buffer <= 0)
    lines_per_buffer = 1; // at least one line per buffer

  file_info->number_of_buffers =
      (number_of_lines + lines_per_buffer - 1) / lines_per_buffer;
  if (file_info->number_of_buffers == 0) {
    file_info->number_of_buffers = 1; // needs to have at least one buffer
  }

  rewind(file);
  return *file_info;
}

Buffer create_buffer(FILE *file) {
  int buffer_capacity = LINES - PADDING;
  int current_lines = 0;
  int char_count = 0;

  Buffer buffer;
  buffer.page_number = 0;
  buffer.current_cu_pointer = 0;
  buffer.offset = 0;

  long start_pos = ftell(file);

  // Count how many characters the buffer needs to able to store
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
    }
  }

  buffer.size = char_count;
  buffer.vect_buff = calloc(buffer.size + 1, sizeof(wchar_t));

  if (buffer.vect_buff == NULL) {
    fprintf(stderr, "Erro ao alocar memória para o buffer\n");
    exit(1);
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
    fprintf(stderr, "Couldn't allocate memory for a new node in NodeBuffer "
                    "doubly-linked list!\nAborting...\n");
    exit(1);
  }

  Buffer *new_buffer = malloc(sizeof(Buffer));
  if (new_buffer == NULL) {
    fprintf(stderr, "Couldn't allocate memory for a new Buffer\nAborting...\n");
    exit(1);
  }

  *new_buffer = create_buffer(file);
  new_node->buffer = new_buffer;
  new_node->next = NULL;
  new_node->previous = NULL;

  return new_node;
}

void set_pages(NodeBuffer **pages, FILE *file, FileInformation *file_info) {
  int current_number_of_buffers = 0;

  while (current_number_of_buffers < file_info->number_of_buffers) {
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
  if ((*pages)->previous != NULL)
    *pages = (*pages)->previous;

  return;
}

void next_buffer(NodeBuffer **pages) {
  if ((*pages)->next != NULL)
    *pages = (*pages)->next;

  return;
}

void draw_buffer(Buffer *buffer, attr_t attr) {
  // TODO: Draw line numbers
  // TODO: pass colors as argument (to draw previous buffer green)

  clear();
  move(PADDING, 0);

  int x_pos = 0;
  int y_pos = PADDING / 2;

  attron(attr);
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

  attroff(attr);

  y_cursor_pos = PADDING / 2;
  x_cursor_pos = 0;
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

wchar_t get_user_input(FILE *file, NodeBuffer **pages) {
  wint_t user_input;
  int result = get_wch(&user_input);

  if (result == ERR) {
    fprintf(stderr, "Some error occurred while typing\n");
    exit_game(1, file, pages);
  }

  wchar_t result_char = (wchar_t)user_input;
  return result_char;
}

void handle_del_key(FILE *file, NodeBuffer **pages) {
  exit_game(0, file, pages);
}

void handle_bs_key(NodeBuffer **pages) {
  // TODO: ignore tabs feature when returning a buffer as well
  wchar_t buffer_cu_char =
      (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer];

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
        if (x_cursor_pos < 0)
          x_cursor_pos = 0;

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
          x_cursor_pos = 0;

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
      if (x_cursor_pos < 0)
        x_cursor_pos = 0;

      display_char(
          y_cursor_pos, x_cursor_pos,
          (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer +
                                      (*pages)->buffer->offset],
          NO_COLOR);
    }
  } else if (!(*pages)->buffer->current_cu_pointer &&
             !((*pages)->buffer->offset)) { // fist character in the buffer
    previous_buffer(pages);

    (*pages)->buffer->current_cu_pointer = (*pages)->buffer->size;
    (*pages)->buffer->offset = 0;

    draw_buffer((*pages)->buffer, GREEN);

    y_cursor_pos = PADDING / 2;
    x_cursor_pos = 0;

    // find the last cursor position in terms of x
    for (int i = 0; i < (*pages)->buffer->size; i++) {
      if ((*pages)->buffer->vect_buff[i] == L'\n' && (*pages)->buffer->vect_buff[i + 1] != L'\0') {
        y_cursor_pos++;
        x_cursor_pos = 0;
      } else {
        x_cursor_pos++;
      }
    }

    x_cursor_pos--;
    (*pages)->buffer->current_cu_pointer--;

    // position the cursor at the end of the previous buffer
    move(y_cursor_pos, x_cursor_pos);
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

    y_cursor_pos = PADDING / 2;
    x_cursor_pos = 0;

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
    x_cursor_pos = 0;

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
  }

  logtf("caractere atual: %lc\n",
        (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer]);
  logtf("offset: %d\n", (*pages)->buffer->offset);
  logtf("número atual do cursor no buffer: %d\n",
        (*pages)->buffer->current_cu_pointer);
  logtf("tamanho do buffer atual: %d\n", (*pages)->buffer->size);
  logtf("posição atual do cursor x: %d\n", x_cursor_pos);
  logtf("posição atual do cursor y: %d\n", y_cursor_pos);
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
  endwin();
  close_file(file_path);
  free_pages(pages);
  exit(exit_status);
}
