/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

/* macros */
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask) (mask & ~(numlockmask | LockMask) &  (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))
#define INTERSECT(x, y, w, h, m) (MAX(0, MIN((x) + (w), (m)->wx + (m)->ww) - MAX((x), (m)->wx)) * MAX(0, MIN((y) + (h), (m)->wy + (m)->wh) - MAX((y), (m)->wy)))
#define ISVISIBLE(C) ((C->tags & C->mon->tagset[C->mon->seltags]))
#define LENGTH(X) (sizeof X / sizeof X[0])
#define MOUSEMASK (BUTTONMASK | PointerMotionMask)
#define WIDTH(X) ((X)->w + 2 * (X)->bw)
#define HEIGHT(X) ((X)->h + 2 * (X)->bw)
#define TAGMASK ((1 << LENGTH(tags)) - 1)
#define TEXTW(X) (drw_fontset_getwidth(drw, (X)) + lrpad)

#define SYSTEM_TRAY_REQUEST_DOCK 0
/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY 0
#define XEMBED_WINDOW_ACTIVATE 1
#define XEMBED_FOCUS_IN 4
#define XEMBED_MODALITY_ON 10
#define XEMBED_MAPPED (1 << 0)
#define XEMBED_WINDOW_ACTIVATE 1
#define XEMBED_WINDOW_DEACTIVATE 2
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define XEMBED_EMBEDDED_VERSION (VERSION_MAJOR << 16) | VERSION_MINOR

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum { SchemeNorm, SchemeSel };                  /* color schemes */
enum {
  NetSupported,
  NetWMName,
  NetWMState,
  NetWMCheck,
  NetSystemTray,
  NetSystemTrayOP,
  NetSystemTrayOrientation,
  NetSystemTrayOrientationHorz,
  NetWMFullscreen,
  NetActiveWindow,
  NetWMWindowType,
  NetWMWindowTypeDialog,
  NetClientList,
  NetLast
};                                           /* EWMH atoms */
enum { Manager, Xembed, XembedInfo, XLast }; /* Xembed atoms */
enum {
  WMProtocols,
  WMDelete,
  WMState,
  WMTakeFocus,
  WMLast
}; /* default atoms */
enum {
  ClkTagBar,
  ClkLtSymbol,
  ClkStatusText,
  ClkWinTitle,
  ClkClientWin,
  ClkRootWin,
  ClkLast
}; /* clicks */

typedef union {
  int i;
  unsigned int ui;
  float f;
  const void *v;
} Arg;

typedef struct {
  unsigned int click;
  unsigned int mask;
  unsigned int button;
  void (*func)(const Arg *arg);
  const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;

// dwm uses clients instead of windows
struct Client {
  char name[256];
  float mina, maxa;
  int x, y, w, h;
  int oldx, oldy, oldw, oldh;
  int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
  int bw, oldbw;
  unsigned int tags;
  int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
  Client *next;
  Client *snext;
  Monitor *mon;
  Window win;
};

typedef struct {
  unsigned int mod;
  KeySym keysym;
  void (*func)(const Arg *);
  const Arg arg;
} Key;

typedef struct {
  const char *symbol;
  void (*arrange)(Monitor *);
} Layout;

struct Monitor {
  char ltsymbol[16];  /* layout symbol */
  float mfact;
  int nmaster;
  int num;
  int by;             /* bar geometry */
  int mx, my, mw, mh; /* screen size */
  int wx, wy, ww, wh; /* window area  */
  int gappx;          /* gaps between windows */
  unsigned int seltags;
  unsigned int sellt;
  unsigned int tagset[2];
  int showbar;
  int topbar;
  Client *clients;
  Client *sel;
  Client *stack;
  Monitor *next;
  Window barwin;
  const Layout *lt[2];
};

typedef struct {
  const char *class;
  const char *instance;
  const char *title;
  unsigned int tags;
  int isfloating;
  int monitor;
} Rule;

typedef struct Systray Systray;
struct Systray {
  Window win;
  Client *icons;
};

/* function declarations */
static void apply_rules(Client *clientlient);
static int apply_size_hints(Client *clientlient, int *x, int *y, int *w, int *h, int interact);
static void arrange(Monitor *monitor);
static void arrange_mon(Monitor *monitor);
static void attach(Client *clientlient);
static void attach_stack(Client *client);
static void button_press(XEvent *event);
static void check_other_wm(void);
static void cleanup(void);
static void cleanup_mon(Monitor *monitoron);
static void client_message(XEvent *event);
static void configure(Client *client);
static void configure_notify(XEvent *event);
static void configure_request(XEvent *event);
static Monitor *create_monitor(void);
static void destroy_notify(XEvent *event);
static void detach(Client *client);
static void detach_stack(Client *client);
static Monitor *dir_to_mon(int dir);
static void draw_bar(Monitor *monitor);
static void draw_bars(void);
static void enter_notify(XEvent *event);
static void expose(XEvent *event);
static void focus(Client *client);
static void focusin(XEvent *event);
static void focus_monitor(const Arg *arg);
static void focus_stack(const Arg *arg);
static Atom get_atom_prop(Client *client, Atom prop);
static int get_root_ptr(int *x, int *y);
static long get_state(Window window);
static unsigned int get_systray_width();
static int get_text_prop(Window window, Atom atom, char *text, unsigned int size);
static void grab_buttons(Client *client, int focused);
static void grab_keys(void);
static void increment_nmaster(const Arg *arg);
static void key_press(XEvent *event);
static void kill_client(const Arg *arg);
static void manage(Window window, XWindowAttributes *wa);
static void mapping_notify(XEvent *event);
static void map_request(XEvent *event);
static void monocle(Monitor *monitor);
static void motion_notify(XEvent *event);
static void move_mouse(const Arg *arg);
static Client *next_tiled(Client *client);
static void pop(Client *client);
static void property_notify(XEvent *event);
static void quit(const Arg *arg);
static Monitor *rect_to_mon(int x, int y, int w, int h);
static void remove_systray_icon(Client *i);
static void resize(Client *client, int x, int y, int w, int h, int interact);
static void resize_bar_win(Monitor *monitor);
static void resize_client(Client *client, int x, int y, int w, int h);
static void resize_mouse(const Arg *arg);
static void resize_request(XEvent *event);
static void restack(Monitor *monitor);
static void run(void);
static void scan(void);
static int send_event(Window window, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);
static void send_montor(Client *client, Monitor *monitor);
static void set_client_state(Client *client, long state);
static void set_focus(Client *client);
static void set_full_screen(Client *client, int fullscreen);
static void set_gaps(const Arg *arg);
static void setlayout(const Arg *arg);
static void set_mfact(const Arg *arg);
static void setup(void);
static void set_urgent(Client *client, int urg);
static void show_hide(Client *client);
static void sighup(int unused);
static void sigterm(int unused);
static void spawn(const Arg *arg);
static Monitor *systray_to_monitor(Monitor *monitor);
static void tag(const Arg *arg);
static void tag_monitor(const Arg *arg);
static void tile(Monitor *monitor);
static void toggle_bar(const Arg *arg);
static void toggle_floating(const Arg *arg);
static void toggle_fullscreen(const Arg *arg);
static void toggle_tag(const Arg *arg);
static void toggle_view(const Arg *arg);
static void unfocus(Client *client, int setfocus);
static void unmanage(Client *client, int destroyed);
static void unmap_notify(XEvent *event);
static void update_bar_pos(Monitor *monitor);
static void update_bars(void);
static void update_client_list(void);
static int update_geom(void);
static void update_numlock_mask(void);
static void update_size_hints(Client *client);
static void update_status(void);
static void update_systray(void);
static void update_systray_icon_geom(Client *i, int w, int h);
static void update_systray_icon_state(Client *i, XPropertyEvent *ev);
static void update_title(Client *client);
static void update_window_type(Client *client);
static void update_wm_hints(Client *client);
static void view(const Arg *arg);
static Client *window_to_client(Window window);
static Monitor *window_to_monitor(Window window);
static Client *window_to_systray_icon(Window window);
static int xerror(Display *display, XErrorEvent *ee);
static int xerrordummy(Display *display, XErrorEvent *ee);
static int xerrorstart(Display *display, XErrorEvent *ee);
static void zoom(const Arg *arg);

/* variables */
static Systray *systray = NULL;
static const char broken[] = "broken";
static char stext[256];
static int screen;
static int sw, sh; /* X display screen geometry width, height */
static int bh;     /* bar height */
static int lrpad;  /* sum of left and right padding for text */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent])(XEvent *) = {
    [ButtonPress] = button_press,
    [ClientMessage] = client_message,
    [ConfigureRequest] = configure_request,
    [ConfigureNotify] = configure_notify,
    [DestroyNotify] = destroy_notify,
    [EnterNotify] = enter_notify,
    [Expose] = expose,
    [FocusIn] = focusin,
    [KeyPress] = key_press,
    [MappingNotify] = mapping_notify,
    [MapRequest] = map_request,
    [MotionNotify] = motion_notify,
    [PropertyNotify] = property_notify,
    [ResizeRequest] = resize_request,
    [UnmapNotify] = unmap_notify};
static Atom wmatom[WMLast], netatom[NetLast], xatom[XLast];
static int restart = 0;
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon;
static Window root, wmcheckwin;

/* configuration, allows nested code to access above variables */
#include "config.h"

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags {
  char limitexceeded[LENGTH(tags) > 31 ? -1 : 1];
};

/* function implementations */
void apply_rules(Client *client) {
  const char *class, *instance;
  unsigned int i;
  const Rule *r;
  Monitor *m;
  XClassHint ch = {NULL, NULL};

  /* rule matching */
  client->isfloating = 0;
  client->tags = 0;
  XGetClassHint(dpy, client->win, &ch);
  class = ch.res_class ? ch.res_class : broken;
  instance = ch.res_name ? ch.res_name : broken;

  for (i = 0; i < LENGTH(rules); i++) {
    r = &rules[i];
    if ((!r->title || strstr(client->name, r->title)) && (!r->class || strstr(class, r->class)) && (!r->instance || strstr(instance, r->instance))) {
      client->isfloating = r->isfloating;
      client->tags |= r->tags;
      for (m = mons; m && m->num != r->monitor; m = m->next);
      if (m){
        client->mon = m;
	    }
    }
  }
  if (ch.res_class){
    XFree(ch.res_class);
  }
  if (ch.res_name){
    XFree(ch.res_name);
  }
  client->tags = client->tags & TAGMASK ? client->tags & TAGMASK : client->mon->tagset[client->mon->seltags];
}

int apply_size_hints(Client *client, int *x, int *y, int *width, int *height, int interact) {
  int baseismin;
  Monitor *m = client->mon;

  /* set minimum possible */
  *width = MAX(1, *width);
  *height = MAX(1, *height);
  if (interact) {
    if (*x > screen_width){
      *x = screen_width - WIDTH(client);
	  }
    if (*y > screen_height){
      *y = screen_height - HEIGHT(client);
	  }
    if (*x + *width + 2 * client->bw < 0){
      *x = 0;
	  }
    if (*y + *height + 2 * client->bw < 0){
      *y = 0;
	  }
  } else {
    if (*x >= m->wx + m->ww){
      *x = m->wx + m->ww - WIDTH(client);
	  }
    if (*y >= m->wy + m->wh){
      *y = m->wy + m->wh - HEIGHT(client);
	  }
    if (*x + *width + 2 * client->bw <= m->wx){
      *x = m->wx;
	  }
    if (*y + *height + 2 * client->bw <= m->wy){
      *y = m->wy;
	  }
  }
  if (*height < bh){
    *height = bh;
  }
  if (*width < bh){
    *width = bh;
  }
  if (resizehints || client->isfloating || !client->mon->lt[client->mon->sellt]->arrange) {
    if (!client->hintsvalid){
      update_size_hints(client);
	  }
    /* see last two sentences in ICCCM 4.1.2.3 */
    baseismin = client->basew == client->minw && client->baseh == client->minh;

    /* temporarily remove base dimensions */
    if (!baseismin) {
      *width -= client->basew;
      *height -= client->baseh;
    }
    /* adjust for aspect limits */
    if (client->mina > 0 && client->maxa > 0) {
      if (client->maxa < (float)*width / *height)
        *width = *height * client->maxa + 0.5;
      else if (client->mina < (float)*height / *width)
        *height = *width * client->mina + 0.5;
    }
    if (baseismin) { /* increment calculation requires this */
      *width -= client->basew;
      *height -= client->baseh;
    }
    /* adjust for increment value */
    if (client->incw){
      *width -= *width % client->incw;
	  }
    if (client->inch){
      *height -= *height % client->inch;
	  }
    /* restore base dimensions */
    *width = MAX(*width + client->basew, client->minw);
    *height = MAX(*height + client->baseh, client->minh);
    if (client->maxw){
      *width = MIN(*width, client->maxw);
	  }
    if (client->maxh){
      *height = MIN(*height, client->maxh);
	  }
  }
  return *x != client->x || *y != client->y || *width != client->w || *height != client->h;
}

void arrange(Monitor *monitor) {
  if (monitor){
    show_hide(monitor->stack);
  } else {
    for (monitor = mons; monitor; monitor = monitor->next){
      show_hide(monitor->stack);
	  }
  }
  if (monitor) {
    arrange_mon(monitor);
    restack(monitor);
  } else {
    for (monitor = mons; monitor; monitor = monitor->next){
      arrange_mon(monitor);
	  }
  }
}

void arrange_mon(Monitor *monitor) {
  strncpy(monitor->ltsymbol, monitor->lt[monitor->sellt]->symbol, sizeof monitor->ltsymbol);
  if (monitor->lt[monitor->sellt]->arrange){
    monitor->lt[monitor->sellt]->arrange(monitor);
  }
}

void attach(Client *client) {
  client->next = client->mon->clients;
  client->mon->clients = client;
}

void attach_stack(Client *client) {
  client->snext = client->mon->stack;
  client->mon->stack = client;
}

void button_press(XEvent *event) {
  unsigned int i, x, click;
  Arg arg = {0};
  Client *c;
  Monitor *m;
  XButtonPressedEvent *ev = &event->xbutton;

  click = ClkRootWin;
  /* focus monitor if necessary */
  if ((m = window_to_monitor(ev->window)) && m != selmon) {
    unfocus(selmon->sel, 1);
    selmon = m;
    focus(NULL);
  }
  if (ev->window == selmon->barwin) {
    i = x = 0;
    do {
      x += TEXTW(tags[i]);
	}
    while (ev->x >= x && ++i < LENGTH(tags));

    if (i < LENGTH(tags)) {
      click = ClkTagBar;
      arg.ui = 1 << i;
    } else if (ev->x < x + TEXTW(selmon->ltsymbol)){
      click = ClkLtSymbol;
	}
    else if (ev->x > selmon->ww - (int)TEXTW(stext) - get_systray_width()){
      click = ClkStatusText;
	} else {
      click = ClkWinTitle;
	}
  } else if ((c = window_to_client(ev->window))) {
    focus(c);
    restack(selmon);
    XAllowEvents(dpy, ReplayPointer, CurrentTime);
    click = ClkClientWin;
  }
  for (i = 0; i < LENGTH(buttons); i++){
    if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state)){
      buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
	  }
  }
}

void check_other_wm(void) {
  xerrorxlib = XSetErrorHandler(xerrorstart);
  /* this causes an error if some other window manager is running */
  XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
  XSync(dpy, False);
  XSetErrorHandler(xerror);
  XSync(dpy, False);
}

void cleanup(void) {
  Arg a = {.ui = ~0};
  Layout foo = {"", NULL};
  Monitor *m;
  size_t i;

  view(&a);
  selmon->lt[selmon->sellt] = &foo;
  for (m = mons; m; m = m->next){
    while (m->stack){
      unmanage(m->stack, 0);
	  }
  }
  XUngrabKey(dpy, AnyKey, AnyModifier, root);
  while (mons){
    cleanup_mon(mons);
  }

  if (showsystray) {
    XUnmapWindow(dpy, systray->win);
    XDestroyWindow(dpy, systray->win);
    free(systray);
  }

  for (i = 0; i < CurLast; i++){
    drw_cur_free(drw, cursor[i]);
  }

  for (i = 0; i < LENGTH(colors); i++){
    free(scheme[i]);
  }

  free(scheme);
  XDestroyWindow(dpy, wmcheckwin);
  drw_free(drw);
  XSync(dpy, False);
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
  XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}

void cleanup_mon(Monitor *monitor) {
  Monitor *m;

  if (monitor == mons){
    mons = mons->next;
  }
  else {
    for (m = mons; m && m->next != monitor; m = m->next);
    m->next = monitor->next;
  }
  XUnmapWindow(dpy, monitor->barwin);
  XDestroyWindow(dpy, monitor->barwin);
  free(monitor);
}

void client_message(XEvent *event) {
  XWindowAttributes wa;
  XSetWindowAttributes swa;
  XClientMessageEvent *cme = &event->xclient;
  Client *c = window_to_client(cme->window);

  if (showsystray && cme->window == systray->win && cme->message_type == netatom[NetSystemTrayOP]) {
    /* add systray icons */
    if (cme->data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {
      if (!(c = (Client *)calloc(1, sizeof(Client)))){
        die("fatal: could not malloc() %u bytes\n", sizeof(Client));
	    }
      if (!(c->win = cme->data.l[2])) {
        free(c);
        return;
      }
      c->mon = selmon;
      c->next = systray->icons;
      systray->icons = c;
      if (!XGetWindowAttributes(dpy, c->win, &wa)) {
        /* use sane defaults */
        wa.width = bh;
        wa.height = bh;
        wa.border_width = 0;
      }
      c->x = c->oldx = c->y = c->oldy = 0;
      c->w = c->oldw = wa.width;
      c->h = c->oldh = wa.height;
      c->oldbw = wa.border_width;
      c->bw = 0;
      c->isfloating = True;
      /* reuse tags field as mapped status */
      c->tags = 1;
      update_size_hints(c);
      update_systray_icon_geom(c, wa.width, wa.height);
      XAddToSaveSet(dpy, c->win);
      XSelectInput(dpy, c->win, StructureNotifyMask | PropertyChangeMask | ResizeRedirectMask);
      XReparentWindow(dpy, c->win, systray->win, 0, 0);
      /* use parents background color */
      swa.background_pixel = scheme[SchemeNorm][ColBg].pixel;
      XChangeWindowAttributes(dpy, c->win, CWBackPixel, &swa);
      send_event(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0, systray->win, XEMBED_EMBEDDED_VERSION);
      /* FIXME not sure if I have to send these events, too */
      send_event(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_FOCUS_IN, 0, systray->win, XEMBED_EMBEDDED_VERSION);
      send_event(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, systray->win, XEMBED_EMBEDDED_VERSION);
      send_event(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_MODALITY_ON, 0, systray->win, XEMBED_EMBEDDED_VERSION);
      XSync(dpy, False);
      resize_bar_win(selmon);
      update_systray();
      set_client_state(c, NormalState);
    }
    return;
  }

  if (!c){
    return;
  }
  if (cme->message_type == netatom[NetWMState]) {
    if (cme->data.l[1] == netatom[NetWMFullscreen] || cme->data.l[2] == netatom[NetWMFullscreen]){
      set_full_screen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
                        || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ &&
                            !c->isfullscreen)));
	  }
  } else if (cme->message_type == netatom[NetActiveWindow]) {
    if (c != selmon->sel && !c->isurgent){
      set_urgent(c, 1);
	  }
  }
}

void configure(Client *client) {
  XConfigureEvent ce;

  ce.type = ConfigureNotify;
  ce.display = dpy;
  ce.event = client->win;
  ce.window = client->win;
  ce.x = client->x;
  ce.y = client->y;
  ce.width = client->w;
  ce.height = client->h;
  ce.border_width = client->bw;
  ce.above = None;
  ce.override_redirect = False;
  XSendEvent(dpy, client->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void configure_notify(XEvent *event) {
  Monitor *m;
  Client *c;
  XConfigureEvent *ev = &event->xconfigure;
  int dirty;

  /* TODO: update_geom handling sucks, needs to be simplified */
  if (ev->window == root) {
    dirty = (screen_width != ev->width || screen_height != ev->height);
    screen_width = ev->width;
    screen_height = ev->height;
    if (update_geom() || dirty) {
      drw_resize(drw, screen_width, bh);
      update_bars();
      for (m = mons; m; m = m->next) {
        for (c = m->clients; c; c = c->next){
          if (c->isfullscreen){
            resize_client(c, m->mx, m->my, m->mw, m->mh);
		      }
		    }

        resize_bar_win(m);
      }

      focus(NULL);
      arrange(NULL);
    }
  }
}

void configure_request(XEvent *event) {
  Client *c;
  Monitor *m;
  XConfigureRequestEvent *ev = &event->xconfigurerequest;
  XWindowChanges wc;

  if ((c = window_to_client(ev->window))) {
    if (ev->value_mask & CWBorderWidth){
      c->bw = ev->border_width;
	  }
    else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
      m = c->mon;
      if (ev->value_mask & CWX) {
        c->oldx = c->x;
        c->x = m->mx + ev->x;
      }
      if (ev->value_mask & CWY) {
        c->oldy = c->y;
        c->y = m->my + ev->y;
      }
      if (ev->value_mask & CWWidth) {
        c->oldw = c->w;
        c->w = ev->width;
      }
      if (ev->value_mask & CWHeight) {
        c->oldh = c->h;
        c->h = ev->height;
      }
      if ((c->x + c->w) > m->mx + m->mw && c->isfloating){
        c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
	  }
      if ((c->y + c->h) > m->my + m->mh && c->isfloating){
        c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
	  }
      if ((ev->value_mask & (CWX | CWY)) && !(ev->value_mask & (CWWidth | CWHeight))){
        configure(c);
	  }
      if (ISVISIBLE(c)){
        XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
	  }
    } else {
      configure(c);
	  }
  } else {
    wc.x = ev->x;
    wc.y = ev->y;
    wc.width = ev->width;
    wc.height = ev->height;
    wc.border_width = ev->border_width;
    wc.sibling = ev->above;
    wc.stack_mode = ev->detail;
    XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
  }
  XSync(dpy, False);
}

Monitor *create_monitor(void) {
  Monitor *m;

  m = ecalloc(1, sizeof(Monitor));
  m->tagset[0] = m->tagset[1] = 1;
  m->mfact = mfact;
  m->nmaster = nmaster;
  m->showbar = showbar;
  m->topbar = topbar;
  m->gappx = gappx;
  m->lt[0] = &layouts[0];
  m->lt[1] = &layouts[1 % LENGTH(layouts)];
  strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
  return m;
}

void destroy_notify(XEvent *event) {
  Client *c;
  XDestroyWindowEvent *ev = &event->xdestroywindow;

  if ((c = window_to_client(ev->window))){
    unmanage(c, 1);
  }
  else if ((c = window_to_systray_icon(ev->window))) {
    remove_systray_icon(c);
    resize_bar_win(selmon);
    update_systray();
  }
}

void detach(Client *client) {
  Client **tc;

  for (tc = &client->mon->clients; *tc && *tc != client; tc = &(*tc)->next);
  *tc = client->next;
}

void detach_stack(Client *client) {
  Client **tc, *t;

  for (tc = &client->mon->stack; *tc && *tc != client; tc = &(*tc)->snext);
  *tc = client->snext;

  if (client == client->mon->sel) {
    for (t = client->mon->stack; t && !ISVISIBLE(t); t = t->snext);
    client->mon->sel = t;
  }
}

Monitor *dir_to_mon(int dir) {
  Monitor *m = NULL;

  if (dir > 0) {
    if (!(m = selmon->next)){
      m = mons;
	  }
  } else if (selmon == mons){
    for (m = mons; m->next; m = m->next);
  }
  else {
    for (m = mons; m->next != selmon; m = m->next);
  }

  return m;
}

void draw_bar(Monitor *monitor) {
  int x, w, tw = 0, trayWidth = 0;
  int boxs = drw->fonts->h / 9;
  int boxw = drw->fonts->h / 6 + 2;
  unsigned int i, occ = 0, urg = 0;
  Client *c;

  if (!monitor->showbar){
    return;
  }

  if (showsystray && monitor == systray_to_monitor(monitor) && !systrayonleft){
    trayWidth = get_systray_width();
  }

  /* draw status first so it can be overdrawn by tags later */
  drw_setscheme(drw, scheme[SchemeNorm]);
  tw = TEXTW(stext) - lrpad / 2 + 2; /* 2px extra right padding */
  drw_text(drw, monitor->ww - tw - trayWidth, 0, tw, bh, lrpad / 2 - 2, stext, 0);

  resize_bar_win(monitor);

  // mark urgent tags
  for (c = monitor->clients; c; c = c->next) {
    occ |= c->tags;
    if (c->isurgent){
      urg |= c->tags;
	  }
  }

  // render tags
  x = 0;
  for (i = 0; i < LENGTH(tags); i++) {
    w = TEXTW(tags[i]);

    // render the tag
    drw_setscheme(drw, scheme[monitor->tagset[monitor->seltags] & 1 << i ? SchemeSel : SchemeNorm]);
    drw_text(drw, x, 0, w, bh, lrpad / 2, tags[i], urg & 1 << i);

    // invert tag if urgent
    if (occ & 1 << i) {
      drw_rect(drw, x + boxs, boxs, boxw, boxw, monitor == selmon && selmon->sel && selmon->sel->tags & 1 << i, urg & 1 << i);
	  }

    // set position for next tag
    x += w;
  }

  // render layout
  w = TEXTW(monitor->ltsymbol);
  drw_setscheme(drw, scheme[SchemeNorm]);
  x = drw_text(drw, x, 0, w, bh, lrpad / 2, monitor->ltsymbol, 0);

  // render title of last highlighted client
  if ((w = monitor->ww - tw - trayWidth - x) > bh) {
    if (monitor->sel) {
      drw_setscheme(drw, scheme[monitor == selmon ? SchemeSel : SchemeNorm]);
      drw_text(drw, x, 0, w, bh, lrpad / 2, monitor->sel->name, 0);

      // render the small indicator when client is floating
      if (monitor->sel->isfloating){
        drw_rect(drw, x + boxs, boxs, boxw, boxw, monitor->sel->isfixed, 0);
	    }
    } else {
      drw_setscheme(drw, scheme[SchemeNorm]);
      drw_rect(drw, x, 0, w, bh, 1, 1);
    }
  }

  // not sure what this does...
  drw_map(drw, monitor->barwin, 0, 0, monitor->ww - trayWidth, bh);
}

void draw_bars(void) {
  Monitor *m;

  for (m = mons; m; m = m->next){
    draw_bar(m);
  }
}

void enter_notify(XEvent *event) {
  Client *c;
  Monitor *m;
  XCrossingEvent *ev = &event->xcrossing;

  if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root) {
    return;
  }

  c = window_to_client(ev->window);
  m = c ? c->mon : window_to_monitor(ev->window);
  if (m != selmon) {
    unfocus(selmon->sel, 1);
    selmon = m;
  } else if (!c || c == selmon->sel) {
    return;
  }
  focus(c);
}

void expose(XEvent *event) {
  Monitor *m;
  XExposeEvent *ev = &event->xexpose;

  if (ev->count == 0 && (m = window_to_monitor(ev->window))) {
    draw_bar(m);
    if (m == selmon){
      update_systray();
	  }
  }
}

void focus(Client *client) {
  if (!client || !ISVISIBLE(client)) {
    for (client = selmon->stack; client && !ISVISIBLE(client); client = client->snext);
  }
  if (selmon->sel && selmon->sel != client) {
    unfocus(selmon->sel, 0);
  }
  if (client) {
    if (client->mon != selmon) {
      selmon = client->mon;
	  }
    if (client->isurgent) {
      set_urgent(client, 0);
	  }

    detach_stack(client);
    attach_stack(client);
    grab_buttons(client, 1);
    XSetWindowBorder(dpy, client->win, scheme[SchemeSel][ColBorder].pixel);
    set_focus(client);
  } else {
    XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
  }
  selmon->sel = client;
  draw_bars();
}

/* there are some broken focus acquiring clients needing extra handling */
void focusin(XEvent *event) {
  XFocusChangeEvent *ev = &event->xfocus;

  if (selmon->sel && ev->window != selmon->sel->win) {
    set_focus(selmon->sel);
  }
}

void focus_monitor(const Arg *arg) {
  Monitor *m;

  if (!mons->next) {
    return;
  }
  if ((m = dir_to_mon(arg->i)) == selmon) {
    return;
  }

  unfocus(selmon->sel, 0);
  selmon = m;
  focus(NULL);
}

void focus_stack(const Arg *arg) {
  Client *c = NULL, *i;

  if (!selmon->sel || (selmon->sel->isfullscreen && lockfullscreen)) {
    return;
  }

  if (arg->i > 0) {
    for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
    if (!c){
      for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
    }
  } else {
    for (i = selmon->clients; i != selmon->sel; i = i->next){
      if (ISVISIBLE(i)){
        c = i;
      }
    }
    if (!c) {
      for (; i; i = i->next) {
        if (ISVISIBLE(i)) {
          c = i;
        }
      }
    }
  }
  if (c) {
    focus(c);
    restack(selmon);
  }
}

Atom get_atom_prop(Client *client, Atom prop) {
  int di;
  unsigned long dl;
  unsigned char *p = NULL;
  Atom da, atom = None;

  /* FIXME getatomprop should return the number of items and a pointer to
   * the stored data instead of this workaround */
  Atom req = XA_ATOM;
  if (prop == xatom[XembedInfo]){
    req = xatom[XembedInfo];
  }

  if (XGetWindowProperty(dpy, client->win, prop, 0L, sizeof atom, False, req, &da, &di, &dl, &dl, &p) == Success && p) {
    atom = *(Atom *)p;
    if (da == xatom[XembedInfo] && dl == 2) {
      atom = ((Atom *)p)[1];
	  }

    XFree(p);
  }

  return atom;
}

unsigned int get_systray_width() {
  unsigned int w = 0;
  Client *i;

  if (showsystray) {
    for (i = systray->icons; i; w += i->w + systrayspacing, i = i->next);
  }

  return w ? w + systrayspacing : 1;
}

int get_root_ptr(int *x, int *y) {
  int di;
  unsigned int dui;
  Window dummy;

  return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long get_state(Window window) {
  int format;
  long result = -1;
  unsigned char *p = NULL;
  unsigned long n, extra;
  Atom real;

  if (XGetWindowProperty(dpy, window, wmatom[WMState], 0L, 2L, False, wmatom[WMState], &real, &format, &n, &extra, (unsigned char **)&p) != Success) {
    return -1;
  }

  if (n != 0) {
    result = *p;
  }

  XFree(p);
  return result;
}

int get_text_prop(Window window, Atom atom, char *text, unsigned int size) {
  char **list = NULL;
  int n;
  XTextProperty name;

  if (!text || size == 0) {
    return 0;
  }

  text[0] = '\0';
  if (!XGetTextProperty(dpy, window, &name, atom) || !name.nitems) {
    return 0;
  }

  if (name.encoding == XA_STRING) {
    strncpy(text, (char *)name.value, size - 1);
  } else if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
    strncpy(text, *list, size - 1);
    XFreeStringList(list);
  }
  text[size - 1] = '\0';
  XFree(name.value);
  return 1;
}

void grab_buttons(Client *client, int focused) {
  update_numlock_mask();
  {
    unsigned int i, j;
    unsigned int modifiers[] = {0, LockMask, numlockmask, numlockmask | LockMask};
    XUngrabButton(dpy, AnyButton, AnyModifier, client->win);
    if (!focused){
      XGrabButton(dpy, AnyButton, AnyModifier, client->win, False, BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
	  }

    for (i = 0; i < LENGTH(buttons); i++){
      if (buttons[i].click == ClkClientWin){
        for (j = 0; j < LENGTH(modifiers); j++) {
          XGrabButton(dpy, buttons[i].button, buttons[i].mask | modifiers[j], client->win, False, BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
		    }
      }
	  }
  }
}

void grab_keys(void) {
  update_numlock_mask();
  {
    unsigned int i, j, k;
    unsigned int modifiers[] = {0, LockMask, numlockmask, numlockmask | LockMask};
    int start, end, skip;
    KeySym *syms;

    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    XDisplayKeycodes(dpy, &start, &end);
    syms = XGetKeyboardMapping(dpy, start, end - start + 1, &skip);
    if (!syms) {
      return;
	  }

    for (k = start; k <= end; k++){
      for (i = 0; i < LENGTH(keys); i++){
        /* skip modifier codes, we do that ourselves */
        if (keys[i].keysym == syms[(k - start) * skip]){
          for (j = 0; j < LENGTH(modifiers); j++){
            XGrabKey(dpy, k, keys[i].mod | modifiers[j], root, True, GrabModeAsync, GrabModeAsync);
		      }
		    }
	    }
	  }

    XFree(syms);
  }
}

void increment_nmaster(const Arg *arg) {
  selmon->nmaster = MAX(selmon->nmaster + arg->i, 0);
  arrange(selmon);
}

#ifdef XINERAMA
static int is_unique_geom(XineramaScreenInfo *unique, size_t n,
                        XineramaScreenInfo *info) {
  while (n--)
    if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org &&
        unique[n].width == info->width && unique[n].height == info->height)
      return 0;
  return 1;
}
#endif /* XINERAMA */

void key_press(XEvent *event) {
  unsigned int i;
  KeySym keysym;
  XKeyEvent *ev;

  ev = &event->xkey;
  keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
  for (i = 0; i < LENGTH(keys); i++){
    if (keysym == keys[i].keysym && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state) && keys[i].func){
      keys[i].func(&(keys[i].arg));
	  }
  }
}

void kill_client(const Arg *arg) {
  if (!selmon->sel) {
    return;
  }

  if (!send_event(selmon->sel->win, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0, 0, 0)) {
    XGrabServer(dpy);
    XSetErrorHandler(xerrordummy);
    XSetCloseDownMode(dpy, DestroyAll);
    XKillClient(dpy, selmon->sel->win);
    XSync(dpy, False);
    XSetErrorHandler(xerror);
    XUngrabServer(dpy);
  }
}

void manage(Window window, XWindowAttributes *window_attributes) {
  Client *c, *t = NULL;
  Window trans = None;
  XWindowChanges wc;

  c = ecalloc(1, sizeof(Client));
  c->win = window;
  /* geometry */
  c->x = c->oldx = window_attributes->x;
  c->y = c->oldy = window_attributes->y;
  c->w = c->oldw = window_attributes->width;
  c->h = c->oldh = window_attributes->height;
  c->oldbw = window_attributes->border_width;

  update_title(c);
  if (XGetTransientForHint(dpy, window, &trans) && (t = window_to_client(trans))){
    c->mon = t->mon;
    c->tags = t->tags;
  } else {
    c->mon = selmon;
    apply_rules(c);
  }

  if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww){
    c->x = c->mon->wx + c->mon->ww - WIDTH(c);
  }
  if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh){
    c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
  }
  c->x = MAX(c->x, c->mon->wx);
  c->y = MAX(c->y, c->mon->wy);
  c->bw = borderpx;

  wc.border_width = c->bw;
  XConfigureWindow(dpy, window, CWBorderWidth, &wc);
  XSetWindowBorder(dpy, window, scheme[SchemeNorm][ColBorder].pixel);
  configure(c); /* propagates border_width, if size doesn't change */
  update_window_type(c);
  update_size_hints(c);
  update_wm_hints(c);

  // set windows to center (patch)
  c->x = c->mon->mx + (c->mon->mw - WIDTH(c)) / 2;
  c->y = c->mon->my + (c->mon->mh - HEIGHT(c)) / 2;

  XSelectInput(dpy, window, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
  grab_buttons(c, 0);
  if (!c->isfloating) {
    c->isfloating = c->oldstate = trans != None || c->isfixed;
  }
  if (c->isfloating) {
    XRaiseWindow(dpy, c->win);
  }
  attach(c);
  attach_stack(c);
  XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->win), 1);
  XMoveResizeWindow(dpy, c->win, c->x + 2 * screen_width, c->y, c->w, c->h); /* some windows require this */
  set_client_state(c, NormalState);
  if (c->mon == selmon) {
    unfocus(selmon->sel, 0);
  }
  c->mon->sel = c;
  arrange(c->mon);
  XMapWindow(dpy, c->win);
  focus(NULL);
}

void mapping_notify(XEvent *event) {
  XMappingEvent *ev = &event->xmapping;

  XRefreshKeyboardMapping(ev);
  if (ev->request == MappingKeyboard) {
    grab_keys();
  }
}

void map_request(XEvent *event) {
  static XWindowAttributes wa;
  XMapRequestEvent *ev = &event->xmaprequest;

  Client *i;
  if ((i = window_to_systray_icon(ev->window))) {
    send_event(i->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, systray->win, XEMBED_EMBEDDED_VERSION);
    resize_bar_win(selmon);
    update_systray();
  }

  if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect) {
    return;
  }
  if (!window_to_client(ev->window)) {
    manage(ev->window, &wa);
  }
}

void monocle(Monitor *monitor) {
	unsigned int n = 0;
	Client *c;

	for (c = monitor->clients; c; c = c->next)
		if (ISVISIBLE(c))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(monitor->ltsymbol, sizeof monitor->ltsymbol, "[%d]", n);
	for (c = next_tiled(monitor->clients); c; c = next_tiled(c->next))
		resize(c, monitor->wx, monitor->wy, monitor->ww - 2 * c->bw, monitor->wh - 2 * c->bw, 0);
}

void motion_notify(XEvent *event) {
  static Monitor *mon = NULL;
  Monitor *m;
  XMotionEvent *ev = &event->xmotion;

  if (ev->window != root) {
    return;
  }
  if ((m = rect_to_mon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
    unfocus(selmon->sel, 1);
    selmon = m;
    focus(NULL);
  }
  mon = m;
}

void move_mouse(const Arg *arg) {
  int x, y, ocx, ocy, nx, ny;
  Client *c;
  Monitor *m;
  XEvent ev;
  Time lasttime = 0;

  if (!(c = selmon->sel)) {
    return;
  }

  /* no support moving fullscreen windows by mouse */
  if (c->isfullscreen){
    return;
  }

  restack(selmon);
  ocx = c->x;
  ocy = c->y;
  if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess) {
    return;
  }

  if (!get_root_ptr(&x, &y)) {
    return;
  }

  do {
    XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
    switch (ev.type) {
    case ConfigureRequest:
    case Expose:
    case MapRequest:
      handler[ev.type](&ev);
      break;
    case MotionNotify:
      if ((ev.xmotion.time - lasttime) <= (1000 / 60)) {
        continue;
	    }
      lasttime = ev.xmotion.time;

      nx = ocx + (ev.xmotion.x - x);
      ny = ocy + (ev.xmotion.y - y);
      if (abs(selmon->wx - nx) < snap) {
        nx = selmon->wx;
	    }
      else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap) {
        nx = selmon->wx + selmon->ww - WIDTH(c);
	    }
      if (abs(selmon->wy - ny) < snap) {
        ny = selmon->wy;
	    }
      else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap) {
        ny = selmon->wy + selmon->wh - HEIGHT(c);
	    }
      if (!c->isfloating && selmon->lt[selmon->sellt]->arrange && (abs(nx - c->x) > snap || abs(ny - c->y) > snap)) {
        toggle_floating(NULL);
	    }
      if (!selmon->lt[selmon->sellt]->arrange || c->isfloating) {
        resize(c, nx, ny, c->w, c->h, 1);
	    }

      break;
    }
  } while (ev.type != ButtonRelease);

  XUngrabPointer(dpy, CurrentTime);
  if ((m = rect_to_mon(c->x, c->y, c->w, c->h)) != selmon) {
    send_montor(c, m);
    selmon = m;
    focus(NULL);
  }
}

Client *next_tiled(Client *client) {
  for (; client && (client->isfloating || !ISVISIBLE(client)); client = client->next);
  return client;
}

void pop(Client *client) {
  detach(client);
  attach(client);
  focus(client);
  arrange(client->mon);
}

void property_notify(XEvent *event) {
  Client *c;
  Window trans;
  XPropertyEvent *ev = &event->xproperty;

  if ((c = window_to_systray_icon(ev->window))) {
    if (ev->atom == XA_WM_NORMAL_HINTS) {
      update_size_hints(c);
      update_systray_icon_geom(c, c->w, c->h);
    } else {
      update_systray_icon_state(c, ev);
	}

    resize_bar_win(selmon);
    update_systray();
  }

  if ((ev->window == root) && (ev->atom == XA_WM_NAME)) {
    update_status();
  }
  else if (ev->state == PropertyDelete) {
    return; /* ignore */
  } else if ((c = window_to_client(ev->window))) {
    switch (ev->atom) {
    default:
      break;
    case XA_WM_TRANSIENT_FOR:
      if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) && (c->isfloating = (window_to_client(trans)) != NULL)) arrange(c->mon);
      break;
    case XA_WM_NORMAL_HINTS:
      c->hintsvalid = 0;
      break;
    case XA_WM_HINTS:
      update_wm_hints(c);
      draw_bars();
      break;
    }
    if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
      update_title(c);
      if (c == c->mon->sel) {
        draw_bar(c->mon);
	  }
    }
    if (ev->atom == netatom[NetWMWindowType]) {
      update_window_type(c);
	  }
  }
}

void quit(const Arg *arg) {
  if (arg->i) {
    restart = 1;
  }

  running = 0;
}

Monitor *rect_to_mon(int x, int y, int width, int height) {
  Monitor *m, *r = selmon;
  int a, area = 0;

  for (m = mons; m; m = m->next) {
    if ((a = INTERSECT(x, y, width, height, m)) > area) {
      area = a;
      r = m;
    }
  }

  return r;
}

void remove_systray_icon(Client *client) {
  Client **ii;

  if (!showsystray || !client) {
    return;
  }
  for (ii = &systray->icons; *ii && *ii != client; ii = &(*ii)->next);
  if (ii) {
    *ii = client->next;
  }
  free(client);
}

void resize(Client *client, int x, int y, int width, int height, int interact) {
  if (apply_size_hints(client, &x, &y, &width, &height, interact)) {
    resize_client(client, x, y, width, height);
  }
}

void resize_bar_win(Monitor *monitor) {
  unsigned int w = monitor->ww;

  if (showsystray && monitor == systray_to_monitor(monitor) && !systrayonleft) {
    w -= get_systray_width();
  }

  XMoveResizeWindow(dpy, monitor->barwin, monitor->wx, monitor->by, w, bh);
}

void resize_client(Client *client, int x, int y, int width, int height) {
  XWindowChanges wc;

  client->oldx = client->x;
  client->x = wc.x = x;
  client->oldy = client->y;
  client->y = wc.y = y;
  client->oldw = client->w;
  client->w = wc.width = width;
  client->oldh = client->h;
  client->h = wc.height = height;
  wc.border_width = client->bw;
  XConfigureWindow(dpy, client->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
  configure(client);
  XSync(dpy, False);
}

void resize_request(XEvent *event) {
  XResizeRequestEvent *ev = &event->xresizerequest;
  Client *i;

  if ((i = window_to_systray_icon(ev->window))) {
    update_systray_icon_geom(i, ev->width, ev->height);
    resize_bar_win(selmon);
    update_systray();
  }
}

void resize_mouse(const Arg *arg) {
  int ocx, ocy, nw, nh;
  Client *c;
  Monitor *m;
  XEvent ev;
  Time lasttime = 0;

  if (!(c = selmon->sel)) {
    return;
  }

  /* no support resizing fullscreen windows by mouse */
  if (c->isfullscreen) {
    return;
  }

  restack(selmon);
  ocx = c->x;
  ocy = c->y;
  if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess) {
    return;
  }

  XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
  do {
    XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
    switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60)){
				continue;
			}
			lasttime = ev.xmotion.time;

			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
			if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww && c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh) {
				if (!c->isfloating && selmon->lt[selmon->sellt]->arrange && (abs(nw - c->w) > snap || abs(nh - c->h) > snap)) {
				toggle_floating(NULL);
				}
			}
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating) {
				resize(c, c->x, c->y, nw, nh, 1);
			}

			break;
    }
  } while (ev.type != ButtonRelease);
  XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
  XUngrabPointer(dpy, CurrentTime);
  while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
  if ((m = rect_to_mon(c->x, c->y, c->w, c->h)) != selmon) {
    send_montor(c, m);
    selmon = m;
    focus(NULL);
  }
}

void restack(Monitor *monitor) {
  Client *c;
  XEvent ev;
  XWindowChanges wc;

  draw_bar(monitor);
  if (!monitor->sel){
    return;
  }

  if (monitor->sel->isfloating || !monitor->lt[monitor->sellt]->arrange){
    XRaiseWindow(dpy, monitor->sel->win);
  }
  if (monitor->lt[monitor->sellt]->arrange) {
    wc.stack_mode = Below;
    wc.sibling = monitor->barwin;
    for (c = monitor->stack; c; c = c->snext) {
      if (!c->isfloating && ISVISIBLE(c)) {
        XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
        wc.sibling = c->win;
      }
	  }
  }
  XSync(dpy, False);
  while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void run(void) {
  XEvent ev;
  /* main event loop */
  XSync(dpy, False);
  while (running && !XNextEvent(dpy, &ev)) {
    if (handler[ev.type]) {
      handler[ev.type](&ev); /* call handler */
	  }
  }
}

void scan(void) {
  unsigned int i, num;
  Window d1, d2, *wins = NULL;
  XWindowAttributes wa;

  if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
    for (i = 0; i < num; i++) {
      if (!XGetWindowAttributes(dpy, wins[i], &wa) || wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1)) {
        continue;
	    }
      if (wa.map_state == IsViewable || get_state(wins[i]) == IconicState) {
        manage(wins[i], &wa);
	    }
    }

    /* now the transients */
    for (i = 0; i < num; i++) {
      if (!XGetWindowAttributes(dpy, wins[i], &wa)) {
        continue;
	    }
      if (XGetTransientForHint(dpy, wins[i], &d1) && (wa.map_state == IsViewable || get_state(wins[i]) == IconicState)) {
        manage(wins[i], &wa);
	    }
    }
    if (wins) {
      XFree(wins);
	}
  }
}

void send_montor(Client *client, Monitor *monitor){
  if (client->mon == monitor) {
    return;
  }

  unfocus(client, 1);
  detach(client);
  detach_stack(client);
  client->mon = monitor;
  client->tags = monitor->tagset[monitor->seltags]; /* assign tags of target monitor */
  attach(client);
  attach_stack(client);
  focus(NULL);
  arrange(NULL);
}

void set_client_state(Client *client, long state){
  long data[] = {state, None};

  XChangeProperty(dpy, client->win, wmatom[WMState], wmatom[WMState], 32, PropModeReplace, (unsigned char *)data, 2);
}

int send_event(Window window, Atom proto, int mask, long d0, long d1, long d2, long d3, long d4) {
  int n;
  Atom *protocols, mt;
  int exists = 0;
  XEvent ev;

  if (proto == wmatom[WMTakeFocus] || proto == wmatom[WMDelete]) {
    mt = wmatom[WMProtocols];
    if (XGetWMProtocols(dpy, window, &protocols, &n)) {
      while (!exists && n--){
        exists = protocols[n] == proto;
	    }
      XFree(protocols);
    }
  } else {
    exists = True;
    mt = proto;
  }

  if (exists) {
    ev.type = ClientMessage;
    ev.xclient.window = window;
    ev.xclient.message_type = mt;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = d0;
    ev.xclient.data.l[1] = d1;
    ev.xclient.data.l[2] = d2;
    ev.xclient.data.l[3] = d3;
    ev.xclient.data.l[4] = d4;
    XSendEvent(dpy, window, False, mask, &ev);
  }
  return exists;
}

void set_focus(Client *client) {
  if (!client->neverfocus) {
    XSetInputFocus(dpy, client->win, RevertToPointerRoot, CurrentTime);
    XChangeProperty(dpy, root, netatom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&(client->win), 1);
  }
  send_event(client->win, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

void set_full_screen(Client *client, int fullscreen) {
  if (fullscreen && !client->isfullscreen) {
    XChangeProperty(dpy, client->win, netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)&netatom[NetWMFullscreen], 1);
    client->isfullscreen = 1;
    client->oldstate = client->isfloating;
    client->oldbw = client->bw;
    client->bw = 0;
    client->isfloating = 1;
    resize_client(client, client->mon->mx, client->mon->my, client->mon->mw, client->mon->mh);
    XRaiseWindow(dpy, client->win);
  } else if (!fullscreen && client->isfullscreen) {
    XChangeProperty(dpy, client->win, netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)0, 0);
    client->isfullscreen = 0;
    client->isfloating = client->oldstate;
    client->bw = client->oldbw;
    client->x = client->oldx;
    client->y = client->oldy;
    client->w = client->oldw;
    client->h = client->oldh;
    resize_client(client, client->x, client->y, client->w, client->h);
    arrange(client->mon);
  }
}

void set_gaps(const Arg *arg) {
  if ((arg->i == 0) || (selmon->gappx + arg->i < 0)) {
    selmon->gappx = 0;
  } else {
    selmon->gappx += arg->i;
  }

  arrange(selmon);
}

void setlayout(const Arg *arg) {
  if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt]) {
    selmon->sellt ^= 1;
  }
  if (arg && arg->v) {
    selmon->lt[selmon->sellt] = (Layout *)arg->v;
  }
  strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
  if (selmon->sel) {
    arrange(selmon);
  } else {
    draw_bar(selmon);
  }
}

/* arg > 1.0 will set mfact absolutely */
void set_mfact(const Arg *arg) {
  float f;

  if (!arg || !selmon->lt[selmon->sellt]->arrange) {
    return;
  }

  f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
  if (f < 0.05 || f > 0.95) {
    return;
  }

  selmon->mfact = f;
  arrange(selmon);
}

void setup(void) {
  int i;
  XSetWindowAttributes window_attributes;
  Atom utf8string;
  struct sigaction action;

  /* do not transform children into zombies when they terminate */
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
  action.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &action, NULL);

  /* clean up any zombies (inherited from .xinitrc etc) immediately */
  while (waitpid(-1, NULL, WNOHANG) > 0);

  signal(SIGHUP, sighup);
  signal(SIGTERM, sigterm);

  /* init screen */
  screen = DefaultScreen(dpy);
  screen_width = DisplayWidth(dpy, screen);
  screen_height = DisplayHeight(dpy, screen);
  root = RootWindow(dpy, screen);
  drw = drw_create(dpy, screen, root, screen_width, screen_height);

  if (!drw_fontset_create(drw, fonts, LENGTH(fonts))) {
    die("no fonts could be loaded.");
  }

  lrpad = drw->fonts->h;
  bh = drw->fonts->h + 2;
  update_geom();
  /* init atoms */
  utf8string = XInternAtom(dpy, "UTF8_STRING", False);
  wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
  wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
  wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
  netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
  netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
  netatom[NetSystemTray] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_S0", False);
  netatom[NetSystemTrayOP] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", False);
  netatom[NetSystemTrayOrientation] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION", False);
  netatom[NetSystemTrayOrientationHorz] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
  netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
  netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
  netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
  netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
  netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
  netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
  netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
  xatom[Manager] = XInternAtom(dpy, "MANAGER", False);
  xatom[Xembed] = XInternAtom(dpy, "_XEMBED", False);
  xatom[XembedInfo] = XInternAtom(dpy, "_XEMBED_INFO", False);
  /* init cursors */
  cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
  cursor[CurResize] = drw_cur_create(drw, XC_sizing);
  cursor[CurMove] = drw_cur_create(drw, XC_fleur);
  /* init appearance */
  scheme = ecalloc(LENGTH(colors), sizeof(Clr *));
  for (i = 0; i < LENGTH(colors); i++){
    scheme[i] = drw_scm_create(drw, colors[i], 3);
  }
  /* init system tray */
  update_systray();
  /* init bars */
  update_bars();
  update_status();
  /* supporting window for NetWMCheck */
  wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
  XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wmcheckwin, 1);
  XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8, PropModeReplace, (unsigned char *)"dwm", 3);
  XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wmcheckwin, 1);
  /* EWMH support per view */
  XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *)netatom, NetLast);
  XDeleteProperty(dpy, root, netatom[NetClientList]);
  /* select events */
  window_attributes.cursor = cursor[CurNormal]->cursor;
  window_attributes.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
  XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &window_attributes);
  XSelectInput(dpy, root, window_attributes.event_mask);
  grab_keys();
  focus(NULL);
}

void set_urgent(Client *client, int urgency) {
  XWMHints *wmh;

  client->isurgent = urgency;
  if (!(wmh = XGetWMHints(dpy, client->win))) {
    return;
  }

  wmh->flags = urgency ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
  XSetWMHints(dpy, client->win, wmh);
  XFree(wmh);
}

void show_hide(Client *client) {
  if (!client){
    return;
  }

  if (ISVISIBLE(client)) {
    /* show clients top down */
    XMoveWindow(dpy, client->win, client->x, client->y);
    if ((!client->mon->lt[client->mon->sellt]->arrange || client->isfloating) && !client->isfullscreen) {
      resize(client, client->x, client->y, client->w, client->h, 0);
	  }
    show_hide(client->snext);
  } else {
    /* hide clients bottom up */
    show_hide(client->snext);
    XMoveWindow(dpy, client->win, WIDTH(client) * -2, client->y);
  }
}

void sighup(int unused) {
  Arg a = {.i = 1};
  quit(&a);
}

void sigterm(int unused) {
  Arg a = {.i = 0};
  quit(&a);
}

void spawn(const Arg *arg) {
  if (arg->v == dmenucmd) {
    dmenumon[0] = '0' + selmon->num;
  }
  if (fork() == 0) {
    if (dpy) {
      close(ConnectionNumber(dpy));
	  }
    setsid();
    execvp(((char **)arg->v)[0], (char **)arg->v);
    die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
  }
}

void tag(const Arg *arg) {
  if (selmon->sel && arg->ui & TAGMASK) {
    selmon->sel->tags = arg->ui & TAGMASK;
    focus(NULL);
    arrange(selmon);
  }
}

void tag_monitor(const Arg *arg) {
  if (!selmon->sel || !mons->next){
    return;
  }

  send_montor(selmon->sel, dir_to_mon(arg->i));
}

void tile(Monitor *monitor) {
  unsigned int i, n, h, mw, my, ty;
  Client *c;

  for (n = 0, c = next_tiled(monitor->clients); c; c = next_tiled(c->next), n++);

  if (n == 0){
    return;
  }

  if (n > monitor->nmaster){
    mw = monitor->nmaster ? monitor->ww * monitor->mfact : 0;
  } else {
    mw = monitor->ww - monitor->gappx;
  }

  for (i = 0, my = ty = monitor->gappx, c = next_tiled(monitor->clients); c; c = next_tiled(c->next), i++) {
    if (i < monitor->nmaster) {
      h = (monitor->wh - my) / (MIN(n, monitor->nmaster) - i) - monitor->gappx;
      resize(c, monitor->wx + monitor->gappx, monitor->wy + my, mw - (2 * c->bw) - monitor->gappx, h - (2 * c->bw), 0);
      if (my + HEIGHT(c) + monitor->gappx < monitor->wh){
        my += HEIGHT(c) + monitor->gappx;
	    }
    } else {
      h = (monitor->wh - ty) / (n - i) - monitor->gappx;
      resize(c, monitor->wx + mw + monitor->gappx, monitor->wy + ty, monitor->ww - mw - (2 * c->bw) - 2 * monitor->gappx, h - (2 * c->bw), 0);
      if (ty + HEIGHT(c) + monitor->gappx < monitor->wh){
        ty += HEIGHT(c) + monitor->gappx;
	    }
    }
  }
}

void toggle_bar(const Arg *arg) {
  selmon->showbar = !selmon->showbar;
  update_bar_pos(selmon);
  resize_bar_win(selmon);

  if (showsystray) {
    XWindowChanges wc;
    if (!selmon->showbar) {
      wc.y = -bh;
	  }
    else if (selmon->showbar) {
      wc.y = 0;
      if (!selmon->topbar){
        wc.y = selmon->mh - bh;
	    }
    }
    XConfigureWindow(dpy, systray->win, CWY, &wc);
  }
  arrange(selmon);
}

void toggle_floating(const Arg *arg) {
  if (!selmon->sel) {
    return;
  }

  /* no support for fullscreen windows */
  if (selmon->sel->isfullscreen) {
    return;
  }

  selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
  if (selmon->sel->isfloating) {
    resize(selmon->sel, selmon->sel->x, selmon->sel->y, selmon->sel->w, selmon->sel->h, 0);
  }

  arrange(selmon);
}

void toggle_fullscreen(const Arg *arg) {
  if (selmon->sel) {
    set_full_screen(selmon->sel, !selmon->sel->isfullscreen);
  }
}

void toggle_tag(const Arg *arg) {
  unsigned int newtags;

  if (!selmon->sel) {
    return;
  }

  newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
  if (newtags) {
    selmon->sel->tags = newtags;
    focus(NULL);
    arrange(selmon);
  }
}

void toggle_view(const Arg *arg) {
  unsigned int newtagset =
      selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);

  if (newtagset) {
    selmon->tagset[selmon->seltags] = newtagset;
    focus(NULL);
    arrange(selmon);
  }
}

void unfocus(Client *client, int setfocus) {
  if (!client) {
    return;
  }

  grab_buttons(client, 0);
  XSetWindowBorder(dpy, client->win, scheme[SchemeNorm][ColBorder].pixel);
  if (setfocus) {
    XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
  }
}

void unmanage(Client *client, int destroyed) {
  Monitor *m = client->mon;
  XWindowChanges wc;

  detach(client);
  detach_stack(client);
  if (!destroyed) {
    wc.border_width = client->oldbw;
    XGrabServer(dpy); /* avoid race conditions */
    XSetErrorHandler(xerrordummy);
    XSelectInput(dpy, client->win, NoEventMask);
    XConfigureWindow(dpy, client->win, CWBorderWidth, &wc); /* restore border */
    XUngrabButton(dpy, AnyButton, AnyModifier, client->win);
    set_client_state(client, WithdrawnState);
    XSync(dpy, False);
    XSetErrorHandler(xerror);
    XUngrabServer(dpy);
  }
  free(client);
  focus(NULL);
  update_client_list();
  arrange(m);
}

void unmap_notify(XEvent *event) {
  Client *c;
  XUnmapEvent *ev = &event->xunmap;

  if ((c = window_to_client(ev->window))) {
    if (ev->send_event){
      set_client_state(c, WithdrawnState);
    } else {
        unmanage(c, 0);
    }
  } else if ((c = window_to_systray_icon(ev->window))) {
    /* KLUDGE! sometimes icons occasionally unmap their windows, but do
     * _not_ destroy them. We map those windows back */
    XMapRaised(dpy, c->win);
    update_systray();
  }
}

void update_bars(void) {
  unsigned int w;
  Monitor *m;
  XSetWindowAttributes wa = {.override_redirect = True, .background_pixmap = ParentRelative, .event_mask = ButtonPressMask | ExposureMask};
  XClassHint ch = {"dwm", "dwm"};

  for (m = mons; m; m = m->next) {
    if (m->barwin){
      continue;
	  }

    w = m->ww;
    if (showsystray && m == systray_to_monitor(m)){
      w -= get_systray_width();
	  }

    m->barwin = XCreateWindow(dpy, root, m->wx, m->by, w, bh, 0, DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen), CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
    XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
    if (showsystray && m == systray_to_monitor(m)){
      XMapRaised(dpy, systray->win);
	  }

    XMapRaised(dpy, m->barwin);
    XSetClassHint(dpy, m->barwin, &ch);
  }
}

void update_bar_pos(Monitor *monitor) {
  monitor->wy = monitor->my;
  monitor->wh = monitor->mh;

  if (monitor->showbar) {
    monitor->wh -= bh;
    monitor->by = monitor->topbar ? monitor->wy : monitor->wy + monitor->wh;
    monitor->wy = monitor->topbar ? monitor->wy + bh : monitor->wy;
  } else {
    monitor->by = -bh;
  }
}

void update_client_list() {
  Client *c;
  Monitor *m;

  XDeleteProperty(dpy, root, netatom[NetClientList]);
  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->win), 1);
	  }
  }
}

int update_geom(void) {
  int dirty = 0;

#ifdef XINERAMA
  if (XineramaIsActive(dpy)) {
    int i, j, n, nn;
    Client *c;
    Monitor *m;
    XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
    XineramaScreenInfo *unique = NULL;

    for (n = 0, m = mons; m; m = m->next, n++)
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
      for (m = mons; m && m->next; m = m->next)
        ;
      if (m) {
        m->next = create_monitor();
	  } else {
        mons = create_monitor();
	  }
    }
    for (i = 0, m = mons; i < nn && m; m = m->next, i++){
      if (i >= n || unique[i].x_org != m->mx || unique[i].y_org != m->my || unique[i].width != m->mw || unique[i].height != m->mh) {
        dirty = 1;
        m->num = i;
        m->mx = m->wx = unique[i].x_org;
        m->my = m->wy = unique[i].y_org;
        m->mw = m->ww = unique[i].width;
        m->mh = m->wh = unique[i].height;
        update_bar_pos(m);
      }
	}

    /* removed monitors if n > nn */
    for (i = nn; i < n; i++) {
      for (m = mons; m && m->next; m = m->next)
        ;
      while ((c = m->clients)) {
        dirty = 1;
        m->clients = c->next;
        detach_stack(c);
        c->mon = mons;
        attach(c);
        attach_stack(c);
      }
      if (m == selmon) {
        selmon = mons;
	  }
      cleanup_mon(m);
    }
    free(unique);
  } else
#endif /* XINERAMA */
  {    /* default monitor setup */
    if (!mons) {
      mons = create_monitor();
	}
    if (mons->mw != screen_width || mons->mh != screen_height) {
      dirty = 1;
      mons->mw = mons->ww = screen_width;
      mons->mh = mons->wh = screen_height;
      update_bar_pos(mons);
    }
  }
  if (dirty) {
    selmon = mons;
    selmon = window_to_monitor(root);
  }

  return dirty;
}

void update_numlock_mask(void) {
  unsigned int i, j;
  XModifierKeymap *modmap;

  numlockmask = 0;
  modmap = XGetModifierMapping(dpy);

  for (i = 0; i < 8; i++) {
    for (j = 0; j < modmap->max_keypermod; j++) {
      if (modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock)) {
        numlockmask = (1 << i);
	    }
	  }
  }

  XFreeModifiermap(modmap);
}

void update_size_hints(Client *client) {
  long msize;
  XSizeHints size;

  if (!XGetWMNormalHints(dpy, client->win, &size, &msize)) {
    /* size is uninitialized, ensure that size.flags aren't used */
    size.flags = PSize;
  }
  if (size.flags & PBaseSize) {
    client->basew = size.base_width;
    client->baseh = size.base_height;
  } else if (size.flags & PMinSize) {
    client->basew = size.min_width;
    client->baseh = size.min_height;
  } else {
    client->basew = client->baseh = 0;
  }
  if (size.flags & PResizeInc) {
    client->incw = size.width_inc;
    client->inch = size.height_inc;
  } else {
    client->incw = client->inch = 0;
  }
  if (size.flags & PMaxSize) {
    client->maxw = size.max_width;
    client->maxh = size.max_height;
  } else {
    client->maxw = client->maxh = 0;
  }
  if (size.flags & PMinSize) {
    client->minw = size.min_width;
    client->minh = size.min_height;
  } else if (size.flags & PBaseSize) {
    client->minw = size.base_width;
    client->minh = size.base_height;
  } else {
    client->minw = client->minh = 0;
  }
  if (size.flags & PAspect) {
    client->mina = (float)size.min_aspect.y / size.min_aspect.x;
    client->maxa = (float)size.max_aspect.x / size.max_aspect.y;
  } else {
    client->maxa = client->mina = 0.0;
  }

  client->isfixed = (client->maxw && client->maxh && client->maxw == client->minw && client->maxh == client->minh);
  client->hintsvalid = 1;
}

void update_status(void) {
  if (!get_text_prop(root, XA_WM_NAME, stext, sizeof(stext))) {
    strcpy(stext, "dwm-" VERSION);
  }

  if (showstatuson) {
    Monitor *m;

    for (m = mons; m; m = m->next) {
      draw_bar(m);
    }
  } else {
    draw_bar(selmon);
  }
  update_systray();
}

void update_systray_icon_geom(Client *client, int width, int height) {
  if (client) {
    client->h = bh;
    if (width == height) {
      client->w = bh;
	  } else if (height == bh) {
      client->w = width;
	  } else {
      client->w = (int)((float)bh * ((float)width / (float)height));
	  }

    apply_size_hints(client, &(client->x), &(client->y), &(client->w), &(client->h), False);
    /* force icons into the systray dimensions if they don't want to */
    if (client->h > bh) {
      if (client->w == client->h) {
        client->w = bh;
	    } else {
        client->w = (int)((float)bh * ((float)client->w / (float)client->h));
	    }

      client->h = bh;
    }
  }
}

void update_systray_icon_state(Client *client, XPropertyEvent *event) {
  long flags;
  int code = 0;

  if (!showsystray || !client || event->atom != xatom[XembedInfo] || !(flags = get_atom_prop(client, xatom[XembedInfo]))) {
    return;
  }

  if (flags & XEMBED_MAPPED && !client->tags) {
    client->tags = 1;
    code = XEMBED_WINDOW_ACTIVATE;
    XMapRaised(dpy, client->win);
    set_client_state(client, NormalState);
  } else if (!(flags & XEMBED_MAPPED) && client->tags) {
    client->tags = 0;
    code = XEMBED_WINDOW_DEACTIVATE;
    XUnmapWindow(dpy, client->win);
    set_client_state(client, WithdrawnState);
  } else {
    return;
  }

  send_event(client->win, xatom[Xembed], StructureNotifyMask, CurrentTime, code, 0, systray->win, XEMBED_EMBEDDED_VERSION);
}

void update_systray(void) {
  XSetWindowAttributes wa;
  XWindowChanges wc;
  Client *i;
  Monitor *m = systray_to_monitor(NULL);
  unsigned int x = m->mx + m->mw;
  unsigned int sw = TEXTW(stext) - lrpad + systrayspacing;
  unsigned int w = 1;

  if (!showsystray) {
    return;
  }

  if (systrayonleft) {
    x -= sw + lrpad / 2;
  }

  // init systray
  if (!systray) {
    if (!(systray = (Systray *)calloc(1, sizeof(Systray)))) {
      die("fatal: could not malloc() %u bytes\n", sizeof(Systray));
	  }

    // create tray window
    systray->win = XCreateSimpleWindow(dpy, root, x, m->by, w, bh, 0, 0, scheme[SchemeSel][ColBg].pixel);
    wa.event_mask = ButtonPressMask | ExposureMask;
    wa.override_redirect = True;
    wa.background_pixel = scheme[SchemeNorm][ColBg].pixel;

    // add class to systray
    XClassHint ch = { .res_class="dwm_tray", .res_name="dwm_tray"};
    XSetClassHint(dpy, systray->win, &ch);

    XSelectInput(dpy, systray->win, SubstructureNotifyMask);
    XChangeProperty(dpy, systray->win, netatom[NetSystemTrayOrientation], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&netatom[NetSystemTrayOrientationHorz], 1);
    XChangeWindowAttributes(dpy, systray->win, CWEventMask | CWOverrideRedirect | CWBackPixel, &wa);
    XMapRaised(dpy, systray->win);
    XSetSelectionOwner(dpy, netatom[NetSystemTray], systray->win, CurrentTime);

    if (XGetSelectionOwner(dpy, netatom[NetSystemTray]) == systray->win) {
      send_event(root, xatom[Manager], StructureNotifyMask, CurrentTime, netatom[NetSystemTray], systray->win, 0, 0);
      XSync(dpy, False);
    } else {
      fprintf(stderr, "dwm: unable to obtain system tray.\n");
      free(systray);
      systray = NULL;
      return;
    }
  }

  for (w = 0, i = systray->icons; i; i = i->next) {
    /* make sure the background color stays the same */
    wa.background_pixel = scheme[SchemeNorm][ColBg].pixel;

    XChangeWindowAttributes(dpy, i->win, CWBackPixel, &wa);
    XMapRaised(dpy, i->win);
    w += systrayspacing;
    i->x = w;
    XMoveResizeWindow(dpy, i->win, i->x, 0, i->w, i->h);
    w += i->w;
    if (i->mon != m) {
      i->mon = m;
	  }
  }

  w = w ? w + systrayspacing : 1;
  x -= w;
  XMoveResizeWindow(dpy, systray->win, x, m->by, w, bh);
  wc.x = x;
  wc.y = m->by;
  wc.width = w;
  wc.height = bh;
  wc.stack_mode = Above;
  wc.sibling = m->barwin;
  XConfigureWindow(dpy, systray->win, CWX | CWY | CWWidth | CWHeight | CWSibling | CWStackMode, &wc);
  XMapWindow(dpy, systray->win);
  XMapSubwindows(dpy, systray->win);

  /* redraw background */
  XSetForeground(dpy, drw->gc, scheme[SchemeNorm][ColBg].pixel);
  XFillRectangle(dpy, systray->win, drw->gc, 0, 0, w, bh);
  XSync(dpy, False);
}

void update_title(Client *client) {
  if (!get_text_prop(client->win, netatom[NetWMName], client->name, sizeof client->name)) {
    get_text_prop(client->win, XA_WM_NAME, client->name, sizeof client->name);
  }

  /* hack to mark broken clients */
  if (client->name[0] == '\0') {
    strcpy(client->name, broken);
  }
}

void update_window_type(Client *client) {
  Atom state = get_atom_prop(client, netatom[NetWMState]);
  Atom wtype = get_atom_prop(client, netatom[NetWMWindowType]);

  if (state == netatom[NetWMFullscreen]) {
    set_full_screen(client, 1);
  }
  if (wtype == netatom[NetWMWindowTypeDialog]) {
    client->isfloating = 1;
  }
}

void update_wm_hints(Client *client) {
  XWMHints *wmh;

  if ((wmh = XGetWMHints(dpy, client->win))) {
    if (client == selmon->sel && wmh->flags & XUrgencyHint){
      wmh->flags &= ~XUrgencyHint;
      XSetWMHints(dpy, client->win, wmh);
    } else {
      client->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
	}

    if (wmh->flags & InputHint) {
      client->neverfocus = !wmh->input;
	}
    else {
      client->neverfocus = 0;
	}

    XFree(wmh);
  }
}

void view(const Arg *arg) {
  if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags]) {
    return;
  }

  selmon->seltags ^= 1; /* toggle sel tagset */
  if (arg->ui & TAGMASK) {
    selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
  }

  focus(NULL);
  arrange(selmon);
}

Client *window_to_client(Window window) {
  Client *c;
  Monitor *m;

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      if (c->win == window){
        return c;
	    }
	  }
  }

  return NULL;
}

Client *window_to_systray_icon(Window window) {
  Client *i = NULL;

  if (!showsystray || !window) {
    return i;
  }

  for (i = systray->icons; i && i->win != window; i = i->next);

  return i;
}

Monitor *window_to_monitor(Window window) {
  int x, y;
  Client *c;
  Monitor *m;

  if (window == root && get_root_ptr(&x, &y)) {
    return rect_to_mon(x, y, 1, 1);
  }

  for (m = mons; m; m = m->next) {
    if (window == m->barwin){
      return m;
	  }
  }

  if ((c = window_to_client(window))) {
    return c->mon;
  }

  return selmon;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int xerror(Display *display, XErrorEvent *event) {
  if (event->error_code == BadWindow ||
      (event->request_code == X_SetInputFocus && event->error_code == BadMatch) ||
      (event->request_code == X_PolyText8 && event->error_code == BadDrawable) ||
      (event->request_code == X_PolyFillRectangle &&
       event->error_code == BadDrawable) ||
      (event->request_code == X_PolySegment && event->error_code == BadDrawable) ||
      (event->request_code == X_ConfigureWindow && event->error_code == BadMatch) ||
      (event->request_code == X_GrabButton && event->error_code == BadAccess) ||
      (event->request_code == X_GrabKey && event->error_code == BadAccess) ||
      (event->request_code == X_CopyArea && event->error_code == BadDrawable)){
  		  return 0;
	  }

  fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n", event->request_code, event->error_code);
  return xerrorxlib(display, event); /* may call exit */
}

int xerrordummy(Display *display, XErrorEvent *event) {
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int xerrorstart(Display *display, XErrorEvent *event) {
  die("dwm: another window manager is already running");
  return -1;
}

Monitor *systray_to_monitor(Monitor *monitor) {
  Monitor *t;
  int i, n;

  if (!systraypinning) {
    if (!monitor) {
      return selmon;
	  }

    return monitor == selmon ? monitor : NULL;
  }

  for (n = 1, t = mons; t && t->next; n++, t = t->next);
  for (i = 1, t = mons; t && t->next && i < systraypinning; i++, t = t->next);

  if (systraypinningfailfirst && n < systraypinning) {
    return mons;
  }

  return t;
}

void zoom(const Arg *arg) {
  Client *client = selmon->sel;

  if (!selmon->lt[selmon->sellt]->arrange || !client || client->isfloating) {
    return;
  }

  if (client == next_tiled(selmon->clients) && !(client = next_tiled(client->next))) {
    return;
  }

  pop(client);
}

int main(int argc, char *argv[]) {
  if (argc == 2 && !strcmp("-v", argv[1])) {
    die("dwm-" VERSION " (pdwm 0.1)");
  }

  if (argc != 1) {
    die("usage: dwm [-v]");
  }

  if (!setlocale(LC_CTYPE, "") || !XSupportsLocale()) {
    fputs("warning: no locale support\n", stderr);
  }

  if (!(dpy = XOpenDisplay(NULL))) {
    die("dwm: cannot open display");
  }

  check_other_wm(); // check if another wm is running
  setup();        // init systray, bars, screens, etc.
  scan();
  run(); 			    // event loop

  if (restart) {
    execvp(argv[0], argv);
  }

  cleanup();
  XCloseDisplay(dpy);

  return EXIT_SUCCESS;
}
