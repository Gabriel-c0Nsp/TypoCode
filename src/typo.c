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

FILE *read_file(char *argv);
void close_file(FILE *file_path);
void clean_buffer(BF_Node **buffer);

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "pt_BR");

  // TODO: Try this one later
  // setlocale(LC_ALL, "UTF-8");

  FILE *file_path = read_file(argv[1]);

  initscr();
  cbreak();
  noecho();

  getch();
  endwin();
  close_file(file_path);

  return 0;
}

FILE *read_file(char *argv) {
  FILE *file_path;

  if ((file_path = fopen(argv, "r")) == NULL) {
    printf("Não foi posspivel abrir o arquivo!\n");
    exit(1);
  }

  return file_path;
}

void close_file(FILE *file_path) {
  fclose(file_path);
  printf("file closed\n");
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

  // new node always refers to buffer
  // buffer prev node refers to last added node (before new node)
  // OBS: file buffer ended when buffer->next == NULL

  /*
           NULL                   new_node
    || <- buffer -> ||   ==   || <- buffer -> ||

           node                               new_node 
    || <- buffer -> ||   ==   || <- node -> <- buffer -> ||
                                                                   new_node
    || <- node -> <- buffer -> ||   ==   || <- node -> <- node -> <- buffer -> ||

  */

  if (*buffer == NULL) {
    *buffer = new_node;
    return;
  }

  BF_Node *temp = *buffer;

  temp->next = new_node;
  *buffer = new_node;
  new_node->prev = temp;

  // XXX: This should do the work
  // TODO: Test this later
}

void clean_buffer(BF_Node **buffer) {
  BF_Node *temp = *buffer;

  while (temp != NULL) {
    BF_Node *prev = temp->prev;
    free(temp);
    temp = prev;
  }

  *buffer = NULL;
}
