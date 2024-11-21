#ifndef WINDOW_H
#define WINDOW_H

#include <X11/Xlib.h>
#include "definitions.h"

// window (X11)
static void unmanage(Client *client, int destroyed);                            // detach, unfocus, update and re-arrange clients
static void manage(Window window, XWindowAttributes *wa);                       // manage given window with dwm
static long getstate(Window window);                                            // get window state
static int gettextprop(Window window, Atom atom, char *text, unsigned int size);// get text prop from window
static Client *wintoclient(Window window);                                      // get dwm client for x11 window
static Monitor *wintomon(Window window);                                        // get monitor the given x11 window is on
static Client *wintosystrayicon(Window window);                                 // get icon (client) for given x11 window

#endif