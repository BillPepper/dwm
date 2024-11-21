#ifndef CORE_H
#define CORE_H

#include "definitions.h"

static void setup(void);                                                        // init dwm, bar, tray, screen, etc...
static void run(void);                                                          // main loop, check events and handle them
static void scan(void);                                                         // scan for client windows
static void checkotherwm(void);                                                 // check if another wm is running
static void cleanup(void);                                                      // ungrab keys, destroy windows, etc...
static void quit(const Arg *arg);                                               // quit dwm, arg=1 for restart, 0 for end

#endif