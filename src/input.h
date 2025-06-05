#ifndef INPUT_H
#define INPUT_H

#include "stacking.h"

// -- Mouse --------------------------------------------------------------------

// @descr: Get the monitor root window index of cursor position (TODO: use position struct)
// @param: x -> mouse x position, y -> mouse y position
int get_root_ptr(int *x, int *y);

// @descr: Move mouse to another screen (arg not used?)
// @param: unused
void move_mouse(const Arg *arg);

// @descr: Handle mouse resizing
// @param: unused
void resize_mouse(const Arg *arg);

// @descr: Passivly grab mouse buttons
// @param: client -> the client buttons should be grabbed for (?)
// @param: focused -> ?
void grab_buttons(Client *client, int focused);


// -- Keys ---------------------------------------------------------------------

// @descr: Passivly grab key inputs
// @param: none
void grab_keys(void);

// @descr:
// @param:
void update_numlock_mask(void);                                            // numlock stuff

#endif