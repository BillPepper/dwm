#ifndef INPUT_H
#define INPUT_H

#include "definitions.h"

// mouse
static int getrootptr(int *x, int *y);                                          // get the monitor root window index of cursor position (TODO: use position struct)
static void movemouse(const Arg *arg);                                          // move mouse to another screen (arg not used?)
static void resizemouse(const Arg *arg);                                        // handle mouse resizing
static void grabbuttons(Client *client, int focused);                           // passivly grab mouse buttons

// keys
static void grabkeys(void);                                                     // passivly grab key inputs
static void updatenumlockmask(void);                                            // numlock stuff

#endif