#ifndef STACKING_H
#define STACKING_H

#include "definitions.h"

void restack(Monitor *monitor);                                          // restack clients on monitor
void focusstack(const Arg *arg);                                         // focus stack by index
void incnmaster(const Arg *arg);                                         // increment master area
void setgaps(const Arg *arg);                                            // set the monitor gaps to arg then call arrange()
void setlayout(const Arg *arg);                                          // set the layout and the label on bar to arg, then arrange and/or update
void setmfact(const Arg *arg);                                           // set master factor to arg

#endif