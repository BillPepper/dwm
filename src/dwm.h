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
#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// kernel
#include <sys/types.h>
#include <sys/wait.h>

// XINERAMA
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

//dwm
#include "drw.h"
#include "util.h"
#include "macro.h"
#include "definitions.h"
#include "globals.h"
#include "core.h"
#include "monitor.h"
#include "layout.h"
#include "events.h"
#include "stacking.h"
#include "input.h"
#include "debug.h"
#include "process.h"
#include "tagging.h"
#include "bar.h"
#include "client.h"
#include "window.h"
#include "error.h"

#include "../config.h"

#endif