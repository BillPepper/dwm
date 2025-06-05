#include "core.h"

void quit(const Arg *arg) {
  if (arg->i) {
    restart = 1;
  }

  running = 0;
}

void setup(void) {
  int i;
  XSetWindowAttributes window_attributes;
  Atom utf8string;
  struct sigaction signal_action;

  /* do not transform children into zombies when they terminate */
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
  signal_action.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &signal_action, NULL);

  /* clean up any zombies (inherited from .xinitrc etc) immediately */
  while (waitpid(-1, NULL, WNOHANG) > 0);

  signal(SIGHUP, sighup);
  signal(SIGTERM, sigterm);

  /* init screen */
  screen = DefaultScreen(display);
  screen_width = DisplayWidth(display, screen);
  screen_height = DisplayHeight(display, screen);
  root = RootWindow(display, screen);
  drw = drw_create(display, screen, root, screen_width, screen_height);

  // font stuff
  if (!drw_fontset_create(drw, fonts, font_count)) {
    die("no fonts could be loaded.");
  }

  padding = drw->fonts->h;
  bar_height = drw->fonts->h + 2;
  update_geom();

  /* init wm atoms */
  utf8string = XInternAtom(display, "UTF8_STRING", False);
  wmatom[WMProtocols] = XInternAtom(display, "WM_PROTOCOLS", False);
  wmatom[WMDelete] = XInternAtom(display, "WM_DELETE_WINDOW", False);
  wmatom[WMState] = XInternAtom(display, "WM_STATE", False);
  wmatom[WMTakeFocus] = XInternAtom(display, "WM_TAKE_FOCUS", False);

  // init net atoms
  netatom[NetActiveWindow] = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
  netatom[NetSupported] = XInternAtom(display, "_NET_SUPPORTED", False);
  netatom[NetSystemTray] = XInternAtom(display, "_NET_SYSTEM_TRAY_S0", False);
  netatom[NetSystemTrayOP] = XInternAtom(display, "_NET_SYSTEM_TRAY_OPCODE", False);
  netatom[NetSystemTrayOrientation] = XInternAtom(display, "_NET_SYSTEM_TRAY_ORIENTATION", False);
  netatom[NetSystemTrayOrientationHorz] = XInternAtom(display, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
  netatom[NetWMName] = XInternAtom(display, "_NET_WM_NAME", False);
  netatom[NetWMState] = XInternAtom(display, "_NET_WM_STATE", False);
  netatom[NetWMCheck] = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False);
  netatom[NetWMFullscreen] = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
  netatom[NetWMWindowType] = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
  netatom[NetWMWindowTypeDialog] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
  netatom[NetClientList] = XInternAtom(display, "_NET_CLIENT_LIST", False);

  // init x atoms
  xatom[Manager] = XInternAtom(display, "MANAGER", False);
  xatom[Xembed] = XInternAtom(display, "_XEMBED", False);
  xatom[XembedInfo] = XInternAtom(display, "_XEMBED_INFO", False);

  // https://tronche.com/gui/x/xlib/appendix/b/
  #define XC_left_ptr 68
  #define XC_sizing 120
  #define XC_fleur 52

  /* init cursors */
  cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
  cursor[CurResize] = drw_cur_create(drw, XC_sizing);
  cursor[CurMove] = drw_cur_create(drw, XC_fleur);

  /* init appearance */
  scheme = ecalloc(color_count, sizeof(Clr *));
  for (i = 0; i < color_count; i++){
    scheme[i] = drw_scm_create(drw, colors[i], 3);
  }

  /* init system tray */
  updatesystray();

  /* init bars */
  update_bars();
  update_status();

  /* supporting window for NetWMCheck */
  wmcheckwin = XCreateSimpleWindow(display, root, 0, 0, 1, 1, 0, 0, 0);
  XChangeProperty(display, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wmcheckwin, 1);
  XChangeProperty(display, wmcheckwin, netatom[NetWMName], utf8string, 8, PropModeReplace, (unsigned char *)"dwm", 3);
  XChangeProperty(display, root, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wmcheckwin, 1);

  /* EWMH support per view */
  XChangeProperty(display, root, netatom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *)netatom, NetLast);
  XDeleteProperty(display, root, netatom[NetClientList]);

  /* select events */
  window_attributes.cursor = cursor[CurNormal]->cursor;
  window_attributes.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
  XChangeWindowAttributes(display, root, CWEventMask | CWCursor, &window_attributes);
  XSelectInput(display, root, window_attributes.event_mask);
  grabkeys();
  focus(NULL);
}

void run(void) {
  XEvent ev;
  /* main event loop */
  XSync(display, False);
  while (running && !XNextEvent(display, &ev)) {
    if (handler[ev.type]) {
      handler[ev.type](&ev); /* call handler */
	  }
  }
}

void scan(void) {
  unsigned int i, num;
  Window d1, d2, *wins = NULL;
  XWindowAttributes wa;

  if (XQueryTree(display, root, &d1, &d2, &wins, &num)) {
    for (i = 0; i < num; i++) {
      if (!XGetWindowAttributes(display, wins[i], &wa) || wa.override_redirect || XGetTransientForHint(display, wins[i], &d1)) {
        continue;
	    }
      if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState) {
        manage(wins[i], &wa);
	    }
    }

    /* now the transients */
    for (i = 0; i < num; i++) {
      if (!XGetWindowAttributes(display, wins[i], &wa)) {
        continue;
	    }
      if (XGetTransientForHint(display, wins[i], &d1) && (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)) {
        manage(wins[i], &wa);
	    }
    }
    if (wins) {
      XFree(wins);
	}
  }
}

void check_other_wm(void) {
  xerrorxlib = XSetErrorHandler(x_error_start);
  /* this causes an error if some other window manager is running */
  XSelectInput(display, DefaultRootWindow(display), SubstructureRedirectMask);
  XSync(display, False);
  XSetErrorHandler(x_error);
  XSync(display, False);
}

void cleanup(void) {
  Arg arg = {.ui = ~0};
  Layout layout = {"", NULL};
  Monitor *monitor;
  size_t i;

  view(&arg);
  selected_monitor->layout[selected_monitor->selected_layout] = &layout;
  for (monitor = monitors; monitor; monitor = monitor->next){
    while (monitor->stack){
      unmanage(monitor->stack, 0);
	  }
  }
  XUngrabKey(display, AnyKey, AnyModifier, root);
  while (monitors){
    cleanupmon(monitors);
  }

  if (systray_enabled) {
    XUnmapWindow(display, systray->window);
    XDestroyWindow(display, systray->window);
    free(systray);
  }

  for (i = 0; i < CurLast; i++){
    drw_cur_free(drw, cursor[i]);
  }

  for (i = 0; i < color_count; i++){
    free(scheme[i]);
  }

  free(scheme);
  XDestroyWindow(display, wmcheckwin);
  drw_free(drw);
  XSync(display, False);
  XSetInputFocus(display, PointerRoot, RevertToPointerRoot, CurrentTime);
  XDeleteProperty(display, root, netatom[NetActiveWindow]);
}

void zoom(const Arg *arg) {
  Client *c = selected_monitor->selected_client;

  if (!selected_monitor->layout[selected_monitor->selected_layout]->arrange_func || !c || c->is_floating) {
    return;
  }

  if (c == nexttiled(selected_monitor->clients) && !(c = nexttiled(c->next))) {
    return;
  }

  pop(c);
}

void view(const Arg *arg) {
  if ((arg->ui & TAGMASK) == selected_monitor->tag_set[selected_monitor->selected_tags]) {
    return;
  }

  selected_monitor->selected_tags ^= 1; /* toggle sel tagset */
  if (arg->ui & TAGMASK) {
    selected_monitor->tag_set[selected_monitor->selected_tags] = arg->ui & TAGMASK;
  }

  focus(NULL);
  arrange(selected_monitor);
}

Atom get_atom_prop(Client *c, Atom prop) {
  int di;
  unsigned long dl;
  unsigned char *p = NULL;
  Atom da, atom = None;

  /* FIXME get_atom_prop should return the number of items and a pointer to
   * the stored data instead of this workaround */
  Atom req = XA_ATOM;
  if (prop == xatom[XembedInfo]){
    req = xatom[XembedInfo];
  }

  if (XGetWindowProperty(display, c->window, prop, 0L, sizeof atom, False, req, &da, &di, &dl, &dl, &p) == Success && p) {
    atom = *(Atom *)p;
    if (da == xatom[XembedInfo] && dl == 2) {
      atom = ((Atom *)p)[1];
	  }

    XFree(p);
  }

  return atom;
}


int update_geom(void) {
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
      if (is_unique_geom(unique, j, &info[i])){
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

#ifdef XINERAMA
int is_unique_geom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info) {
  while (n--)
    if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org &&
        unique[n].width == info->width && unique[n].height == info->height)
      return 0;
  return 1;
}
#endif /* XINERAMA */
