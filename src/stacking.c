#include "stacking.h"


void restack(Monitor *monitor) {
  Client *client;
  XEvent event;
  XWindowChanges window_changes;

  drawbar(monitor);

  if (!monitor->selected_client){
    return;
  }

  // floating clients. if layout callback is NULL, float by default
  if (monitor->selected_client->is_floating || !monitor->layout[monitor->selected_layout]->arrange_func){
    XRaiseWindow(display, monitor->selected_client->window);
  }

  // re-stack clients
  if (monitor->layout[monitor->selected_layout]->arrange_func) {
    window_changes.stack_mode = Below;
    window_changes.sibling = monitor->bar_window;

    for (client = monitor->stack; client; client = client->next_stack) {
      if (!client->is_floating && ISVISIBLE(client)) {
        XConfigureWindow(display, client->window, CWSibling | CWStackMode, &window_changes);
        window_changes.sibling = client->window;
      }
	  }
  }

  XSync(display, False);
  while (XCheckMaskEvent(display, EnterWindowMask, &event));
}


void focusstack(const Arg *arg) {
  Client *c = NULL, *i;

  if (!selected_monitor->selected_client || (selected_monitor->selected_client->is_fullscreen && is_fullscreen_locked)) {
    return;
  }

  if (arg->i > 0) {
    for (c = selected_monitor->selected_client->next; c && !ISVISIBLE(c); c = c->next);
    if (!c){
      for (c = selected_monitor->clients; c && !ISVISIBLE(c); c = c->next);
    }
  } else {
    for (i = selected_monitor->clients; i != selected_monitor->selected_client; i = i->next){
      if (ISVISIBLE(i)){
        c = i;
      }
    }
    if (!c) {
      for (; i; i = i->next) {
        if (ISVISIBLE(i)) {
          c = i;
        }
      }
    }
  }
  if (c) {
    focus(c);
    restack(selected_monitor);
  }
}


void incnmaster(const Arg *arg) {
  selected_monitor->master_count = MAX(selected_monitor->master_count + arg->i, 0);
  arrange(selected_monitor);
}



void setgaps(const Arg *arg) {
  if ((arg->i == 0) || (selected_monitor->gap + arg->i < 0)) {
    selected_monitor->gap = 0;
  } else {
    selected_monitor->gap += arg->i;
  }

  arrange(selected_monitor);
}

void setlayout(const Arg *arg) {
  // has no or invalid arg, reset to default?
  if (!arg || !arg->v || arg->v != selected_monitor->layout[selected_monitor->selected_layout]) {
    selected_monitor->selected_layout ^= 1;
  }

  // has valid arg, set layout
  if (arg && arg->v) {
    selected_monitor->layout[selected_monitor->selected_layout] = (Layout *)arg->v;
  }

  // copy layout name to display
  strncpy(selected_monitor->layout_symbol, selected_monitor->layout[selected_monitor->selected_layout]->symbol, sizeof selected_monitor->layout_symbol);

  // this condition is not clear to me...
  if (selected_monitor->selected_client) {
    arrange(selected_monitor);
  } else {
    drawbar(selected_monitor);
  }
}

void setmfact(const Arg *arg) {
  /* arg > 1.0 will set mfact absolutely */

  float f;

  if (!arg || !selected_monitor->layout[selected_monitor->selected_layout]->arrange_func) {
    return;
  }

  f = arg->f < 1.0 ? arg->f + selected_monitor->master_factor : arg->f - 1.0;
  if (f < 0.05 || f > 0.95) {
    return;
  }

  selected_monitor->master_factor = f;
  arrange(selected_monitor);
}
