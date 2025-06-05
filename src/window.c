#include "window.h"

void unmanage(Client *client, int destroyed) {
  Monitor *monitor = client->monitor;
  XWindowChanges window_changes;

  detach(client);
  detach_stack(client);
  if (!destroyed) {
    window_changes.border_width = client->old_border_width;
    XGrabServer(display); /* avoid race conditions */
    XSetErrorHandler(x_error_dummy);
    XSelectInput(display, client->window, NoEventMask);
    XConfigureWindow(display, client->window, CWBorderWidth, &window_changes); /* restore border */
    XUngrabButton(display, AnyButton, AnyModifier, client->window);
    set_client_state(client, WithdrawnState);
    XSync(display, False);
    XSetErrorHandler(x_error);
    XUngrabServer(display);
  }
  free(client);
  focus(NULL);
  update_client_list();
  arrange(monitor);
}

void manage(Window window, XWindowAttributes *window_attributes) {
  // Appears to create a client struct and tie the input x window to it


  Client *client;
  Client *t = NULL;
  Window trans = None;
  XWindowChanges window_changes;

  int client_x, client_y;
  int win_area_x, win_area_y, win_area_w, win_area_h;
  int monitor_area_x, monitor_area_y, monitor_area_w, monitor_area_h;

  // Create client struct
  client = ecalloc(1, sizeof(Client));
  client->window = window;

  /* geometry */
  client->area.position.x = window_attributes->x;
  client->area.position.y = window_attributes->y;
  client->area.size.w = window_attributes->width;
  client->area.size.h = window_attributes->height;

  client->old_area.position.x = window_attributes->x;
  client->old_area.position.y = window_attributes->y;
  client->old_area.size.w = window_attributes->width;
  client->old_area.size.h = window_attributes->height;

  client->old_border_width = window_attributes->border_width;

  // Update the client struct's title with current one of the x window
  update_title(client);

  // (?) if window is a transient, find client by it's window to set position
  if (XGetTransientForHint(display, window, &trans) && (t = window_to_client(trans))){
    client->monitor = t->monitor;
    client->tags = t->tags;
  } else {
    client->monitor = selected_monitor;
    apply_config_rules(client);
  }

  client_x = client->area.position.x;
  client_y = client->area.position.y;

  win_area_x = client->monitor->window_area.position.x;
  win_area_y = client->monitor->window_area.position.y;
  win_area_w = client->monitor->window_area.size.w;
  win_area_h = client->monitor->window_area.size.h;

  if (client_x + WIDTH(client) > win_area_x + win_area_w){
    client->area.position.x = win_area_x + win_area_w - WIDTH(client);
  }

  if (client_y + HEIGHT(client) > win_area_y + win_area_h){
    client->area.position.y = win_area_y + win_area_h - HEIGHT(client);
  }

  client->area.position.x = MAX(client_x, win_area_x);
  client->area.position.y = MAX(client_y, win_area_y);
  client->border_width = border_width;

  window_changes.border_width = client->border_width;
  XConfigureWindow(display, window, CWBorderWidth, &window_changes);
  XSetWindowBorder(display, window, scheme[SchemeNorm][ColBorder].pixel);
  configure(client); /* propagates border_width, if size doesn't change */
  update_window_type(client);
  update_size_hints(client);
  update_wm_hints(client);

  monitor_area_x = client->monitor->monitor_area.position.x;
  monitor_area_y = client->monitor->monitor_area.position.y;
  monitor_area_w = client->monitor->monitor_area.size.w;
  monitor_area_h = client->monitor->monitor_area.size.h;

  // set windows to center (patch)
  client->area.position.x = monitor_area_x + (monitor_area_w - WIDTH(client)) / 2;
  client->area.position.y = monitor_area_y + (monitor_area_h - HEIGHT(client)) / 2;

  XSelectInput(display, window, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
  grab_buttons(client, 0);
  if (!client->is_floating) {
    client->is_floating = client->old_state = trans != None || client->is_fixed;
  }
  if (client->is_floating) {
    XRaiseWindow(display, client->window);
  }

  attach(client);
  attach_stack(client);

  XChangeProperty(display, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(client->window), 1);
  XMoveResizeWindow(display, client->window, client_x + 2 * screen_width, client_y, client->area.size.w, client->area.size.h); /* some windows require this */
  set_client_state(client, NormalState);

  if (client->monitor == selected_monitor) {
    unfocus(selected_monitor->selected_client, 0);
  }

  client->monitor->selected_client = client;

  arrange(client->monitor);
  XMapWindow(display, client->window);
  focus(NULL);
}

long get_state(Window window) {
  int format;
  long result = -1;
  unsigned char *ptr = NULL;
  unsigned long n, extra;
  Atom real;

  if (XGetWindowProperty(display, window, wmatom[WMState], 0L, 2L, False, wmatom[WMState], &real, &format, &n, &extra, (unsigned char **)&ptr) != Success) {
    return -1;
  }

  if (n != 0) {
    result = *ptr;
  }

  XFree(ptr);
  return result;
}

int get_text_prop(Window window, Atom atom, char *text, unsigned int size) {
  char **list = NULL;
  int n;
  XTextProperty name;

  if (!text || size == 0) {
    return 0;
  }

  text[0] = '\0';
  if (!XGetTextProperty(display, window, &name, atom) || !name.nitems) {
    return 0;
  }

  if (name.encoding == XA_STRING) {
    strncpy(text, (char *)name.value, size - 1);
  } else if (XmbTextPropertyToTextList(display, &name, &list, &n) >= Success && n > 0 && *list) {
    strncpy(text, *list, size - 1);
    XFreeStringList(list);
  }
  text[size - 1] = '\0';
  XFree(name.value);
  return 1;
}

Client *window_to_client(Window window) {
  Client *client;
  Monitor *monitor;

  for (monitor = monitors; monitor; monitor = monitor->next) {
    for (client = monitor->clients; client; client = client->next) {
      if (client->window == window){
        return client;
	    }
	  }
  }

  return NULL;
}

Monitor *window_to_monitor(Window window) {
  int x, y;
  Client *client;
  Monitor *monitor;
  Area area;

  area.position.x = 0;
  area.position.y = 0;
  area.size.w = 1;
  area.size.h = 1;

  if (window == root && get_root_ptr(&x, &y)) {
    return rect_to_monitor(&area);
  }

  for (monitor = monitors; monitor; monitor = monitor->next) {
    if (window == monitor->bar_window){
      return monitor;
	  }
  }

  if ((client = window_to_client(window))) {
    return client->monitor;
  }

  return selected_monitor;
}

Client *window_to_systray_icon(Window window) {
  Client *icons = NULL;

  if (!systray_enabled || !window) {
    return icons;
  }

  for (icons = systray->icons; icons && icons->window != window; icons = icons->next);

  return icons;
}
