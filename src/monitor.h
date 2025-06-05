
#ifndef MONITOR_H
#define MONITOR_H

#include <stdlib.h>
#include "stacking.h"

Monitor *create_monitor(void);                        // create Monitor struct
Monitor *dir_to_monitor(int dir);                      // get monitor in direction?
Monitor *rect_to_monitor(Area *area);                  // return the monitor given rect should be rendered on
void focus_monitor(const Arg *arg);                   // focus monitor by index
void arrange(Monitor *monitor);                  // arrange specified, or all monitors if m = NULL
void arrange_monitor(Monitor *monitor);               // set layout string and call arrange callback
void cleanup_monitor(Monitor *monitor);               // remove monitor

#endif