#ifndef DWM_H
#define DWM_H

#include "definitions.h"
#include <X11/Xlib.h>

/* function declarations */

// monitor
static Monitor *createmon(void);                        // create Monitor struct
static Monitor *dirtomon(int dir);                      // get monitor in direction?
static Monitor *recttomon(Area *area);                  // return the monitor given rect should be rendered on
static void focusmon(const Arg *arg);                   // focus monitor by index
static void arrange(Monitor *monitor);                  // arrange specified, or all monitors if m = NULL
static void arrangemon(Monitor *monitor);               // set layout string and call arrange callback
static void cleanupmon(Monitor *monitor);               // remove monitor

// layout 'arrange' function callbacks
static void monocle(Monitor *monitor);                  // monocle layout callback
static void tile(Monitor *monitor);                     // tile layout callback

// events
static void buttonpress(XEvent *event);
static void clientmessage(XEvent *event);
static void configurenotify(XEvent *event);
static void configurerequest(XEvent *event);
static void destroynotify(XEvent *event);
static void enternotify(XEvent *event);
static void expose(XEvent *event);
static void focusin(XEvent *event);
static void keypress(XEvent *event);
static void mappingnotify(XEvent *event);
static void maprequest(XEvent *event);
static void motionnotify(XEvent *event);
static void propertynotify(XEvent *event);
static void resizerequest(XEvent *event);
static void unmapnotify(XEvent *event);
static int sendevent(Window window, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);

// stacking
static void restack(Monitor *monitor);                                          // restack clients on monitor
static void focusstack(const Arg *arg);                                         // focus stack by index
static void incnmaster(const Arg *arg);                                         // increment master area
static void setgaps(const Arg *arg);                                            // set the monitor gaps to arg then call arrange()
static void setlayout(const Arg *arg);                                          // set the layout and the label on bar to arg, then arrange and/or update
static void setmfact(const Arg *arg);                                           // set master factor to arg

// mouse
static int getrootptr(int *x, int *y);                                          // get the monitor root window index of cursor position (TODO: use position struct)
static void movemouse(const Arg *arg);                                          // move mouse to another screen (arg not used?)
static void resizemouse(const Arg *arg);                                        // handle mouse resizing
static void grabbuttons(Client *client, int focused);                           // passivly grab mouse buttons

// processes and signals
static void sighup(int unused);                                                 // quit dwm with arg=1, which will restart dwm (unused arg)
static void sigterm(int unused);                                                // quit dwm with arg=0, which will end dwm (unused arg)
static void spawn(const Arg *arg);                                              // spawn a new process using fork()

// tagging
static void tag(const Arg *arg);
static void tagmon(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);

// keys
static void grabkeys(void);                                                     // passivly grab key inputs
static void updatenumlockmask(void);                                            // numlock stuff

// dwm core
static void setup(void);                                                        // init dwm, bar, tray, screen, etc...
static void run(void);                                                          // main loop, check events and handle them
static void scan(void);                                                         // scan for client windows
static void checkotherwm(void);                                                 // check if another wm is running
static void cleanup(void);                                                      // ungrab keys, destroy windows, etc...
static void quit(const Arg *arg);                                               // quit dwm, arg=1 for restart, 0 for end

// status and bars
static void togglebar(const Arg *arg);                                          // toggle bar (arg unused)
static void update_bar_position(Monitor *monitor);                              // recalculate bar position
static void drawbar(Monitor *monitor);                                          // draw bar, tags, layout and title
static void drawbars(void);                                                     // wrapper calling drawbar() for all monitors
static void resizebarwin(Monitor *monitor);                                     // resize bar and tray
static void updatestatus(void);                                                 // render current status text or fallback then update tray
static void updatebars(void);                                                   // set bar class hints, then render bar (top most) for each monitor

// tray (uses clients as icons, TODO: implement icon struct)
static void updatesystray(void);                                                // initializes (for some reason) and updates the tray
static unsigned int getsystraywidth();                                          // retreive the length of the tray area
static Monitor *systraytomon(Monitor *monitor);                                 // move tray to monitor, TODO: refactor or rewrite
static void removesystrayicon(Client *client);
static void updatesystrayicongeom(Client *client, int w, int h);                // (TODO: use size struct)
static void updatesystrayiconstate(Client *client, XPropertyEvent *event);

// client
static void killclient(const Arg *arg);                                         // close client window
static void togglefloating(const Arg *arg);                                     // toggle floating for current client (arg unused)
static void togglefullscreen(const Arg *arg);                                   // toggle fullscreen for current client (arg unused)
static void updateclientlist(void);                                             // update all clients on all monitors
static int applysizehints(Client *client, int *x, int *y, int *w, int *h, int interact); // (TODO: use area struct)
static void applyrules(Client *client);                                         // apply client rules defined in config
static void attach(Client *client);                                             // attach new client to client list
static void attachstack(Client *client);                                        // attach client to it's monitors stack
static void configure(Client *client);                                          // configure new client
static void detach(Client *client);                                             // remove client from client list
static void detachstack(Client *client);                                        // remove client from it's monitors stack
static void focus(Client *client);                                              // focus given client
static Client *nexttiled(Client *client);                                       // get next tiled client
static void pop(Client *client);                                                // remove client from stack? (TODO: use area struct)
static void resize(Client *client, int x, int y, int w, int h, int interact);   // apply size hints (TODO: use area struct)
static void resizeclient(Client *client, int x, int y, int w, int h);           // resize client (TODO: use area struct)
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
static void _debug();                                                           // debug
static void parse_args(int argc, char *argv[]);                                 // parse main() args

#endif