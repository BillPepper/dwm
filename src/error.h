#ifndef ERROR_H
#define ERROR_H

#include "globals.h"

// Error
static int xerror(Display *display, XErrorEvent *event);                        // handle errors
static int xerrordummy(Display *display, XErrorEvent *event);                   // returns 0
static int xerrorstart(Display *display, XErrorEvent *event);                   // called when other wm is running


#endif