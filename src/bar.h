#ifndef BAR_H
#define BAR_H

#include <sys/wait.h>

#include "definitions.h"
#include "globals.h"
#include "tray.h"


// @descr: Enable/disable bar on current screen
// @param: arg -> unsed
void toggle_bar(const Arg *arg);

// @descr: Recalculate bar position, depending on bar visibility
// @param: monitor -> Target monitor on which the bar should be updated
void update_bar_position(Monitor *monitor);

// @descr: Draw bar, tags, layout and title
// @param: monitor -> Target monitor on which the bar should be drawn
void draw_bar(Monitor *monitor);

// @descr: Wrapper calling draw_bar() for all monitors
// @param: none
void draw_bars(void);

// @descr: Resize the bar window, depending on wheter the tray is shown or not
// @param: monitor -> Target monitor the bar is on
void resize_bar_win(Monitor *monitor);

// @descr: Render current status text or fallback then update tray
// @param: none
void update_status(void);

// @descr: Set bar class hints, then render bar (top most) for each monitor
// @param: none
void update_bars(void);


#endif