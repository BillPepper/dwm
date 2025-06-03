#ifndef WINDOW_H
#define WINDOW_H

#include <X11/Xlib.h>

#include "definitions.h"
#include "stacking.h"

extern int border_width;

// window (X11)
void unmanage(Client *client, int destroyed);                            // detach, unfocus, update and re-arrange clients
void manage(Window window, XWindowAttributes *wa);                       // manage given window with dwm
long getstate(Window window);                                            // get window state
int gettextprop(Window window, Atom atom, char *text, unsigned int size);// get text prop from window
Client *wintoclient(Window window);                                      // get dwm client for x11 window
Monitor *wintomon(Window window);                                        // get monitor the given x11 window is on
Client *wintosystrayicon(Window window);                                 // get icon (client) for given x11 window

#endif