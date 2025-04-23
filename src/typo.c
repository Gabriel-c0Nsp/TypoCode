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

void display_buffer(BF_Node *buffer);
void exit_game(int exit_status, FILE *file_path);

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

  display_buffer(buffer);

  getch();
  endwin();
  close_file(file_path);

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

void display_buffer(BF_Node *buffer) {
  move(0, 0);

  int original_y_cu_pos = y_cursor_pos;
  int original_x_cu_pos = x_cursor_pos;

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

  y_cursor_pos = original_y_cu_pos;
  x_cursor_pos = original_x_cu_pos;

  move(y_cursor_pos, x_cursor_pos);
}
void exit_game(int exit_status, FILE *file_path) {
  endwin();
  close_file(file_path);

  exit(exit_status);
}
