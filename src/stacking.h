#ifndef STACKING_H
#define STACKING_H

#include "definitions.h"

static void restack(Monitor *monitor);                                          // restack clients on monitor
static void focusstack(const Arg *arg);                                         // focus stack by index
static void incnmaster(const Arg *arg);                                         // increment master area
static void setgaps(const Arg *arg);                                            // set the monitor gaps to arg then call arrange()
static void setlayout(const Arg *arg);                                          // set the layout and the label on bar to arg, then arrange and/or update
static void setmfact(const Arg *arg);                                           // set master factor to arg

#endif