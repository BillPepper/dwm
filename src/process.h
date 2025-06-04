#ifndef PROCESS_H
#define PROCESS_H

#include <unistd.h>
#include "core.h"

// processes and signals
void sighup(int unused);                                                 // quit dwm with arg=1, which will restart dwm (unused arg)
void sigterm(int unused);                                                // quit dwm with arg=0, which will end dwm (unused arg)
void spawn(const Arg *arg);                                              // spawn a new process using fork()

#endif