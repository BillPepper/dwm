
#ifndef MONITOR_H
#define MONITOR_H

#include <stdlib.h>
#include "stacking.h"

Monitor *createmon(void);                        // create Monitor struct
Monitor *dirtomon(int dir);                      // get monitor in direction?
Monitor *recttomon(Area *area);                  // return the monitor given rect should be rendered on
void focusmon(const Arg *arg);                   // focus monitor by index
void arrange(Monitor *monitor);                  // arrange specified, or all monitors if m = NULL
void arrangemon(Monitor *monitor);               // set layout string and call arrange callback
void cleanupmon(Monitor *monitor);               // remove monitor

#endif