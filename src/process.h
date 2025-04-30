#ifndef PROCESS_H
#define PROCESS_H

#include <X11/Xlib.h>

#include <stdlib.h>
#include <unistd.h>

#include "definitions.h"
#include "util.h"
#include "core.h"

extern Display *display;
extern char dmenumon[2];
extern const char *dmenucmd[];
extern Monitor *selected_monitor;

// processes and signals
void sighup(int unused);                                                 // quit dwm with arg=1, which will restart dwm (unused arg)
void sigterm(int unused);                                                // quit dwm with arg=0, which will end dwm (unused arg)
void spawn(const Arg *arg);                                              // spawn a new process using fork()

#endif