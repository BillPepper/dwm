#include "dwm.h"

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags {
  char limitexceeded[LENGTH(tags) > 31 ? -1 : 1];
};

#ifdef XINERAMA
static int isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info) {
  while (n--)
    if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org &&
        unique[n].width == info->width && unique[n].height == info->height)
      return 0;
  return 1;
}
#endif /* XINERAMA */

int updategeom(void) {
  int dirty = 0;

  #ifdef XINERAMA
  if (XineramaIsActive(display)) {
    int i, j, n, nn;
    Client *c;
    Monitor *m;
    XineramaScreenInfo *info = XineramaQueryScreens(display, &nn);
    XineramaScreenInfo *unique = NULL;

    for (n = 0, m = monitors; m; m = m->next, n++)
      ;
    /* only consider unique geometries as separate screens */
    unique = ecalloc(nn, sizeof(XineramaScreenInfo));
    for (i = 0, j = 0; i < nn; i++) {
      if (isuniquegeom(unique, j, &info[i])){
        memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
	  }
	}
    XFree(info);
    nn = j;

    /* new monitors if nn > n */
    for (i = n; i < nn; i++) {
      for (m = monitors; m && m->next; m = m->next)
        ;
      if (m) {
        m->next = createmon();
	  } else {
        monitors = createmon();
	  }
    }
    for (i = 0, m = monitors; i < nn && m; m = m->next, i++){
      if (i >= n || unique[i].x_org != m->monitor_area.position.x || unique[i].y_org != m->monitor_area.position.y || unique[i].width != m->monitor_area.size.w || unique[i].height != m->monitor_area.size.h) {
        dirty = 1;
        m->num = i;
        m->monitor_area.position.x = m->window_area.position.x = unique[i].x_org;
        m->monitor_area.position.y = m->window_area.position.y = unique[i].y_org;
        m->monitor_area.size.w = m->window_area.size.w = unique[i].width;
        m->monitor_area.size.h = m->window_area.size.h = unique[i].height;
        update_bar_position(m);
      }
	}

    /* removed monitors if n > nn */
    for (i = nn; i < n; i++) {
      for (m = monitors; m && m->next; m = m->next)
        ;
      while ((c = m->clients)) {
        dirty = 1;
        m->clients = c->next;
        detachstack(c);
        c->monitor = monitors;
        attach(c);
        attachstack(c);
      }
      if (m == selected_monitor) {
        selected_monitor = monitors;
	  }
      cleanupmon(m);
    }
    free(unique);
  } else
  #endif /* XINERAMA */
  {    /* default monitor setup */
    if (!monitors) {
      monitors = createmon();
	}
    if (monitors->monitor_area.size.w != screen_width || monitors->monitor_area.size.h != screen_height) {
      dirty = 1;
      monitors->monitor_area.size.w = monitors->window_area.size.w = screen_width;
      monitors->monitor_area.size.h = monitors->window_area.size.h = screen_height;
      update_bar_position(monitors);
    }
  }
  if (dirty) {
    selected_monitor = monitors;
    selected_monitor = wintomon(root);
  }

  return dirty;
}


// -- Events -------------------------------------------------------------------




// -- Main ---------------------------------------------------------------------

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
