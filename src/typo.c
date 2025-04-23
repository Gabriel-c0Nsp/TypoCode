#define NCURSES_WIDECHAR 1

#include <curses.h>
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

// Doubly linked list to store file buffer
typedef struct BF_Node {
  wchar_t file_char;
  struct BF_Node *next;
  struct BF_Node *prev;
} BF_Node;


BF_Node *create_node(wchar_t file_char);
void insert_buffer(BF_Node **buffer, wchar_t file_char);
void clean_buffer(BF_Node **buffer);

FILE *open_file(char *argv);
void close_file(FILE *file_path);

void store_file_buffer(BF_Node **buffer, FILE *file);

void draw_buffer(BF_Node *buffer);

wchar_t get_user_input(FILE *file, BF_Node *buffer);

void exit_game(int exit_status, FILE *file_path, BF_Node **buffer);


int y_cursor_pos = 0;
int x_cursor_pos = 0;

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "pt_BR");

  // TODO: Try this one later
  // setlocale(LC_ALL, "UTF-8");

  FILE *file_path = open_file(argv[1]);

  BF_Node *buffer = NULL;
  store_file_buffer(&buffer, file_path);

  initscr();
  cbreak();
  noecho();

  draw_buffer(buffer);

  while (true) {
    wchar_t input = get_user_input(file_path, buffer);
  }

  return 0;
}

BF_Node *create_node(wchar_t file_char) {
  BF_Node *new_node = (BF_Node *)malloc(sizeof(BF_Node));

  if (new_node == NULL) {
    printf("Something went wrong while reading the provided file!\nAborting...\n");
    exit(1);
  }

  new_node->file_char = file_char;
  new_node->next = NULL;
  new_node->prev = NULL;

  return new_node;
}

void insert_buffer(BF_Node **buffer, wchar_t file_char) {
  BF_Node *new_node = create_node(file_char);  

  if (*buffer == NULL) {
    *buffer = new_node;
    return;
  }

  BF_Node *temp = *buffer;

  while (temp->next != NULL) {
    temp = temp->next;
  }

  temp->next = new_node;
  new_node->prev = temp;
}

void clean_buffer(BF_Node **buffer) {
  BF_Node *temp = *buffer;

  while (temp != NULL) {
    BF_Node *next = temp->next;
    free(temp);
    temp = next;
  }

  *buffer = NULL;
}

FILE *open_file(char *argv) {
  FILE *file_path;

  if ((file_path = fopen(argv, "r")) == NULL) {
    printf("Não foi posspivel abrir o arquivo!\n");
    exit(1);
  }

  return file_path;
}

void close_file(FILE *file_path) {
  fclose(file_path);
}

void store_file_buffer(BF_Node **buffer, FILE *file) {
  wchar_t file_char;

  do {
    file_char = getc(file);

    insert_buffer(buffer, file_char);
  } while (file_char != EOF);
}

// TODO: Insert '\n' characters to the end of the lines
void draw_buffer(BF_Node *buffer) {
  clear();
  move(0, 0);

  int original_y_cu_pos = y_cursor_pos;
  int original_x_cu_pos = x_cursor_pos;

  y_cursor_pos = 0;
  x_cursor_pos = 0;

  BF_Node *temp = buffer;

  while (temp->next != NULL) {
    mvaddch(y_cursor_pos, x_cursor_pos, temp->file_char); 
    x_cursor_pos++;

    if (temp->file_char == '\n') {
      y_cursor_pos++;   
      x_cursor_pos = 0;
    }
    
    temp = temp->next;
  }

  refresh();

  y_cursor_pos = original_y_cu_pos;
  x_cursor_pos = original_x_cu_pos;

  move(y_cursor_pos, x_cursor_pos);
}

wchar_t get_user_input(FILE *file, BF_Node *buffer) {
  wchar_t user_input = getch();

  if (user_input == 27) exit_game(0, file, &buffer);

  if (user_input >= 32 && user_input <= 125) {
    mvaddch(y_cursor_pos, x_cursor_pos, user_input);
    refresh();
    x_cursor_pos++;
  } else if (user_input == 127) {
    // TODO: Handle user input backspace key
  } else if (user_input == '\n') {
    // TODO: Handle user input '\n' case
  }

  return user_input;
}

void exit_game(int exit_status, FILE *file_path, BF_Node **buffer) {
  endwin();
  close_file(file_path);
  clean_buffer(buffer);

  exit(exit_status);
}
