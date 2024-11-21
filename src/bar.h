#ifndef BAR_H
#define BAR_H

#include <X11/Xlib.h>
#include "definitions.h"

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
static void removesystrayicon(Client *client);                                  // remove tray icon
static void updatesystrayicongeom(Client *client, Size *size);                  // ?
static void updatesystrayiconstate(Client *client, XPropertyEvent *event);      // ?

#endif