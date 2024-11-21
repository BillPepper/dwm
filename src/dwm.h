#ifndef DWM_H
#define DWM_H

#include "definitions.h"
#include <X11/Xlib.h>

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

// tray (uses clients as icons, TODO: implement icon struct)
static void updatesystray(void);                                                // initializes (for some reason) and updates the tray
static unsigned int getsystraywidth();                                          // retreive the length of the tray area
static Monitor *systraytomon(Monitor *monitor);                                 // move tray to monitor, TODO: refactor or rewrite
static void removesystrayicon(Client *client);                                  // remove tray icon
static void updatesystrayicongeom(Client *client, Size *size);                  // ?
static void updatesystrayiconstate(Client *client, XPropertyEvent *event);      // ?

// client
static void killclient(const Arg *arg);                                         // close client window
static void togglefloating(const Arg *arg);                                     // toggle floating for current client (arg unused)
static void togglefullscreen(const Arg *arg);                                   // toggle fullscreen for current client (arg unused)
static void updateclientlist(void);                                             // update all clients on all monitors
static int applysizehints(Client *client, Area *area, int interact);
static void applyrules(Client *client);                                         // apply client rules defined in config
static void attach(Client *client);                                             // attach new client to client list
static void attachstack(Client *client);                                        // attach client to it's monitors stack
static void configure(Client *client);                                          // configure new client
static void detach(Client *client);                                             // remove client from client list
static void detachstack(Client *client);                                        // remove client from it's monitors stack
static void focus(Client *client);                                              // focus given client
static Client *nexttiled(Client *client);                                       // get next tiled client
static void pop(Client *client);                                                // remove client from stack?
static void resize(Client *client, Area *area, int interact);                   // apply size hints
static void resizeclient(Client *client, Area *area);                           // resize client
static void sendmon(Client *client, Monitor *m);                                // send client to montior
static void setclientstate(Client *client, long state);                         // set the client state (normal/icon/withdrawn)
static void setfocus(Client *client);                                           // focus given client if focusable
static void setfullscreen(Client *client, int fullscreen);                      // set clients fullscreen state
static void seturgent(Client *client, int urgency_state);                       // set windows urgency state
static void showhide(Client *client);                                           // recursively show and hide windows in stack of given window
static void unfocus(Client *client, int setfocus);                              // ungrab button and update border
static void updatesizehints(Client *client);                                    // re-calculate client size hints
static void updatetitle(Client *client);                                        // get title from client, and set it to dwm title
static void updatewindowtype(Client *client);                                   // update fullscreen and floating window types
static void updatewmhints(Client *client);                                      // update urgency and input hints

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