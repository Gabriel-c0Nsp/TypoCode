#ifndef FILE_H
#define FILE_H

#include <stdio.h>

typedef struct FileInformation {
  char *file_name;
  int number_of_characters;
  int number_of_lines;
  int number_of_buffers;
} FileInformation;

// helper functions
char *extract_file_name(char *file_path);
int str_bytes_num(int number);

FILE *open_file(char *argv);
void close_file(FILE *file);
FileInformation get_file_information(FileInformation *file_info, FILE *file,
                                     char *file_name);

extern FileInformation file_info;

#endif
