#ifndef BAR_H
#define BAR_H

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <string.h>

#include "macro.h"

#include "definitions.h"
#include "drw.h"
#include "tray.h"
#include "window.h"
#include "monitor.h"

// defined in config.h and global.h
extern void arrange(Monitor *monitor);
extern int systray_enabled;
extern Monitor *selected_monitor;
extern int bar_height;
extern Systray *systray;
extern Display *display;
extern Drw *drw;
extern Monitor *monitors;
extern char status_text[256];
extern int status_monitor;
extern int padding;
extern Clr **scheme;
extern Window root;
extern Cur *cursor[];
extern int screen;
extern const char *tags[15];
extern int systray_on_left;

void togglebar(const Arg *arg);                                          // toggle bar (arg unused)
void update_bar_position(Monitor *monitor);                              // recalculate bar position
void drawbar(Monitor *monitor);                                          // draw bar, tags, layout and title
void drawbars(void);                                                     // wrapper calling drawbar() for all monitors
void resizebarwin(Monitor *monitor);                                     // resize bar and tray
void updatestatus(void);                                                 // render current status text or fallback then update tray
void updatebars(void);                                                   // set bar class hints, then render bar (top most) for each monitor


#endif