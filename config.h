#include "src/definitions.h"
#include <X11/keysym.h>
#include "src/exitdwm.c"

#include "src/debug.h"
#include "src/layout.h"

void tile(Monitor *m);

// -- Appearance ---------------------------------------------------------------

int border_width = 3;
int gap = 15;
int snap = 32;


// -- Tray ---------------------------------------------------------------------

int systray_fail_pin_position = 1; // (0:first,1:last) monitor
int systray_pinned = 1; // (0: follow cursor, >0: index of monitor)
int systray_on_left = 0;
int systray_enabled = 1;
int systray_spacing = 2;


// -- Bar ----------------------------------------------------------------------

int is_bar_enabled = 1;
int is_top_bar = 1;
int status_monitor = 1; // index


// -- Fonts --------------------------------------------------------------------
int font_count = 1;
const char *fonts[] = { "monospace:size=10" };
char dmenufont[] = "monospace:size=10";


// - Colors --------------------------------------------------------------------
static const char col_gray[] = "#222222";
static const char col_green[] = "#4f772d";
static const char col_light_green[] = "#90a955";

static const char col_orange[] = "#fb8500";
static const char col_light_orange[] = "#ffb703";

int color_count = 2;
const char *colors[][3] = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_orange, col_gray, col_gray},
	[SchemeSel]  = { col_light_orange, col_gray,  col_light_orange  },
};

/* tagging */
const char *tags[15] = {
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d","e", "f"
};

// FIXME: When adding rules, you need to update the count in client.c:apply_config_rules()
Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "TestRule",     NULL,       NULL,       0,            1,           -1 },
	// { "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
float mfact     = 0.5;					/* factor of master area size [0.05..0.95] */
int nmaster     = 1;						/* number of clients in master area */
int resize_hints_enabled = 0;	/* 1 means respect size hints in tiled resizals */
int is_fullscreen_locked = 1;	/* 1 will force focus on the fullscreen window */

/* Layout symbols and corresponding arrance function callbacks. NULL = floating */
Layout layouts[3] = {
	{ "[T]", tile },
	{ "[F]", NULL },
	{ "[M]", monocle },
};

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray, "-nf", col_green, "-sb", col_gray, "-sf", col_light_green, NULL };
static const char *termcmd[]  = { "alacritty", NULL };
static const char *termaltcmd[]  = { "urxvt", NULL };

/* -- Keys -- */
/* -- modifier                key        function        argument -- */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,	view,       {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,	toggle_view, {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,	tag,        {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,	toggle_tag,  {.ui = 1 << TAG} },

int key_count = 66;
Key keys[] = {
	{ MODKEY, XK_b, toggle_bar, {0} },																							// Mod + B

	// command
	{ MODKEY,           XK_p,         spawn, {.v = dmenucmd } },									// Mod + P
	{ MODKEY|ShiftMask, XK_Return,    spawn, {.v = termcmd } },										// Mod + Shift + Enter
	{ MODKEY|ShiftMask, XK_BackSpace, spawn, {.v = termaltcmd } },								// Mod + Shift + Backspace

	// stacking
	{ MODKEY, XK_j, focus_stack, {.i = +1 } },																			// Mod + J
	{ MODKEY, XK_k, focus_stack, {.i = -1 } },																			// Mod + K
	{ MODKEY, XK_i, increment_master, {.i = +1 } },																			// Mod + I
	{ MODKEY, XK_d, increment_master, {.i = -1 } },																			// Mod + D
	{ MODKEY, XK_h, set_master_factor,   {.f = -0.05} },																		// Mod + H
	{ MODKEY, XK_l, set_master_factor,   {.f = +0.05} },																		// Mod + L

	// client
	{ MODKEY,           XK_Return, zoom,           {0} },													// Mod + Return
	{ MODKEY,           XK_Tab,    view,           {0} },													// Mod + Tab
	{ MODKEY|ShiftMask, XK_c,      killclient,     {0} },													// Mod + Shift + C

	// layout
	{ MODKEY,           XK_t,     set_layout,        {.v = &layouts[0]} },					// Mod + T
	{ MODKEY,           XK_f,     set_layout,        {.v = &layouts[1]} },					// Mod + F
	{ MODKEY,           XK_m,     set_layout,        {.v = &layouts[2]} },					// Mod + M
	{ MODKEY,           XK_space, set_layout,        {0} },												// Mod + Shift + Space
	{ MODKEY|ShiftMask, XK_space, togglefloating,   {0} },												// Mod + Shift + Space
	{ MODKEY|ShiftMask, XK_f,     togglefullscreen, {0} },												// Mod + Shift + F

	// monitor
	{ MODKEY, XK_comma,  focus_monitor, {.i = -1 } },																	// Mod + ,
	{ MODKEY, XK_period, focus_monitor, {.i = +1 } },																	// Mod + .

	// tagging
	{ MODKEY,           XK_0,      view,   {.ui = ~0 } },													// Mod + 0
	{ MODKEY|ShiftMask, XK_0,      tag,    {.ui = ~0 } },													// Mod + Shift + 0
	{ MODKEY|ShiftMask, XK_comma,  tag_monitor, {.i = -1 } },													// Mod + Shift + ,
	{ MODKEY|ShiftMask, XK_period, tag_monitor, {.i = +1 } },													// Mod + Shift + .

	// gap control
	{ MODKEY,           XK_minus, set_gaps, {.i = -1 } },													// Mod + -
	{ MODKEY,           XK_equal, set_gaps, {.i = +1 } },													// Mod + =
	{ MODKEY|ShiftMask, XK_equal, set_gaps, {.i = 0  } },													// Mod + Shift + =

	TAGKEYS(XK_1, 0)
	TAGKEYS(XK_2, 1)
	TAGKEYS(XK_3, 2)
	TAGKEYS(XK_4, 3)
	TAGKEYS(XK_5, 4)
	TAGKEYS(XK_6, 5)
	TAGKEYS(XK_7, 6)
	TAGKEYS(XK_8, 7)
	TAGKEYS(XK_9, 8)

	{ MODKEY|ShiftMask, XK_q, exitdwm, {0} },																			// Mod + Shift + Q
	{ MODKEY|ShiftMask, XK_w, _debug, {0}},																				// Mod + Shift + W
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
int button_count = 11;
Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,  0,      Button1, set_layout,      {0} },
	{ ClkLtSymbol,  0,      Button3, set_layout,      {.v = &layouts[2]} },
	{ ClkWinTitle,  0,      Button2, zoom,           {0} },
	{ ClkStatusText,0,      Button2, spawn,          {.v = termcmd } },
	{ ClkClientWin, MODKEY, Button1, move_mouse,      {0} },
	{ ClkClientWin, MODKEY, Button2, togglefloating, {0} },
	{ ClkClientWin, MODKEY, Button3, resize_mouse,    {0} },
	{ ClkTagBar,    0,      Button1, view,           {0} },
	{ ClkTagBar,    0,      Button3, toggle_view,     {0} },
	{ ClkTagBar,    MODKEY, Button1, tag,            {0} },
	{ ClkTagBar,    MODKEY, Button3, toggle_tag,      {0} },
};
