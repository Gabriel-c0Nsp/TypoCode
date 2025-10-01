#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *version = "v1.0\n";
char *help_message =
    "\nTypoCode is a very simple terminal-based game where the typing "
    "challenges\n"
    "are made of computer science algorithms or source code files provided\n"
    "by the user. The idea is to help people who are either practicing typing\n"
    "skills or learning programming in general.\n\n"

    "The main goal of the game is to be a fun little warm-up before a coding\n"
    "session, something lightweight you can run directly in your terminal,\n"
    "anytime and anywhere.\n\n"

    "Usage: typo [file name | -h | -v]\n\n"

    "[file path]\t runs the typing game using the provided file\n"
    "-v, --version\t display version information and exit\n"
    "-h, --help\t display this help messagem and exit\n";

void check_enough_args(int argc) {
  if (argc < 2) {
    fprintf(stderr, "You should specify a file!\n");
    exit(1);
  }
}

void check_argument_flags(char *argv[]) {
  if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
    printf("%s", version);
    exit(0);
  } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
    printf("%s", help_message);
    exit(0);
  }
}
