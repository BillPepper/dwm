#ifndef WINDOW_H
#define WINDOW_H

#include "stacking.h"

// window (X11)
void unmanage(Client *client, int destroyed);                            // detach, unfocus, update and re-arrange clients
void manage(Window window, XWindowAttributes *wa);                       // manage given window with dwm
long get_state(Window window);                                            // get window state
int get_text_prop(Window window, Atom atom, char *text, unsigned int size);// get text prop from window
Client *window_to_client(Window window);                                      // get dwm client for x11 window
Monitor *window_to_monitor(Window window);                                        // get monitor the given x11 window is on
Client *window_to_systray_icon(Window window);                                 // get icon (client) for given x11 window

#endif