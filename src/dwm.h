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

// window (X11)
static void unmanage(Client *client, int destroyed);                            // detach, unfocus, update and re-arrange clients
static void manage(Window window, XWindowAttributes *wa);                       // manage given window with dwm
static long getstate(Window window);                                            // get window state
static int gettextprop(Window window, Atom atom, char *text, unsigned int size);// get text prop from window
static Client *wintoclient(Window window);                                      // get dwm client for x11 window
static Monitor *wintomon(Window window);                                        // get monitor the given x11 window is on
static Client *wintosystrayicon(Window window);                                 // get icon (client) for given x11 window

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