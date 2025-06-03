#ifndef INPUT_H
#define INPUT_H

#include "definitions.h"

// -- Mouse --------------------------------------------------------------------

// @descr: Get the monitor root window index of cursor position (TODO: use position struct)
// @param: x -> mouse x position, y -> mouse y position
int getrootptr(int *x, int *y);

// @descr: Move mouse to another screen (arg not used?)
// @param: unused
void movemouse(const Arg *arg);

// @descr: Handle mouse resizing
// @param: unused
void resizemouse(const Arg *arg);

// @descr: Passivly grab mouse buttons
// @param: client -> the client buttons should be grabbed for (?)
// @param: focused -> ?
void grabbuttons(Client *client, int focused);


// -- Keys ---------------------------------------------------------------------

// @descr: Passivly grab key inputs
// @param: none
void grabkeys(void);

// @descr:
// @param:
void updatenumlockmask(void);                                            // numlock stuff

#endif