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
  updategeom();

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
  updatebars();
  updatestatus();

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