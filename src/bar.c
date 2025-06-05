#include "bar.h"

void toggle_bar(const Arg *arg) {
  selected_monitor->bar_enabled = !selected_monitor->bar_enabled;
  update_bar_position(selected_monitor);
  resize_bar_win(selected_monitor);

  if (systray_enabled) {
    XWindowChanges wc;
    if (!selected_monitor->bar_enabled) {
      wc.y = -bar_height;
	  }
    else if (selected_monitor->bar_enabled) {
      wc.y = 0;
      if (!selected_monitor->is_topbar){
        wc.y = selected_monitor->monitor_area.size.h - bar_height;
	    }
    }
    XConfigureWindow(display, systray->window, CWY, &wc);
  }
  arrange(selected_monitor);
}

void update_bar_position(Monitor *monitor) {
  monitor->window_area.position.y = monitor->monitor_area.position.y;
  monitor->window_area.size.h = monitor->monitor_area.size.h;

  if (monitor->bar_enabled) {
    monitor->window_area.size.h -= bar_height;
    monitor->bar_y = monitor->is_topbar ? monitor->window_area.position.y : monitor->window_area.position.y + monitor->window_area.size.h;
    monitor->window_area.position.y = monitor->is_topbar ? monitor->window_area.position.y + bar_height : monitor->window_area.position.y;
  } else {
    monitor->bar_y = -bar_height;
  }
}

void draw_bar(Monitor *monitor) {
  int x, w, text_width = 0, tray_width = 0;
  int boxs = drw->fonts->h / 9;
  int boxw = drw->fonts->h / 6 + 2;
  unsigned int i, occ = 0, is_urgent = 0;
  Client *client;

  if (!monitor->bar_enabled){
    return;
  }

  if (systray_enabled && monitor == systray_to_mon(monitor) && !systray_on_left){
    tray_width = get_systray_width();
  }

  /* draw status first so it can be overdrawn by tags later */
  drw_setscheme(drw, scheme[SchemeNorm]);
  text_width = TEXTW(status_text) - padding / 2 + 2; /* 2px extra right padding */
  drw_text(drw, monitor->window_area.size.w - text_width - tray_width, 0, text_width, bar_height, padding / 2 - 2, status_text, 0);

  resize_bar_win(monitor);

  // mark urgent tags
  for (client = monitor->clients; client; client = client->next) {
    occ |= client->tags;
    if (client->is_urgent){
      is_urgent |= client->tags;
	  }
  }

  // render tags
  x = 0;
  for (i = 0; i < LENGTH(tags); i++) {
    w = TEXTW(tags[i]);

    // render the tag
    drw_setscheme(drw, scheme[monitor->tag_set[monitor->selected_tags] & 1 << i ? SchemeSel : SchemeNorm]);
    drw_text(drw, x, 0, w, bar_height, padding / 2, tags[i], is_urgent & 1 << i);

    // draw inverted tag if urgent
    if (occ & 1 << i) {
      drw_rect(drw, x + boxs, boxs, boxw, boxw, monitor == selected_monitor && selected_monitor->selected_client && selected_monitor->selected_client->tags & 1 << i, is_urgent & 1 << i);
	  }

    // set position for next tag
    x += w;
  }

  // render layout
  w = TEXTW(monitor->layout_symbol);
  drw_setscheme(drw, scheme[SchemeNorm]);
  x = drw_text(drw, x, 0, w, bar_height, padding / 2, monitor->layout_symbol, 0);

  // render title of last highlighted client
  if ((w = monitor->window_area.size.w - text_width - tray_width - x) > bar_height) {
    if (monitor->selected_client) {
      drw_setscheme(drw, scheme[monitor == selected_monitor ? SchemeSel : SchemeNorm]);
      drw_text(drw, x, 0, w, bar_height, padding / 2, monitor->selected_client->name, 0);

      // render the small indicator when client is floating
      if (monitor->selected_client->is_floating){
        drw_rect(drw, x + boxs, boxs, boxw, boxw, monitor->selected_client->is_fixed, 0);
	    }
    } else {
      drw_setscheme(drw, scheme[SchemeNorm]);
      drw_rect(drw, x, 0, w, bar_height, 1, 1);
    }
  }

  // not sure what this does...
  drw_map(drw, monitor->bar_window, 0, 0, monitor->window_area.size.w - tray_width, bar_height);
}

void draw_bars(void) {
  Monitor *m;

  for (m = monitors; m; m = m->next){
    draw_bar(m);
  }
}

void resize_bar_win(Monitor *monitor) {
  unsigned int width = monitor->window_area.size.w;

  if (systray_enabled && monitor == systray_to_mon(monitor) && !systray_on_left) {
    width -= get_systray_width();
  }

  XMoveResizeWindow(display, monitor->bar_window, monitor->window_area.position.x, monitor->bar_y, width, bar_height);
}

void update_status(void) {
  // default status text
  if (!get_text_prop(root, XA_WM_NAME, status_text, sizeof(status_text))) {
    strcpy(status_text, "pdwm-" VERSION);
  }

  // show status on all screens
  if (status_monitor) {
    Monitor *m;

    for (m = monitors; m; m = m->next) {
      draw_bar(m);
    }
  }

  // show bar on selected screen
  else {
    draw_bar(selected_monitor);
  }

  update_systray();
}

void update_bars(void) {
  unsigned int width;
  Monitor *monitor;
  XSetWindowAttributes window_attributes = {.override_redirect = True, .background_pixmap = ParentRelative, .event_mask = ButtonPressMask | ExposureMask};
  XClassHint class_hint = {"dwm", "dwm"};

  // loop monitors
  for (monitor = monitors; monitor; monitor = monitor->next) {

    // check if monitor has bar
    if (monitor->bar_window){
      continue;
	  }

    // calculate how long the bar is without the systray
    width = monitor->window_area.size.w;
    if (systray_enabled && monitor == systray_to_mon(monitor)){
      width -= get_systray_width();
	  }

    // create bar
    monitor->bar_window = XCreateWindow(display, root, monitor->window_area.position.x, monitor->bar_y, width, bar_height, 0, DefaultDepth(display, screen), CopyFromParent, DefaultVisual(display, screen), CWOverrideRedirect | CWBackPixmap | CWEventMask, &window_attributes);

    // set cursor
    XDefineCursor(display, monitor->bar_window, cursor[CurNormal]->cursor);

    // raise bar parts
    if (systray_enabled && monitor == systray_to_mon(monitor)){
      XMapRaised(display, systray->window);
	  }
    XMapRaised(display, monitor->bar_window);

    XSetClassHint(display, monitor->bar_window, &class_hint);
  }
}
