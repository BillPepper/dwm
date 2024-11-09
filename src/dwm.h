#ifndef DWM_H
#define DWM_H

#include "definitions.h"
#include <X11/Xlib.h>

/* function declarations */
static Monitor *createmon(void); // create Monitor struct
static Monitor *dirtomon(int dir); // get monitor in direction?
static Monitor *recttomon(int x, int y, int w, int h); // which monitor is rect on?

// layout
static void monocle(Monitor *m);  // monocle layout callback
static void tile(Monitor *m);     // tile layout callback

static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void cleanupmon(Monitor *mon);
static void drawbar(Monitor *m);
static void resizebarwin(Monitor *m);
static void restack(Monitor *m);
static void updatebarpos(Monitor *m);

static void buttonpress(XEvent *e);
static void clientmessage(XEvent *e);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static void destroynotify(XEvent *e);
static void drawbars(void);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void focusin(XEvent *e);
static void keypress(XEvent *e);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void motionnotify(XEvent *e);
static void propertynotify(XEvent *e);
static void resizerequest(XEvent *e);
static void unmapnotify(XEvent *e);

static void focusmon(const Arg *arg);
static void focusstack(const Arg *arg);
static int getrootptr(int *x, int *y);

static void incnmaster(const Arg *arg);
static void killclient(const Arg *arg);
static void movemouse(const Arg *arg);
static void resizemouse(const Arg *arg);
static void setgaps(const Arg *arg);
static void setlayout(const Arg *arg);
static void setmfact(const Arg *arg);
static void setup(void);
static void sighup(int unused);
static void sigterm(int unused);
static void spawn(const Arg *arg);
static void tag(const Arg *arg);
static void tagmon(const Arg *arg);
static void togglebar(const Arg *arg);
static void togglefloating(const Arg *arg);
static void togglefullscreen(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);

static void grabkeys(void);
static int updategeom(void);
static void updatenumlockmask(void);

static void run(void);
static void scan(void);
static void checkotherwm(void);
static void cleanup(void);
static void quit(const Arg *arg);

// tray and status and bars
static void updatestatus(void);
static void updatesystray(void);
static void updatebars(void);
static unsigned int getsystraywidth();
static Monitor *systraytomon(Monitor *m);
static void removesystrayicon(Client *i);
static void updatesystrayicongeom(Client *i, int w, int h);
static void updatesystrayiconstate(Client *i, XPropertyEvent *ev);

// client
static void updateclientlist(void);
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void applyrules(Client *c);
static void attach(Client *c);
static void attachstack(Client *c);
static void configure(Client *c);
static void detach(Client *c);
static void detachstack(Client *c);
static void focus(Client *c);
static Atom getatomprop(Client *c, Atom prop);
static void grabbuttons(Client *c, int focused);
static Client *nexttiled(Client *c);
static void pop(Client *c);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void sendmon(Client *c, Monitor *m);
static void setclientstate(Client *c, long state);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
static void seturgent(Client *c, int urg);
static void showhide(Client *c);
static void unfocus(Client *c, int setfocus);
static void unmanage(Client *c, int destroyed);

static void updatesizehints(Client *c);
static void updatetitle(Client *c);
static void updatewindowtype(Client *c);
static void updatewmhints(Client *c);

static long getstate(Window w);
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void manage(Window w, XWindowAttributes *wa);
static int sendevent(Window w, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);

// window
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static Client *wintosystrayicon(Window w);

// Error
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);

// Other
static void view(const Arg *arg);
static void zoom(const Arg *arg);

// Custom
static void _debug();
static void parse_args(int argc, char *argv[]);

#endif