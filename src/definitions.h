#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <X11/X.h>

// VERSION is defined in makefile.mk, but my editor does not know that.
#ifndef VERSION
#define VERSION
#endif

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

/* -- enums -- */

/* cursor */
enum {
  CurNormal,
  CurResize,
  CurMove,
  CurLast
};

/* color schemes */
enum {
  SchemeNorm,
  SchemeSel
};

/* EWMH atoms */
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
};

/* Xembed atoms */
enum {
  Manager,
  Xembed,
  XembedInfo,
  XLast
};

/* default atoms */
enum {
  WMProtocols,
  WMDelete,
  WMState,
  WMTakeFocus,
  WMLast
};

/* clicks */
enum {
  ClkTagBar,
  ClkLtSymbol,
  ClkStatusText,
  ClkWinTitle,
  ClkClientWin,
  ClkRootWin,
  ClkLast
};

// -- Types --

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

// a client is a 'window'
struct Client {
  char name[256];
  float mina, maxa;
  int x, y;
  int w, h;
  int oldx, oldy;
  int oldw, oldh;
  int basew, baseh;
  int incw, inch;
  int maxw, maxh;
  int minw, minh;
  int hintsvalid;
  int bw, oldbw;
  unsigned int tags;
  int isfixed;
  int isfloating;
  int isurgent;
  int neverfocus;
  int oldstate;
  int isfullscreen;
  Client *next;
  Client *snext;
  Monitor *monitor;
  Window window;
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
  const char *class_name;
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