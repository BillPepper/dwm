#include "core.h"

void quit(const Arg *arg) {
  if (arg->i) {
    restart = 1;
  }

  running = 0;
}
