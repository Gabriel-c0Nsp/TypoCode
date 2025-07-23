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

#define PADDING 8

typedef struct Buffer {
  int page_number;
  int current_cu_pointer;
  int offset;
  int size;
  wchar_t *vect_buff;
} Buffer;

typedef struct NodeBuffer {
  struct Buffer *buffer;
  struct NodeBuffer *proximo;
  struct NodeBuffer *anterior;
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

void handle_pages(NodeBuffer **pages, FileInformation *file_info, FILE *file);
void draw_buffer(Buffer *buffer);
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

  /* draw_buffer(pages->buffer); */
  /* draw_buffer(pages->proximo->buffer); */
  /* draw_buffer(pages->proximo->proximo->buffer); */
  draw_buffer(pages->proximo->proximo->proximo->buffer);
  while (1) {
    handle_input(get_user_input(file, &pages), file, &pages);
  }

  // TEST:
  // NOTE: Delete this later
  /* while (getch() != 'q') { */
  /*   printw("Número de letras no arquivo: %d\n",
   * file_info.number_of_characters); */
  /*   printw("Número de linhas no arquivo: %d\n", file_info.number_of_lines);
   */
  /*   printw("Número de buffers: %d\n", file_info.number_of_buffers); */
  /*   printw("Número de linhas disponívels no terminal: %d\n", LINES); */
  /*   printw("Posição atual do cursor do buffer: %d\n",
   * pages->buffer->current_cu_pointer); */
  /*   printw("Número do buffer atual: %d\n", pages->buffer->page_number); */
  /*   printw("Primeira letra do primeiro buffer: %lc\n",
   * pages->buffer->vect_buff[2]); */
  /* } */
  /**/
  /* endwin(); */

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

  // Se o arquivo está vazio
  if (number_of_characters == 0) {
    number_of_lines = 0;
  }

  file_info->number_of_characters = number_of_characters;
  file_info->number_of_lines = number_of_lines;

  // Calcular número de buffers baseado nas linhas disponíveis
  int lines_per_buffer = LINES - PADDING;
  if (lines_per_buffer <= 0)
    lines_per_buffer = 1; // at least one line per buffer

  file_info->number_of_buffers =
      (number_of_lines + lines_per_buffer - 1) / lines_per_buffer;
  if (file_info->number_of_buffers == 0) {
    file_info->number_of_buffers = 1; // pelo menos 1 buffer
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

  // Count how many characters the buffer needs to able to store
  long start_pos = ftell(file);
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

  // Ler o conteúdo do buffer
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

  // CORREÇÃO: Garantir que o buffer termine com \0
  if (i <= buffer.size) {
    buffer.vect_buff[i] = L'\0';
    // Atualizar o tamanho real do buffer
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
  new_node->proximo = NULL;
  new_node->anterior = NULL;

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
      while (temp->proximo != NULL) {
        temp = temp->proximo;
      }
      temp->proximo = new_node;
      new_node->anterior = temp;
    }

    current_number_of_buffers++;
    new_node->buffer->page_number = current_number_of_buffers;

    if (feof(file) == WEOF) {
      break;
    }
  }
}

void handle_pages(NodeBuffer **pages, FileInformation *file_info, FILE *file) {
  /*
    // TODO: Implement
  */
}

void draw_buffer(Buffer *buffer) {
  // FIX: sometimes draws an extra "@" at the end
  // TODO: Make 8 lines padding so it looks better (4 at the top and 4 at the
  // bottom)
  // TODO: Draw line numbers
  // TODO: Implement pagination
  // NOTE: Remember to use LINES and COLS variables

  clear();
  move(0, 0); // NOTE: Need to calculate padding after (y = PADDING)

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

  y_cursor_pos = 0;
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
  // FIX: with the new updates to a new data structure, don't delete the last
  // character of the line if it was typed wrong
  // TODO: Draw buffer depending on the key the user is deleting
  if ((*pages)->buffer->current_cu_pointer || (*pages)->buffer->offset) {
    if ((*pages)->buffer->offset) {
      (*pages)->buffer->offset--;

      x_cursor_pos--;
      if (x_cursor_pos < 0)
        x_cursor_pos = 0;

      display_char(
          y_cursor_pos, x_cursor_pos,
          (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer +
                                      (*pages)->buffer->offset],
          NO_COLOR);

    } else if (!(*pages)->buffer->offset &&
               (*pages)->buffer->current_cu_pointer) {
      int i = (*pages)->buffer->current_cu_pointer;
      int temp_counter = 0;

      do {
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
  }
}

void handle_enter_key(NodeBuffer **pages) {
  // TODO: Needs to point to the next pages->buffer if the user gets to the end
  // of the buffer
  wchar_t buffer_cu_char =
      (*pages)->buffer->vect_buff[(*pages)->buffer->current_cu_pointer];

  if (buffer_cu_char == L'\n' && !(*pages)->buffer->offset) {
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
  } else if (buffer_cu_char != L'\n') {
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
}

void free_pages(NodeBuffer **pages) {
  NodeBuffer *current = *pages;

  while (current != NULL) {
    NodeBuffer *next = current->proximo;
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
