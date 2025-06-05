#ifndef CORE_H
#define CORE_H

// XINERAMA
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include "definitions.h"
#include "globals.h"
#include "process.h"
#include "tray.h"

// @desc: init dwm, bar, tray, screen, etc...
void setup(void);

// @descr: main loop, check events and handle them
void run(void);

// @descr: scan for client windows
void scan(void);

// @descr: check if another wm is running
void check_other_wm(void);

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
Atom get_atom_prop(Client *client, Atom prop);

// @desc: some multi screen stuff, related to xinerama
int update_geometry(void);

#ifdef XINERAMA
int is_unique_geometry(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info);
#endif /* XINERAMA */

#endif