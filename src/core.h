#ifndef CORE_H
#define CORE_H

extern int restart;
extern int running;

#include "definitions.h"


// @desc: init dwm, bar, tray, screen, etc...
void setup(void);

// @descr: main loop, check events and handle them
void run(void);

// @descr: scan for client windows
void scan(void);

// @descr: check if another wm is running
void checkotherwm(void);

// @descr: ungrab keys, destroy windows, etc...
void cleanup(void);

// @descr: quit dwm
// @param: arg -> 1 for restart, 0 for end
void quit(const Arg *arg);

// @descr: view clients with given tag
// @param: arg -> tag
void view(const Arg *arg);

// @descr: zooms the master to the next client
// @param: ?
void zoom(const Arg *arg);

// @desc: get prop of given client
Atom getatomprop(Client *client, Atom prop);

// @desc: some multi screen stuff, related to xinerama
int updategeom(void);

// @desc: parse main() args
void parse_args(int argc, char *argv[]);

#endif