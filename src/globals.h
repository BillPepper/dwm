#ifndef GLOBALS_H
#define GLOBALS_H

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "definitions.h"
#include "drw.h"

extern int restart;
extern int running;
extern int screen;
extern int screen_width;
extern int screen_height;
extern int padding;
extern int bar_height;
extern Atom wmatom[WMLast];
extern Atom netatom[NetLast];
extern Window root;
extern Drw *drw;
extern Display *display;
extern int font_count;
extern const char *fonts[];
extern Atom xatom[XLast];
extern Cur *cursor[];
extern Window wmcheckwin;
extern const char *colors[][3];
extern int color_count;
extern int systray_enabled;
extern Monitor *selected_monitor;
extern Systray *systray;
extern Monitor *monitors;
extern char status_text[256];
extern int status_monitor;
extern Clr **scheme;
extern const char *tags[15];
extern int systray_on_left;
extern int resize_hints_enabled;
extern Rule rules[];
extern char broken[];
extern int (*xerrorxlib)(Display *, XErrorEvent *);
extern Window root;
extern int snap;
extern unsigned int numlockmask;
extern int button_count;
extern Button buttons[];
extern int key_count;
extern Key keys[];
extern int border_width;
extern int systray_spacing;
extern int systray_pinned;
extern int systray_fail_pin_position;
extern Atom netatom[];
extern Atom xatom[];
extern int is_bar_enabled;
extern int is_top_bar;
extern float mfact;
extern int nmaster;
extern int gap;
extern Layout layouts[3];
extern char dmenumon[2];
extern const char *dmenucmd[];
extern int is_fullscreen_locked;

#endif