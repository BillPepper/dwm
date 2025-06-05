#include "src/definitions.h"
#include "src/dwm.h"
#include <X11/keysym.h>
#include "src/exitdwm.c"

#include "src/debug.h"

// -- Appearance ---------------------------------------------------------------

static const int border_width = 3;
static const int gap = 15;
static const int snap = 32;


// -- Tray ---------------------------------------------------------------------

static const int systray_fail_pin_position = 1; // (0:first,1:last) monitor
static const int systray_pinned = 1; // (0: follow cursor, >0: index of monitor)
static const int systray_on_left = 0;
static const int systray_enabled = 1;
static const int systray_spacing = 2;


// -- Bar ----------------------------------------------------------------------

static const int is_bar_enabled = 1;
static const int is_top_bar = 1;
static const int status_monitor = 1; // index


// -- Fonts --------------------------------------------------------------------
static const char *fonts[] = { "monospace:size=10" };
static const char dmenufont[] = "monospace:size=10";


// - Colors --------------------------------------------------------------------
static const char col_gray[] = "#222222";
static const char col_green[] = "#4f772d";
static const char col_light_green[] = "#90a955";

static const char col_orange[] = "#fb8500";
static const char col_light_orange[] = "#ffb703";

static const char *colors[][3] = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_orange, col_gray, col_gray},
	[SchemeSel]  = { col_light_orange, col_gray,  col_light_orange  },
};

/* tagging */
static const char *tags[] = {
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"a",
	"b",
	"c",
	"d",
	"e",
	"f"
};

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "TestRule",     NULL,       NULL,       0,            1,           -1 },
	// { "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.5;					/* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;						/* number of clients in master area */
static const int resize_hints_enabled = 0;	/* 1 means respect size hints in tiled resizals */
static const int is_fullscreen_locked = 1;	/* 1 will force focus on the fullscreen window */

/* Layout symbols and corresponding arrance function callbacks. NULL = floating */
static const Layout layouts[] = {
	{ "[T]", tile },
	{ "[N]", NULL },
	{ "[M]", monocle },
};

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray, "-nf", col_green, "-sb", col_gray, "-sf", col_light_green, NULL };
static const char *termcmd[]  = { "alacritty", NULL };
static const char *termaltcmd[]  = { "urxvt", NULL };

/* Keys */
/*- modifier                key        function        argument */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,	view,       {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,	toggleview, {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,	tag,        {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,	toggletag,  {.ui = 1 << TAG} },

static const Key keys[] = {
	{ MODKEY, XK_b, toggle_bar, {0} },

	// command
	{ MODKEY,           XK_p,         spawn, {.v = dmenucmd } },
	{ MODKEY|ShiftMask, XK_Return,    spawn, {.v = termcmd } },
	{ MODKEY|ShiftMask, XK_BackSpace, spawn, {.v = termaltcmd } },

	// stacking
	{ MODKEY, XK_j, focusstack, {.i = +1 } },
	{ MODKEY, XK_k, focusstack, {.i = -1 } },
	{ MODKEY, XK_i, incnmaster, {.i = +1 } },
	{ MODKEY, XK_d, incnmaster, {.i = -1 } },
	{ MODKEY, XK_h, setmfact,   {.f = -0.05} },
	{ MODKEY, XK_l, setmfact,   {.f = +0.05} },

	// client
	{ MODKEY,           XK_Return, zoom,           {0} },
	{ MODKEY,           XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask, XK_c,      killclient,     {0} },

	// layout
	{ MODKEY,           XK_t,     setlayout,        {.v = &layouts[0]} },
	{ MODKEY,           XK_f,     setlayout,        {.v = &layouts[1]} },
	{ MODKEY,           XK_m,     setlayout,        {.v = &layouts[2]} },
	{ MODKEY,           XK_space, setlayout,        {0} },
	{ MODKEY|ShiftMask, XK_space, togglefloating,   {0} },
	{ MODKEY|ShiftMask, XK_f,     togglefullscreen, {0} },

	// monitor
	{ MODKEY, XK_comma,  focusmon, {.i = -1 } },
	{ MODKEY, XK_period, focusmon, {.i = +1 } },

	// tagging
	{ MODKEY,           XK_0,      view,   {.ui = ~0 } },
	{ MODKEY|ShiftMask, XK_0,      tag,    {.ui = ~0 } },
	{ MODKEY|ShiftMask, XK_comma,  tagmon, {.i = -1 } },
	{ MODKEY|ShiftMask, XK_period, tagmon, {.i = +1 } },

	// gap control
	{ MODKEY,           XK_minus, setgaps, {.i = -1 } },
	{ MODKEY,           XK_equal, setgaps, {.i = +1 } },
	{ MODKEY|ShiftMask, XK_equal, setgaps, {.i = 0  } },

	TAGKEYS(XK_1, 0)
	TAGKEYS(XK_2, 1)
	TAGKEYS(XK_3, 2)
	TAGKEYS(XK_4, 3)
	TAGKEYS(XK_5, 4)
	TAGKEYS(XK_6, 5)
	TAGKEYS(XK_7, 6)
	TAGKEYS(XK_8, 7)
	TAGKEYS(XK_9, 8)

	{ MODKEY|ShiftMask, XK_q, exitdwm, {0} },
	{ MODKEY|ShiftMask, XK_w, _debug, {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,  0,      Button1, setlayout,      {0} },
	{ ClkLtSymbol,  0,      Button3, setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,  0,      Button2, zoom,           {0} },
	{ ClkStatusText,0,      Button2, spawn,          {.v = termcmd } },
	{ ClkClientWin, MODKEY, Button1, movemouse,      {0} },
	{ ClkClientWin, MODKEY, Button2, togglefloating, {0} },
	{ ClkClientWin, MODKEY, Button3, resizemouse,    {0} },
	{ ClkTagBar,    0,      Button1, view,           {0} },
	{ ClkTagBar,    0,      Button3, toggleview,     {0} },
	{ ClkTagBar,    MODKEY, Button1, tag,            {0} },
	{ ClkTagBar,    MODKEY, Button3, toggletag,      {0} },
};
