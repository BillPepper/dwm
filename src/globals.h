#ifndef GLOBALS_H
#define GLOBALS_H

#include "drw.h"

#include "definitions.h"
#include "events.h"

/* variables */
Systray *systray = NULL;
const char broken[] = "broken";                                          // default window title
char status_text[256];                                                   // right side bar text
int screen;
int screen_width;
int screen_height;                                                       /* X display screen geometry width, height */
int bar_height;                                                          /* bar height */
int padding;                                                             /* sum of left and right padding for text */
int (*xerrorxlib)(Display *, XErrorEvent *);
unsigned int numlockmask = 0;

Atom wmatom[WMLast];
Atom netatom[NetLast];
Atom xatom[XLast];
int restart = 0;                                                         // 1 if dwm should restart
int running = 1;                                                         // 1 if dwm is running
Cur *cursor[CurLast];
Clr **scheme;
Display *display;
Drw *drw;
Monitor *monitors;                                                       // monitor list
Monitor *selected_monitor;                                               // currently selected monitor
Window root;                                                             // root window of dwn
Window wmcheckwin;

// Array of function pointers
#define EVENT_COUNT LASTEvent
void (*handler[EVENT_COUNT])(XEvent *) = {
  // handler[index] = callback
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


#endif