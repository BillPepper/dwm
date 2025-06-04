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
  client = nexttiled(monitor->clients);
	for (; client; client = nexttiled(client->next)){
    area.position.x = monitor->window_area.position.x;
    area.position.y = monitor->window_area.position.y;
    area.size.w = monitor->window_area.size.w - (client->border_width * 2);
    area.size.h = monitor->window_area.size.h - (client->border_width * 2);

		resize(client, &area, 0);
  }
}

void tile(Monitor *m) {
  unsigned int i, n, h, mw, my, ty;
  Client *c;
  Area area;

  for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);

  if (n == 0){
    return;
  }

  if (n > m->master_count){
    mw = m->master_count ? m->window_area.size.w * m->master_factor : 0;
  } else {
    mw = m->window_area.size.w - m->gap;
  }

  for (i = 0, my = ty = m->gap, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
    if (i < m->master_count) {
      h = (m->window_area.size.h - my) / (MIN(n, m->master_count) - i) - m->gap;

      area.position.x = m->window_area.position.x + m->gap;
      area.position.y = m->window_area.position.y + my;
      area.size.w = mw - (2 * c->border_width) - m->gap;
      area.size.h = h - (2 * c->border_width);

      resize(c, &area, 0);
      if (my + HEIGHT(c) + m->gap < m->window_area.size.h){
        my += HEIGHT(c) + m->gap;
	    }
    } else {
      h = (m->window_area.size.h - ty) / (n - i) - m->gap;

      area.position.x = m->window_area.position.x + mw + m->gap;
      area.position.y = m->window_area.position.y + ty;
      area.size.w = m->window_area.size.w - mw - (2 * c->border_width) - 2 * m->gap;
      area.size.h = h - (2 * c->border_width);

      resize(c, &area, 0);
      if (ty + HEIGHT(c) + m->gap < m->window_area.size.h){
        ty += HEIGHT(c) + m->gap;
	    }
    }
  }
}
