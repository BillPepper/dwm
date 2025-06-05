#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>

#include "definitions.h"
#include "events.h"
#include "error.h"
#include "input.h"
#include "tagging.h"

// @desc: close client window
// @arg0: arg -> unused
void kill_client(const Arg *arg);

// @desc: toggle floating for current client
// @arg0: arg -> unused
void toggle_floating(const Arg *arg);

// @desc: toggle fullscreen for current client
// @arg0: arg -> unused
void toggle_fullscreen(const Arg *arg);

// @desc: update all clients on all monitors
void update_client_list(void);

// @desc: apply size to client window
// @arg0: client   -> target client
// @arg1: area     -> target area
// @arg2: interact -> ?
int apply_size_hints(Client *client, Area *area, int interact);

// @descr: apply client rules defined in config
void apply_config_rules(Client *client);

// @descr: attach new client to client list
void attach(Client *client);

// @descr: attach client to it's monitors stack
void attach_stack(Client *client);

// @descr: configure new client
void configure(Client *client);

// @descr: remove client from client list
void detach(Client *client);

// @descr: remove client from it's monitors stack
void detach_stack(Client *client);

// @descr: focus given client
void focus(Client *client);

// @descr: get next tiled client
Client *next_tiled(Client *client);

// @descr: remove client from stack?
void pop(Client *client);

// @descr: apply size hints
void resize(Client *client, Area *area, int interact);

// @descr: resize client
void resize_client(Client *client, Area *area);

// @descr: send client to montior
void send_to_monitor(Client *client, Monitor *monitor);

// @descr: set the client state (normal/icon/withdrawn)
void set_client_state(Client *client, long state);

// @descr: focus given client if focusable
void set_focus(Client *client);

// @descr: set clients fullscreen state
void set_fullscreen(Client *client, int fullscreen);

// @descr: set windows urgency state
void set_urgent(Client *client, int urgency_state);

// @descr: recursively show and hide windows in stack of given window
void show_hide(Client *client);

// @descr: ungrab button and update border
void unfocus(Client *client, int setfocus);

// @descr: re-calculate client size hints
void update_size_hints(Client *client);

// @descr: get title from client, and set it to dwm title
void update_title(Client *client);

// @descr: update fullscreen and floating window types
void update_window_type(Client *client);

// @descr: update urgency and input hints
void update_wm_hints(Client *client);

#endif