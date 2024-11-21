#ifndef CORE_H
#define CORE_H

#include "definitions.h"

static void setup(void);                                                        // init dwm, bar, tray, screen, etc...
static void run(void);                                                          // main loop, check events and handle them
static void scan(void);                                                         // scan for client windows
static void checkotherwm(void);                                                 // check if another wm is running
static void cleanup(void);                                                      // ungrab keys, destroy windows, etc...
static void quit(const Arg *arg);                                               // quit dwm, arg=1 for restart, 0 for end

// Other
static void view(const Arg *arg);                                               // view clients with given tag
static void zoom(const Arg *arg);                                               // zooms the master to the next client
static Atom getatomprop(Client *client, Atom prop);                             // get prop of given client
static int updategeom(void);                                                    // some multi screen stuff, related to xinerama
static void parse_args(int argc, char *argv[]);                                 // parse main() args

#endif