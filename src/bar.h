#ifndef BAR_H
#define BAR_H

#include <X11/Xlib.h>

#include "definitions.h"

static void togglebar(const Arg *arg);                                          // toggle bar (arg unused)
static void update_bar_position(Monitor *monitor);                              // recalculate bar position
static void drawbar(Monitor *monitor);                                          // draw bar, tags, layout and title
static void drawbars(void);                                                     // wrapper calling drawbar() for all monitors
static void resizebarwin(Monitor *monitor);                                     // resize bar and tray
static void updatestatus(void);                                                 // render current status text or fallback then update tray
static void updatebars(void);                                                   // set bar class hints, then render bar (top most) for each monitor


#endif