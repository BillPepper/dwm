#include "definitions.h"

#ifndef MONITOR_H
#define MONITOR_H

static Monitor *createmon(void);                        // create Monitor struct
static Monitor *dirtomon(int dir);                      // get monitor in direction?
static Monitor *recttomon(Area *area);                  // return the monitor given rect should be rendered on
static void focusmon(const Arg *arg);                   // focus monitor by index
static void arrange(Monitor *monitor);                  // arrange specified, or all monitors if m = NULL
static void arrangemon(Monitor *monitor);               // set layout string and call arrange callback
static void cleanupmon(Monitor *monitor);               // remove monitor

#endif