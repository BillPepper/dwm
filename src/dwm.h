#ifndef DWM_H
#define DWM_H

#include "definitions.h"

#include "monitor.h"
#include "layout.h"
#include "events.h"
#include "stacking.h"
#include "input.h"
#include "debug.h"
#include "process.h"
#include "tagging.h"
#include "bar.h"
#include "client.h"
#include "window.h"

// Error
static int xerror(Display *display, XErrorEvent *event);                        // handle errors
static int xerrordummy(Display *display, XErrorEvent *event);                   // returns 0
static int xerrorstart(Display *display, XErrorEvent *event);                   // called when other wm is running

// Other
static void view(const Arg *arg);                                               // view clients with given tag
static void zoom(const Arg *arg);                                               // zooms the master to the next client
static Atom getatomprop(Client *client, Atom prop);                             // get prop of given client
static int updategeom(void);                                                    // some multi screen stuff, related to xinerama

// Custom
static void parse_args(int argc, char *argv[]);                                 // parse main() args

#endif