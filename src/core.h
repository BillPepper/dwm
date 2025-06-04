#ifndef CORE_H
#define CORE_H

#include <signal.h>

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

// XINERAMA
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include "definitions.h"

#include "process.h"
#include "tray.h"

#include "globals.h"

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

#ifdef XINERAMA
int isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info);
#endif /* XINERAMA */

#endif