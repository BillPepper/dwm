#include "tray.h"

void update_systray(void) {
  XSetWindowAttributes window_attributes;
  XWindowChanges window_changes;
  Client *client;
  Monitor *monitor = systray_to_mon(NULL);
  unsigned int x = monitor->monitor_area.position.x + monitor->monitor_area.size.w;
  unsigned int status_width = TEXTW(status_text) - padding + systray_spacing;
  unsigned int width = 1;

  if (!systray_enabled) {
    return;
  }

  if (systray_on_left) {
    x -= status_width + padding / 2;
  }

  // TODO: why is the tray initialized in the update function? should have an init function!
  // init systray
  if (!systray) {
    if (!(systray = (Systray *)calloc(1, sizeof(Systray)))) {
      die("fatal: could not malloc() %u bytes\n", sizeof(Systray));
	  }

    // create tray window
    systray->window = XCreateSimpleWindow(display, root, x, monitor->bar_y, width, bar_height, 0, 0, scheme[SchemeSel][ColBg].pixel);
    window_attributes.event_mask = ButtonPressMask | ExposureMask;
    window_attributes.override_redirect = True;
    window_attributes.background_pixel = scheme[SchemeNorm][ColBg].pixel;

    // add class to systray
    XClassHint ch = { .res_class="dwm_tray", .res_name="dwm_tray"};
    XSetClassHint(display, systray->window, &ch);

    XSelectInput(display, systray->window, SubstructureNotifyMask);
    XChangeProperty(display, systray->window, netatom[NetSystemTrayOrientation], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&netatom[NetSystemTrayOrientationHorz], 1);
    XChangeWindowAttributes(display, systray->window, CWEventMask | CWOverrideRedirect | CWBackPixel, &window_attributes);
    XMapRaised(display, systray->window);
    XSetSelectionOwner(display, netatom[NetSystemTray], systray->window, CurrentTime);

    if (XGetSelectionOwner(display, netatom[NetSystemTray]) == systray->window) {
      send_event(root, xatom[Manager], StructureNotifyMask, CurrentTime, netatom[NetSystemTray], systray->window, 0, 0);
      XSync(display, False);
    } else {
      fprintf(stderr, "dwm: unable to obtain system tray.\n");
      free(systray);
      systray = NULL;
      return;
    }
  }

  for (width = 0, client = systray->icons; client; client = client->next) {
    /* make sure the background color stays the same */
    window_attributes.background_pixel = scheme[SchemeNorm][ColBg].pixel;

    XChangeWindowAttributes(display, client->window, CWBackPixel, &window_attributes);
    XMapRaised(display, client->window);
    width += systray_spacing;
    client->area.position.x = width;
    XMoveResizeWindow(display, client->window, client->area.position.x, 0, client->area.size.w, client->area.size.h);
    width += client->area.size.w;
    if (client->monitor != monitor) {
      client->monitor = monitor;
	  }
  }

  width = width ? width + systray_spacing : 1;
  x -= width;
  XMoveResizeWindow(display, systray->window, x, monitor->bar_y, width, bar_height);
  window_changes.x = x;
  window_changes.y = monitor->bar_y;
  window_changes.width = width;
  window_changes.height = bar_height;
  window_changes.stack_mode = Above;
  window_changes.sibling = monitor->bar_window;
  XConfigureWindow(display, systray->window, CWX | CWY | CWWidth | CWHeight | CWSibling | CWStackMode, &window_changes);
  XMapWindow(display, systray->window);
  XMapSubwindows(display, systray->window);

  /* redraw background */
  XSetForeground(display, drw->gc, scheme[SchemeNorm][ColBg].pixel);
  XFillRectangle(display, systray->window, drw->gc, 0, 0, width, bar_height);
  XSync(display, False);
}

unsigned int get_systray_width() {
  unsigned int w = 0;
  Client *i;

  if (systray_enabled) {
    for (i = systray->icons; i; w += i->area.size.w + systray_spacing, i = i->next);
  }

  return w ? w + systray_spacing : 1;
}

Monitor *systray_to_mon(Monitor *monitor) {
  Monitor *current_monitor;
  int i, n;

  // move systray along with current monitor if enabled
  if (!systray_pinned) {
    if (!monitor) {
      return selected_monitor;
	  }

    return monitor == selected_monitor ? monitor : NULL;
  }

  for (n = 1, current_monitor = monitors; current_monitor && current_monitor->next; n++, current_monitor = current_monitor->next);
  for (i = 1, current_monitor = monitors; current_monitor && current_monitor->next && i < systray_pinned; i++, current_monitor = current_monitor->next);

  if (systray_fail_pin_position && n < systray_pinned) {
    return monitors;
  }

  return current_monitor;
}

void remove_systray_icon(Client *client) {
  Client **client2;

  // systray disabled
  if (!systray_enabled || !client) {
    return;
  }

  for (client2 = &systray->icons; *client2 && *client2 != client; client2 = &(*client2)->next);

  if (client2) {
    *client2 = client->next;
  }

  free(client);
}

void update_systray_icon_geom(Client *client, Size *size) {
  Area area;
  int w, h;

  w = size->w;
  h = size->h;

  if (client) {
    client->area.size.h = bar_height;
    if (w == h) {
      client->area.size.w = bar_height;
	  } else if (h == bar_height) {
      client->area.size.w = w;
	  } else {
      client->area.size.w = (int)((float)bar_height * ((float)w / (float)h));
	  }

    area.position.x = client->area.position.x;
    area.position.y = client->area.position.y;
    area.size.w = client->area.size.w;
    area.size.h = client->area.size.h;

    apply_size_hints(client, &area, False);
    /* force icons into the systray dimensions if they don't want to */
    if (client->area.size.h > bar_height) {
      if (client->area.size.w == client->area.size.h) {
        client->area.size.w = bar_height;
	    } else {
        client->area.size.w = (int)((float)bar_height * ((float)client->area.size.w / (float)client->area.size.h));
	    }

      client->area.size.h = bar_height;
    }
  }
}

void update_systray_icon_state(Client *client, XPropertyEvent *event) {
  long flags;
  int code = 0;

  if (!systray_enabled || !client || event->atom != xatom[XembedInfo] || !(flags = get_atom_prop(client, xatom[XembedInfo]))) {
    return;
  }

  if (flags & XEMBED_MAPPED && !client->tags) {
    client->tags = 1;
    code = XEMBED_WINDOW_ACTIVATE;
    XMapRaised(display, client->window);
    set_client_state(client, NormalState);
  } else if (!(flags & XEMBED_MAPPED) && client->tags) {
    client->tags = 0;
    code = XEMBED_WINDOW_DEACTIVATE;
    XUnmapWindow(display, client->window);
    set_client_state(client, WithdrawnState);
  } else {
    return;
  }

  send_event(client->window, xatom[Xembed], StructureNotifyMask, CurrentTime, code, 0, systray->window, XEMBED_EMBEDDED_VERSION);
}
