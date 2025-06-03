#ifndef INPUT_H
#define INPUT_H

#include "definitions.h"

// -- Mouse --------------------------------------------------------------------

// @descr: Get the monitor root window index of cursor position (TODO: use position struct)
// @param: x -> mouse x position, y -> mouse y position
static int getrootptr(int *x, int *y);

// @descr: Move mouse to another screen (arg not used?)
// @param: unused
static void movemouse(const Arg *arg);

// @descr: Handle mouse resizing
// @param: unused
static void resizemouse(const Arg *arg);

// @descr: Passivly grab mouse buttons
// @param: client -> the client buttons should be grabbed for (?)
// @param: focused -> ?
static void grabbuttons(Client *client, int focused);


// -- Keys ---------------------------------------------------------------------

// @descr: Passivly grab key inputs
// @param: none
static void grabkeys(void);

// @descr:
// @param:
static void updatenumlockmask(void);                                            // numlock stuff

#endif