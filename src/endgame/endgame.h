#ifndef ENDGAME_H
#define ENDGAME_H

#include "../buffer/buffer.h"

void check_end_game(FILE *file, NodeBuffer **pages);
void free_pages(NodeBuffer **pages);
void exit_game(int exit_status, FILE *file_path, NodeBuffer **pages);

#endif
