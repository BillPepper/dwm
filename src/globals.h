#include "definitions.h"
#include "dwm.h"

/* variables */
static Systray *systray = NULL;
static const char broken[] = "broken";
static char status_text[256];
static int screen;
static int screen_width, screen_height; /* X display screen geometry width, height */
static int bar_height;     /* bar height */
static int lr_padding;  /* sum of left and right padding for text */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;

static Atom wmatom[WMLast], netatom[NetLast], xatom[XLast];
static int restart = 0;
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *display;
static Drw *drw;
static Monitor *monitors, *selected_monitor;
static Window root, wmcheckwin;

static void (*handler[LASTEvent])(XEvent *) = {
  [ButtonPress] = buttonpress,
  [ClientMessage] = clientmessage,
  [ConfigureRequest] = configurerequest,
  [ConfigureNotify] = configurenotify,
  [DestroyNotify] = destroynotify,
  [EnterNotify] = enternotify,
  [Expose] = expose,
  [FocusIn] = focusin,
  [KeyPress] = keypress,
  [MappingNotify] = mappingnotify,
  [MapRequest] = maprequest,
  [MotionNotify] = motionnotify,
  [PropertyNotify] = propertynotify,
  [ResizeRequest] = resizerequest,
  [UnmapNotify] = unmapnotify
};
