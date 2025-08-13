#include <math.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "../endgame/endgame.h"
#include "file.h"

FileInformation file_info;

char *extract_file_name(char *file_path) {
  char *file_name = strrchr(file_path, '/');

  if (file_name != NULL)
    file_name++;
  else
    file_name = file_path;

  return file_name;
}

int str_bytes_num(int number) {
  return (int)((ceil(log10(number)) + 2) * sizeof(char));
}

FILE *open_file(char *argv) {
  FILE *file;

  if ((file = fopen(argv, "r")) == NULL) {
    fprintf(stderr, "couldn't open file\nAborting...\n");
    exit(1);
  }

  return file;
}

void close_file(FILE *file) {
  if (file != NULL)
    fclose(file);
}

FileInformation get_file_information(FileInformation *file_info, FILE *file,
                                     char *file_name) {
  int number_of_characters = 0;
  wint_t file_char;
  int number_of_lines = 0;

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
    endwin();
    fprintf(stderr, "You can't play with a blank file!\n");
    exit_game(1, file, NULL);
  }

  file_info->number_of_characters = number_of_characters;
  file_info->number_of_lines = number_of_lines - 1; // exclude the last \n
  file_info->file_name = file_name;

  int lines_per_buffer = LINES - Y_PADDING;
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
