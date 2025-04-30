#include "process.h"

void sighup(int unused) {
  Arg a = {.i = 1};
  quit(&a);
}

void sigterm(int unused) {
  Arg a = {.i = 0};
  quit(&a);
}

void spawn(const Arg *arg) {
  // check command
  if (arg->v == dmenucmd) {
    dmenumon[0] = '0' + selected_monitor->num;
  }

  // create new process
  if (fork() == 0) {
    if (display) {
      close(ConnectionNumber(display));
	  }
    setsid();
    execvp(((char **)arg->v)[0], (char **)arg->v);
    die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
  }
}