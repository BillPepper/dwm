#ifndef STACKING_H
#define STACKING_H

#include "bar.h"

void restack(Monitor *monitor);                                                 // restack clients on monitor
void focus_stack(const Arg *arg);                                               // focus stack by index
void increment_master(const Arg *arg);                                          // increment master area
void set_gaps(const Arg *arg);                                                  // set the monitor gaps to arg then call arrange()
void set_layout(const Arg *arg);                                                // set the layout and the label on bar to arg, then arrange and/or update
void set_master_factor(const Arg *arg);                                         // set master factor to arg

#endif