#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "events.h"
#include "error.h"
#include "macro.h"
#include "input.h"
#include "bar.h"

#include "definitions.h"
#include "tagging.h"

// defined in config.h and global.h
extern void arrange(Monitor *monitor);

extern int resize_hints_enabled;
extern Rule rules[];
extern const char *tags[15];

extern Monitor *selected_monitor;
extern Display *display;
extern Atom wmatom[WMLast];
extern Atom netatom[NetLast];
extern Display *display;
extern Window root;
extern Monitor *monitors;
extern int screen_width;
extern int screen_height;
extern int bar_height;
extern char broken[];

// @desc: close client window
// @arg0: arg -> unused
void killclient(const Arg *arg);

// @desc: toggle floating for current client
// @arg0: arg -> unused
void togglefloating(const Arg *arg);

// @desc: toggle fullscreen for current client
// @arg0: arg -> unused
void togglefullscreen(const Arg *arg);

// @desc: update all clients on all monitors
void updateclientlist(void);

// @desc: apply size to client window
// @arg0: client   -> target client
// @arg1: area     -> target area
// @arg2: interact -> ?
int applysizehints(Client *client, Area *area, int interact);

// @descr: apply client rules defined in config
void apply_config_rules(Client *client);

// @descr: attach new client to client list
void attach(Client *client);

// @descr: attach client to it's monitors stack
void attachstack(Client *client);

// @descr: configure new client
void configure(Client *client);

// @descr: remove client from client list
void detach(Client *client);

// @descr: remove client from it's monitors stack
void detachstack(Client *client);

// @descr: focus given client
void focus(Client *client);

// @descr: get next tiled client
Client *nexttiled(Client *client);

// @descr: remove client from stack?
void pop(Client *client);

// @descr: apply size hints
void resize(Client *client, Area *area, int interact);

// @descr: resize client
void resizeclient(Client *client, Area *area);

// @descr: send client to montior
void sendmon(Client *client, Monitor *m);

// @descr: set the client state (normal/icon/withdrawn)
void setclientstate(Client *client, long state);

// @descr: focus given client if focusable
void setfocus(Client *client);

// @descr: set clients fullscreen state
void setfullscreen(Client *client, int fullscreen);

// @descr: set windows urgency state
void seturgent(Client *client, int urgency_state);

// @descr: recursively show and hide windows in stack of given window
void showhide(Client *client);

// @descr: ungrab button and update border
void unfocus(Client *client, int setfocus);

// @descr: re-calculate client size hints
void updatesizehints(Client *client);

// @descr: get title from client, and set it to dwm title
void updatetitle(Client *client);

// @descr: update fullscreen and floating window types
void updatewindowtype(Client *client);

// @descr: update urgency and input hints
void updatewmhints(Client *client);

#endif