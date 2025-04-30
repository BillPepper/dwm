#include "monitor.h"


Monitor *createmon(void) {
  Monitor *monitor;

  monitor = ecalloc(1, sizeof(Monitor));
  monitor->tag_set[0] = monitor->tag_set[1] = 1;
  monitor->master_factor = mfact;
  monitor->master_count = nmaster;
  monitor->bar_enabled = is_bar_enabled;
  monitor->is_topbar = is_top_bar;
  monitor->gap = gap;
  monitor->layout[0] = &layouts[0];
  monitor->layout[1] = &layouts[1 % LENGTH(layouts)];
  strncpy(monitor->layout_symbol, layouts[0].symbol, sizeof monitor->layout_symbol);

  return monitor;
}

Monitor *dirtomon(int dir) {
  Monitor *monitor = NULL;

  if (dir > 0) {
    if (!(monitor = selected_monitor->next)){
      monitor = monitors;
	  }
  } else if (selected_monitor == monitors){
    for (monitor = monitors; monitor->next; monitor = monitor->next);
  }
  else {
    for (monitor = monitors; monitor->next != selected_monitor; monitor = monitor->next);
  }

  return monitor;
}

Monitor *recttomon(Area *area) {
  Monitor *monitor, *r = selected_monitor;
  int a;
  int area_val = 0; // used to be 'area' until I used the area struct as arg
  int x, y, w, h;

  x = area->position.x;
  y = area->position.y;
  w = area->size.w;
  h = area->size.h;

  for (monitor = monitors; monitor; monitor = monitor->next) {
    if ((a = INTERSECT(x, y, w, h, monitor)) > area_val) {
      area_val = a;
      r = monitor;
    }
  }

  return r;
}


void focusmon(const Arg *arg) {
  Monitor *m;

  if (!monitors->next) {
    return;
  }
  if ((m = dirtomon(arg->i)) == selected_monitor) {
    return;
  }

  unfocus(selected_monitor->selected_client, 0);
  selected_monitor = m;
  focus(NULL);
}


void arrange(Monitor *monitor) {
  // if monitor specified
  if (monitor){
    showhide(monitor->stack);
  }

  // otherwise, do it for all
  else {
    for (monitor = monitors; monitor; monitor = monitor->next){
      showhide(monitor->stack);
	  }
  }

  // again, if monitor specified
  if (monitor) {
    arrangemon(monitor);
    restack(monitor);
  }

  // again, do it for all if not specified
  else {
    for (monitor = monitors; monitor; monitor = monitor->next){
      arrangemon(monitor);
	  }
  }
}

void arrangemon(Monitor *monitor) {
  strncpy(monitor->layout_symbol, monitor->layout[monitor->selected_layout]->symbol, sizeof monitor->layout_symbol);
  if (monitor->layout[monitor->selected_layout]->arrange_func){
    monitor->layout[monitor->selected_layout]->arrange_func(monitor);
  }
}

void cleanupmon(Monitor *mon) {
  Monitor *m;

  if (mon == monitors){
    monitors = monitors->next;
  }
  else {
    for (m = monitors; m && m->next != mon; m = m->next);
    m->next = mon->next;
  }
  XUnmapWindow(display, mon->bar_window);
  XDestroyWindow(display, mon->bar_window);
  free(mon);
}