#ifndef INPUT_H
#define INPUT_H

#include "../buffer/buffer.h"

wchar_t get_user_input(FILE *file, NodeBuffer **pages);
void handle_del_key(FILE *file, NodeBuffer **pages);
void handle_bs_key(NodeBuffer **pages);
void handle_enter_key(NodeBuffer **pages);
void handle_space_key(wchar_t user_input, Buffer *buffer);
void handle_wrong_key(wchar_t user_input, Buffer *buffer);
void handle_right_key(wchar_t user_input, Buffer *buffer);
void handle_input(wchar_t user_input, FILE *file, NodeBuffer **pages);

#endif
