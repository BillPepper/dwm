#ifndef DWM_H
#define DWM_H

#include <locale.h>

#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include "definitions.h"
#include "globals.h"
#include "../config.h"

/* variables */
char status_text[256];                                                   // right side bar text
int screen;
int screen_width;
int screen_height;                                                       /* X display screen geometry width, height */
int bar_height;                                                          /* bar height */
int padding;                                                             /* sum of left and right padding for text */
int (*xerrorxlib)(Display *, XErrorEvent *);

Atom wmatom[WMLast];
Atom netatom[NetLast];
Atom xatom[XLast];
Cur *cursor[CurLast];
Clr **scheme;
Display *display;
Drw *drw;
Monitor *monitors;                                                       // monitor list
Monitor *selected_monitor;                                               // currently selected monitor
Window root;                                                             // root window of dwn
Window wmcheckwin;

Systray *systray = NULL;
char broken[] = "broken";                                                // default window title
unsigned int numlockmask = 0;
int restart = 0;                                                         // 1 if dwm should restart
int running = 1;                                                         // 1 if dwm is running

// Array of function pointers
#define EVENT_COUNT LASTEvent
void (*handler[EVENT_COUNT])(XEvent *) = {
  // handler[index] = callback
  [ButtonPress] = button_press,
  [ClientMessage] = client_message,
  [ConfigureRequest] = configure_request,
  [ConfigureNotify] = configure_notify,
  [DestroyNotify] = destroy_notify,
  [EnterNotify] = enter_notify,
  [Expose] = expose,
  [FocusIn] = focus_in,
  [KeyPress] = key_press,
  [MappingNotify] = mapping_notify,
  [MapRequest] = map_request,
  [MotionNotify] = motion_notify,
  [PropertyNotify] = property_notify,
  [ResizeRequest] = resize_request,
  [UnmapNotify] = unmap_notify
};

// ----

#endif