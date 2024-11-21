#ifndef PROCESS_H
#define PROCESS_H

#include "definitions.h"

// processes and signals
static void sighup(int unused);                                                 // quit dwm with arg=1, which will restart dwm (unused arg)
static void sigterm(int unused);                                                // quit dwm with arg=0, which will end dwm (unused arg)
static void spawn(const Arg *arg);                                              // spawn a new process using fork()

#endif