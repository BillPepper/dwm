#ifndef DWM_H
#define DWM_H

#include "definitions.h"
#include <X11/Xlib.h>

/* function declarations */

// monitor
static Monitor *createmon(void);                        // create Monitor struct
static Monitor *dirtomon(int dir);                      // get monitor in direction?
static Monitor *recttomon(int x, int y, int w, int h);  // return the monitor given rect should be rendered on
static void focusmon(const Arg *arg);                   // focus monitor by index

// layout 'arrange' function callbacks
static void monocle(Monitor *monitor);                  // monocle layout callback
static void tile(Monitor *monitor);                     // tile layout callback

static void arrange(Monitor *monitor);                  // arrange specified, or all monitors if m = NULL
static void arrangemon(Monitor *monitor);               // set layout string and call arrange callback
static void cleanupmon(Monitor *monitor);               // remove monitor

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

// stacking
static void restack(Monitor *monitor);                  // restack clients on monitor
static void focusstack(const Arg *arg);                 // focus stack by index
static void incnmaster(const Arg *arg);                 // increment master area
static void setgaps(const Arg *arg);                    // set the monitor gaps to arg then call arrange()
static void setlayout(const Arg *arg);                  // set the layout and the label on bar to arg, then arrange and/or update
static void setmfact(const Arg *arg);                   // set master factor to arg

// mouse
static int getrootptr(int *x, int *y);                  // get the monitor root window index of cursor position
static void movemouse(const Arg *arg);                  // move mouse to another screen (arg not used?)
static void resizemouse(const Arg *arg);                // handle mouse resizing

// processes and signals
static void sighup(int unused);                         // quit dwm with arg=1, which will restart dwm (unused arg)
static void sigterm(int unused);                        // quit dwm with arg=0, which will end dwm (unused arg)
static void spawn(const Arg *arg);                      // spawn a new process using fork()

// tagging
static void tag(const Arg *arg);
static void tagmon(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);

// keys
static void grabkeys(void);
static void updatenumlockmask(void);

// dwm core
static void setup(void);
static void run(void);
static void scan(void);
static void checkotherwm(void);
static void cleanup(void);
static void quit(const Arg *arg);

// status and bars
static void togglebar(const Arg *arg);              // toggle bar (arg unused)
static void update_bar_position(Monitor *monitor);  // recalculate bar position
static void drawbar(Monitor *monitor);              // draw bar, tags, layout and title
static void drawbars(void);                         // wrapper calling drawbar() for all monitors
static void resizebarwin(Monitor *monitor);         // resize bar and tray
static void updatestatus(void);                     // render current status text or fallback then update tray
static void updatebars(void);                       // set bar class hints, then render bar (top most) for each monitor

// tray
static void updatesystray(void);
static unsigned int getsystraywidth();
static Monitor *systraytomon(Monitor *m);
static void removesystrayicon(Client *i);
static void updatesystrayicongeom(Client *i, int w, int h);
static void updatesystrayiconstate(Client *i, XPropertyEvent *ev);

// client
static void killclient(const Arg *arg);           // close client window
static void togglefloating(const Arg *arg);       // toggle floating for current client (arg unused)
static void togglefullscreen(const Arg *arg);     // toggle fullscreen for current client (arg unused)
static void updateclientlist(void);               // update all clients on all monitors
static int applysizehints(Client *client, int *x, int *y, int *w, int *h, int interact);
static void applyrules(Client *client);           // apply client rules defined in config
static void attach(Client *client);               // attach new client to client list
static void attachstack(Client *client);          // attach client to it's monitors stack
static void configure(Client *client);            // configure new client
static void detach(Client *client);               // remove client from client list
static void detachstack(Client *client);          // remove client from it's monitors stack
static void focus(Client *client);                // focus given client


static void grabbuttons(Client *client, int focused);
static Client *nexttiled(Client *client);
static void pop(Client *client);
static void resize(Client *client, int x, int y, int w, int h, int interact);
static void resizeclient(Client *client, int x, int y, int w, int h);
static void sendmon(Client *client, Monitor *m);
static void setclientstate(Client *client, long state);
static void setfocus(Client *client);
static void setfullscreen(Client *client, int fullscreen);
static void seturgent(Client *client, int urg);
static void showhide(Client *client);
static void unfocus(Client *client, int setfocus);
static void unmanage(Client *client, int destroyed);

static void updatesizehints(Client *client);
static void updatetitle(Client *client);
static void updatewindowtype(Client *client);
static void updatewmhints(Client *client);

static long getstate(Window window);
static int gettextprop(Window window, Atom atom, char *text, unsigned int size);
static void manage(Window window, XWindowAttributes *wa);
static int sendevent(Window window, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);

// window
static Client *wintoclient(Window window);
static Monitor *wintomon(Window window);
static Client *wintosystrayicon(Window window);

// Error
static int xerror(Display *display, XErrorEvent *event);
static int xerrordummy(Display *display, XErrorEvent *event);
static int xerrorstart(Display *display, XErrorEvent *event);

// atom stuff
static Atom getatomprop(Client *client, Atom prop);

// Other
static void view(const Arg *arg);
static void zoom(const Arg *arg); // zooms the master to the next client
static int updategeom(void);

// Custom
static void _debug();
static void parse_args(int argc, char *argv[]);

#endif