#include "layout.h"

void monocle(Monitor *monitor){
	unsigned int client_count = 0;
	Client *client;
  Area area;
  int size;

  // count visible clients
	for (client = monitor->clients; client; client = client->next){
		if (ISVISIBLE(client)){
      client_count++;
    }
  }

  /* override layout symbol */
  size = sizeof monitor->layout_symbol;
	if (client_count > 0){
		snprintf(monitor->layout_symbol, size, "[%d]", client_count);
  }

  // resize all visible clients to window area
  client = next_tiled(monitor->clients);
	for (; client; client = next_tiled(client->next)){
    area.position.x = monitor->window_area.position.x;
    area.position.y = monitor->window_area.position.y;
    area.size.w = monitor->window_area.size.w - (client->border_width * 2);
    area.size.h = monitor->window_area.size.h - (client->border_width * 2);

		resize(client, &area, 0);
  }
}

void tile(Monitor *monitor) {
  unsigned int i, n, h, mw, my, ty;
  Client *c;
  Area area;

  for (n = 0, c = next_tiled(monitor->clients); c; c = next_tiled(c->next), n++);

  if (n == 0){
    return;
  }

  if (n > monitor->master_count){
    mw = monitor->master_count ? monitor->window_area.size.w * monitor->master_factor : 0;
  } else {
    mw = monitor->window_area.size.w - monitor->gap;
  }

  for (i = 0, my = ty = monitor->gap, c = next_tiled(monitor->clients); c; c = next_tiled(c->next), i++) {
    if (i < monitor->master_count) {
      h = (monitor->window_area.size.h - my) / (MIN(n, monitor->master_count) - i) - monitor->gap;

      area.position.x = monitor->window_area.position.x + monitor->gap;
      area.position.y = monitor->window_area.position.y + my;
      area.size.w = mw - (2 * c->border_width) - monitor->gap;
      area.size.h = h - (2 * c->border_width);

      resize(c, &area, 0);
      if (my + HEIGHT(c) + monitor->gap < monitor->window_area.size.h){
        my += HEIGHT(c) + monitor->gap;
	    }
    } else {
      h = (monitor->window_area.size.h - ty) / (n - i) - monitor->gap;

      area.position.x = monitor->window_area.position.x + mw + monitor->gap;
      area.position.y = monitor->window_area.position.y + ty;
      area.size.w = monitor->window_area.size.w - mw - (2 * c->border_width) - 2 * monitor->gap;
      area.size.h = h - (2 * c->border_width);

      resize(c, &area, 0);
      if (ty + HEIGHT(c) + monitor->gap < monitor->window_area.size.h){
        ty += HEIGHT(c) + monitor->gap;
	    }
    }
  }
}
