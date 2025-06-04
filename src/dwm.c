#include "dwm.h"

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags {
  char limitexceeded[LENGTH(tags) > 31 ? -1 : 1];
};

void parse_args(int argc, char *argv[]){
  if (argc == 2 && !strcmp("-v", argv[1])) {
    die("pdwm-" VERSION);
  }
  if (argc != 1) {
    die("usage: dwm [-v]");
  }
}

int main(int argc, char *argv[]) {
  parse_args(argc, argv);

  // locale
  if (!setlocale(LC_CTYPE, "") || !XSupportsLocale()) {
    fputs("warning: no locale support\n", stderr);
  }

  // open display
  if (!(display = XOpenDisplay(NULL))) {
    die("dwm: cannot open display");
  }

  checkotherwm(); // check if another wm is running
  setup();        // init systray, bars, screens, etc.
  scan();         // scan for windows and mangage() them
  run(); 			    // event loop

  // relaunch dwm
  if (restart) {
    execvp(argv[0], argv);
  }

  // exit
  cleanup();
  XCloseDisplay(display);

  return EXIT_SUCCESS;
}
