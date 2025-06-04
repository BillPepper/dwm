#ifndef BAR_H
#define BAR_H

#include <sys/wait.h>

#include "definitions.h"
#include "globals.h"
#include "tray.h"


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