#ifndef TRAY_H
#define TRAY_H

#include <X11/Xatom.h>

#include "core.h"
#include "macro.h"
#include "client.h"

// tray (uses clients as icons, TODO: implement icon struct)
void update_systray(void);                                                      // initializes (for some reason) and updates the tray
unsigned int get_systray_width();                                               // retreive the length of the tray area
Monitor *systray_to_mon(Monitor *monitor);                                      // move tray to monitor, TODO: refactor or rewrite
void remove_systray_icon(Client *client);                                       // remove tray icon
void update_systray_icon_geom(Client *client, Size *size);                      // ?
void update_systray_icon_state(Client *client, XPropertyEvent *event);          // ?


#endif // TRAY_H