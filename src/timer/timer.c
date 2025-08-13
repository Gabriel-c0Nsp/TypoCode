#include <ncurses.h>
#include <time.h>

#include "../gamestate/gamestate.h"
#include "timer.h"

static struct timespec start;

void start_timer() {
  clock_gettime(CLOCK_MONOTONIC, &start);
  started_test = 1;
}

void stop_timer() {
  struct timespec end;
  clock_gettime(CLOCK_MONOTONIC, &end);

  long seconds = end.tv_sec - start.tv_sec;
  long minutes = seconds / 60;
  seconds %= 60;

  endwin(); // simple way to get to the stdout (maybe I'm being kinda lazy here)
  printf("Finished in: %02ld:%02ld\n", minutes, seconds);
}
