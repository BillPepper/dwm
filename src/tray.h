#ifndef TRAY_H
#define TRAY_H

#include <X11/Xlib.h>

#include "definitions.h"

// tray (uses clients as icons, TODO: implement icon struct)
void updatesystray(void);                                                // initializes (for some reason) and updates the tray
unsigned int getsystraywidth();                                          // retreive the length of the tray area
Monitor *systraytomon(Monitor *monitor);                                 // move tray to monitor, TODO: refactor or rewrite
void removesystrayicon(Client *client);                                  // remove tray icon
void updatesystrayicongeom(Client *client, Size *size);                  // ?
void updatesystrayiconstate(Client *client, XPropertyEvent *event);      // ?


#endif // TRAY_H