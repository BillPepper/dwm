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
