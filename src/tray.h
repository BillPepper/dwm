#ifndef TRAY_H
#define TRAY_H

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/Xatom.h>

#include <stdlib.h>
#include <stdio.h>

#include "definitions.h"
#include "core.h"
#include "drw.h"
#include "util.h"
#include "macro.h"
#include "client.h"


extern char status_text[256];
extern int padding;
extern int systray_spacing;
extern Display *display;
extern int systray_enabled;
extern int systray_on_left;
extern Systray *systray;
extern Window root;
extern int systray_pinned;
extern Monitor *selected_monitor;
extern int bar_height;
extern Clr **scheme;
extern Monitor *monitors;
extern int systray_fail_pin_position;
extern Drw *drw;
extern Atom netatom[];
extern Atom xatom[];

extern int sendevent(Window window, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);

// tray (uses clients as icons, TODO: implement icon struct)
void updatesystray(void);                                                // initializes (for some reason) and updates the tray
unsigned int getsystraywidth();                                          // retreive the length of the tray area
Monitor *systraytomon(Monitor *monitor);                                 // move tray to monitor, TODO: refactor or rewrite
void removesystrayicon(Client *client);                                  // remove tray icon
void updatesystrayicongeom(Client *client, Size *size);                  // ?
void updatesystrayiconstate(Client *client, XPropertyEvent *event);      // ?


#endif // TRAY_H