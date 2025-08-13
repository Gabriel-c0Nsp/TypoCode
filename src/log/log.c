#include <ncurses.h>
#include <stdio.h>
#include <time.h>

#include "../endgame/endgame.h"
#include "log.h"

void logtf(const char *fmt, ...) {
  FILE *log_file = fopen("log.txt", "a");
  if (!log_file) {
    endwin();
    fprintf(
        stderr,
        "Failed to open log file: log.txt\nAborting for security reasons...\n");
    exit_game(1, NULL, NULL);
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
