#ifndef DWM_H
#define DWM_H

#include "definitions.h"


/* function declarations */
#include "monitor.h"
#include "layout.h"
#include "events.h"
#include "stacking.h"
#include "input.h"

#include "debug.h"

// processes and signals
static void sighup(int unused);                                                 // quit dwm with arg=1, which will restart dwm (unused arg)
static void sigterm(int unused);                                                // quit dwm with arg=0, which will end dwm (unused arg)
static void spawn(const Arg *arg);                                              // spawn a new process using fork()

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