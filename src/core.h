#ifndef CORE_H
#define CORE_H

#include "definitions.h"

void setup(void);                                                        // init dwm, bar, tray, screen, etc...
void run(void);                                                          // main loop, check events and handle them
void scan(void);                                                         // scan for client windows
void checkotherwm(void);                                                 // check if another wm is running
void cleanup(void);                                                      // ungrab keys, destroy windows, etc...
void quit(const Arg *arg);                                               // quit dwm, arg=1 for restart, 0 for end

// Other
void view(const Arg *arg);                                               // view clients with given tag
void zoom(const Arg *arg);                                               // zooms the master to the next client
Atom getatomprop(Client *client, Atom prop);                             // get prop of given client
int updategeom(void);                                                    // some multi screen stuff, related to xinerama
void parse_args(int argc, char *argv[]);                                 // parse main() args

#endif