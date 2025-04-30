#ifndef TRAY_H
#define TRAY_H

#include <X11/Xlib.h>

#include "definitions.h"

// tray (uses clients as icons, TODO: implement icon struct)
static void updatesystray(void);                                                // initializes (for some reason) and updates the tray
static unsigned int getsystraywidth();                                          // retreive the length of the tray area
static Monitor *systraytomon(Monitor *monitor);                                 // move tray to monitor, TODO: refactor or rewrite
static void removesystrayicon(Client *client);                                  // remove tray icon
static void updatesystrayicongeom(Client *client, Size *size);                  // ?
static void updatesystrayiconstate(Client *client, XPropertyEvent *event);      // ?


#endif // TRAY_H