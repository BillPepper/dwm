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

  // create fontset
  if (!drw_fontset_create(drw, fonts, font_count)) {
    die("no fonts could be loaded.");
  }

  padding = drw->fonts->h;
  bar_height = drw->fonts->h + 2;
  update_geometry();

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
  update_systray();

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
  grab_keys();
  focus(NULL);
}

void run(void) {
  XEvent event;
  /* main event loop */
  XSync(display, False);
  while (running && !XNextEvent(display, &event)) {
    if (handler[event.type]) {
      handler[event.type](&event); /* call handler */
	  }
  }
}

void scan(void) {
  unsigned int i;
  unsigned int child_count;
  Window root_return;
  Window parent_return;
  Window *children = NULL;
  XWindowAttributes window_attributes;

  // query windows
  if (XQueryTree(display, root, &root_return, &parent_return, &children, &child_count)) {

    for (i = 0; i < child_count; i++) {

      // (?) if child has not attributes, is overriden or is a transient, skip
      if (!XGetWindowAttributes(display, children[i], &window_attributes) || window_attributes.override_redirect || XGetTransientForHint(display, children[i], &root_return)) {
        continue;
	    }

      // create a client for the window
      if (window_attributes.map_state == IsViewable || get_state(children[i]) == IconicState) {
        manage(children[i], &window_attributes);
	    }
    }

    /* now the transients */
    for (i = 0; i < child_count; i++) {
      if (!XGetWindowAttributes(display, children[i], &window_attributes)) {
        continue;
	    }
      if (XGetTransientForHint(display, children[i], &root_return) && (window_attributes.map_state == IsViewable || get_state(children[i]) == IconicState)) {
        manage(children[i], &window_attributes);
	    }
    }

    if (children) {
      XFree(children);
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
    cleanup_monitor(monitors);
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
  Client *client = selected_monitor->selected_client;

  if (!selected_monitor->layout[selected_monitor->selected_layout]->arrange_func) {
    return;
  }

  if (!client || client->is_floating){
    return;
  }

  if (client == next_tiled(selected_monitor->clients) && !(client = next_tiled(client->next))) {
    return;
  }

  pop(client);
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

Atom get_atom_prop(Client *client, Atom prop) {
  int di;
  unsigned long dl;
  unsigned char *return_property = NULL;
  Atom da, atom = None;

  /* FIXME get_atom_prop should return the number of items and a pointer to
   * the stored data instead of this workaround */
  Atom req = XA_ATOM;
  if (prop == xatom[XembedInfo]){
    req = xatom[XembedInfo];
  }

  if (XGetWindowProperty(display, client->window, prop, 0L, sizeof atom, False, req, &da, &di, &dl, &dl, &return_property) == Success && return_property) {
    atom = *(Atom *)return_property;
    if (da == xatom[XembedInfo] && dl == 2) {
      atom = ((Atom *)return_property)[1];
	  }

    XFree(return_property);
  }

  return atom;
}

int update_geometry(void) {
  int dirty = 0;

  #ifdef XINERAMA
  if (XineramaIsActive(display)) {
    int i, j, n, nn;
    Client *client;
    Monitor *monitor;
    XineramaScreenInfo *info = XineramaQueryScreens(display, &nn);
    XineramaScreenInfo *unique = NULL;

    for (n = 0, monitor = monitors; monitor; monitor = monitor->next, n++)
      ;
    /* only consider unique geometries as separate screens */
    unique = ecalloc(nn, sizeof(XineramaScreenInfo));
    for (i = 0, j = 0; i < nn; i++) {
      if (is_unique_geometry(unique, j, &info[i])){
        memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
	  }
	}
    XFree(info);
    nn = j;

    /* new monitors if nn > n */
    for (i = n; i < nn; i++) {
      for (monitor = monitors; monitor && monitor->next; monitor = monitor->next)
        ;
      if (monitor) {
        monitor->next = create_monitor();
	  } else {
        monitors = create_monitor();
	  }
    }
    for (i = 0, monitor = monitors; i < nn && monitor; monitor = monitor->next, i++){
      if (i >= n || unique[i].x_org != m->monitor_area.position.x || unique[i].y_org != m->monitor_area.position.y || unique[i].width != m->monitor_area.size.w || unique[i].height != m->monitor_area.size.h) {
        dirty = 1;
        monitor->num = i;
        m->monitor_area.position.x = m->window_area.position.x = unique[i].x_org;
        m->monitor_area.position.y = m->window_area.position.y = unique[i].y_org;
        m->monitor_area.size.w = m->window_area.size.w = unique[i].width;
        m->monitor_area.size.h = m->window_area.size.h = unique[i].height;
        update_bar_position(monitor);
      }
	}

    /* removed monitors if n > nn */
    for (i = nn; i < n; i++) {
      for (monitor = monitors; monitor && monitor->next; monitor = monitor->next)
        ;
      while ((client = monitor->clients)) {
        dirty = 1;
        monitor->clients = client->next;
        detach_stack(client);
        client->monitor = monitors;
        attach(client);
        attach_stack(client);
      }
      if (monitor == selected_monitor) {
        selected_monitor = monitors;
	  }
      cleanup_monitor(monitor);
    }
    free(unique);
  } else
  #endif /* XINERAMA */
  {    /* default monitor setup */
    if (!monitors) {
      monitors = create_monitor();
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
    selected_monitor = window_to_monitor(root);
  }

  return dirty;
}

#ifdef XINERAMA
int is_unique_geometry(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info) {
  while (n--)
    if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org &&
        unique[n].width == info->width && unique[n].height == info->height)
      return 0;
  return 1;
}
#endif /* XINERAMA */
