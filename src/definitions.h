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

typedef struct {
  float min;
  float max;
} fMinMax;

typedef struct {
  int x;
  int y;
} Position;

typedef struct {
  int w;
  int h;
} Size;

typedef struct {
  Position position;
  Size size;
} Area;

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
  fMinMax aspect;
  Area area;
  Area old_area;
  Size base;
  Size inc;
  Size max;
  Size min;
  int hintsvalid;
  int bw;
  int oldbw;
  unsigned int tags;
  int is_fixed;
  int is_floating;
  int is_urgent;
  int never_focus;
  int old_state;
  int is_fullscreen;
  Client *next;
  Client *next_stack;
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
  void (*arrange_func)(Monitor *);
} Layout;

struct Monitor {
  char layout_symbol[16];
  float master_factor;            // size of master
  int master_count;               // clients in master
  int num;                        // monitor index?
  int bar_y;
  Area monitor_area;
  Area window_area;
  int gap;
  unsigned int selected_tags;     // mask of seleted tags?
  unsigned int selected_layout;   // index of current layout
  unsigned int tag_set[2];
  int bar_enabled;
  int is_topbar;
  Client *clients;
  Client *selected_client;
  Client *stack;
  Monitor *next;
  Window bar_window;
  const Layout *layout[2];
};

typedef struct {
  const char *class_name;
  const char *instance;
  const char *title;
  unsigned int tags;
  int is_floating;
  int monitor;
} Rule;

typedef struct Systray Systray;
struct Systray {
  Window window;
  Client *icons;
};

#endif