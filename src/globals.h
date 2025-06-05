#ifndef GLOBALS_H
#define GLOBALS_H

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "definitions.h"
#include "drw.h"

extern int restart;                                                             // Should DWM restart or exit
extern int running;                                                             // Controls the loop in core:run()
extern int screen;                                                              // The default screen
extern int screen_width;                                                        // Width of (entire?) screen
extern int screen_height;                                                       // Height of (entire?) screen
extern int padding;                                                             // Sum of padding left+right
extern int bar_height;                                                          // Size of the status bar
extern Atom wmatom[WMLast];                                                     // (?) Window manager atom
extern Atom netatom[NetLast];                                                   // ?
extern Atom xatom[XLast];                                                       // ?
extern Window root;                                                             // Root window of dwm
extern Drw *drw;                                                                // Rendering implementations
extern Display *display;                                                        // Active display dwm uses
extern int font_count;                                                          // The count of fonts defined
extern const char *fonts[];                                                     // List of fonts
extern Cur *cursor[];                                                           // List of cursors
extern Window wmcheckwin;                                                       // (?) Something related to NetWMCheck
extern const char *colors[][3];                                                 // Colors dwm uses to render
extern int color_count;                                                         // Count of defined colors
extern int systray_enabled;                                                     // Is the system tray enabled
extern Monitor *selected_monitor;                                               // Currently selected monitor
extern Systray *systray;                                                        // The system tray
extern Monitor *monitors;                                                       // List of all monitors
extern char status_text[256];                                                   // Text to display in status area
extern int status_monitor;                                                      // Controls where to render status
extern Clr **scheme;                                                            // List of color schemes
extern const char *tags[15];                                                    // List of tag letters
extern int systray_on_left;                                                     // Should the tray be rendered on the left
extern int resize_hints_enabled;                                                // Controls window scaling behaviour
extern Rule rules[];                                                            // List of window rules
extern char broken[];                                                           // Fallback title for windows
extern int (*xerrorxlib)(Display *, XErrorEvent *);                             // Error callback function
extern int snap;                                                                // Window snapping size
extern unsigned int numlockmask;                                                // (?) Numlock stuff
extern int button_count;                                                        // Count of defined buttons
extern Button buttons[];                                                        // List of button handlers
extern int key_count;                                                           // Count of defined keys
extern Key keys[];                                                              // List of key handlers
extern int border_width;                                                        // Width of the window borders
extern int systray_spacing;                                                     //
extern int systray_pinned;                                                      // Display the tray should be pinned on
extern int systray_fail_pin_position;                                           // Fallback pin position
extern int is_bar_enabled;                                                      // Controls if the bar is rendered
extern int is_top_bar;                                                          // Controls if the bar should render in top of screen
extern float mfact;                                                             // Master area size factor
extern int nmaster;                                                             // Master area client count
extern int gap;                                                                 // Gap size between clients
extern Layout layouts[3];                                                       // List of available client layouts
extern char dmenumon[2];                                                        // (?) dmenu related
extern const char *dmenucmd[];                                                  // Array of commands, assembled to the command that dmenu runs
extern int is_fullscreen_locked;                                                // Force focus on fullscreen client

#endif