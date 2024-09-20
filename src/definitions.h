#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <X11/X.h>

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

#endif