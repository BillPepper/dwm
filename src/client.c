#include "client.h"

void killclient(const Arg *arg) {
  if (!selected_monitor->selected_client) {
    return;
  }

  if (!sendevent(selected_monitor->selected_client->window, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0, 0, 0)) {
    XGrabServer(display);
    XSetErrorHandler(xerrordummy);
    XSetCloseDownMode(display, DestroyAll);
    XKillClient(display, selected_monitor->selected_client->window);
    XSync(display, False);
    XSetErrorHandler(xerror);
    XUngrabServer(display);
  }
}

void togglefloating(const Arg *arg) {
  Area area;

  if (!selected_monitor->selected_client) {
    return;
  }

  /* no support for fullscreen windows */
  if (selected_monitor->selected_client->is_fullscreen) {
    return;
  }

  selected_monitor->selected_client->is_floating = !selected_monitor->selected_client->is_floating || selected_monitor->selected_client->is_fixed;
  if (selected_monitor->selected_client->is_floating) {
    area.position.x = selected_monitor->selected_client->area.position.x;
    area.position.y = selected_monitor->selected_client->area.position.y;
    area.size.w = selected_monitor->selected_client->area.size.w;
    area.size.h = selected_monitor->selected_client->area.size.h;

    resize(selected_monitor->selected_client, &area, 0);
  }

  arrange(selected_monitor);
}

void togglefullscreen(const Arg *arg) {
  if (selected_monitor->selected_client) {
    setfullscreen(selected_monitor->selected_client, !selected_monitor->selected_client->is_fullscreen);
  }
}
