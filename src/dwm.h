#ifndef DWM_H
#define DWM_H

// X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xft/Xft.h>

// stdlib
#include <unistd.h>
#include <locale.h>
#include <stdbool.h>

// kernel
#include <sys/types.h>
#include <sys/wait.h>

// XINERAMA
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

//dwm
#include "globals.h"

#include "../config.h"

#endif