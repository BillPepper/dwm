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

// @descr: Enable/disable bar on current screen
// @param: arg -> unsed
void togglebar(const Arg *arg);

// @descr: Recalculate bar position, depending on bar visibility
// @param: monitor -> Target monitor on which the bar should be updated
void update_bar_position(Monitor *monitor);

// @descr: Draw bar, tags, layout and title
// @param: monitor -> Target monitor on which the bar should be drawn
void drawbar(Monitor *monitor);

// @descr: Wrapper calling drawbar() for all monitors
// @param: none
void drawbars(void);

// @descr: Resize the bar window, depending on wheter the tray is shown or not
// @param: monitor -> Target monitor the bar is on
void resizebarwin(Monitor *monitor);

// @descr: Render current status text or fallback then update tray
// @param: none
void updatestatus(void);

// @descr: Set bar class hints, then render bar (top most) for each monitor
// @param: none
void updatebars(void);


#endif