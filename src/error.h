#ifndef ERROR_H
#define ERROR_H

#include <X11/Xproto.h>
#include <X11/Xft/Xft.h>

#include "util.h"
#include "globals.h"

// Error
int x_error(Display *display, XErrorEvent *event);                               // handle errors
int x_error_dummy(Display *display, XErrorEvent *event);                          // returns 0
int x_error_start(Display *display, XErrorEvent *event);                          // called when other wm is running

#endif