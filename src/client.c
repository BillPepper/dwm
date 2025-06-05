#include "client.h"

void killclient(const Arg *arg) {
  if (!selected_monitor->selected_client) {
    return;
  }

  if (!send_event(selected_monitor->selected_client->window, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0, 0, 0)) {
    XGrabServer(display);
    XSetErrorHandler(x_error_dummy);
    XSetCloseDownMode(display, DestroyAll);
    XKillClient(display, selected_monitor->selected_client->window);
    XSync(display, False);
    XSetErrorHandler(x_error);
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

void updateclientlist() {
  Client *client;
  Monitor *monitor;

  XDeleteProperty(display, root, netatom[NetClientList]);
  for (monitor = monitors; monitor; monitor = monitor->next) {
    for (client = monitor->clients; client; client = client->next) {
      XChangeProperty(display, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(client->window), 1);
	  }
  }
}

int applysizehints(Client *client, Area *area, int interact) {
  int baseismin;
  Monitor *monitor = client->monitor;

  int *x, *y, *width, *height;
  x = &area->position.x;
  y = &area->position.y;
  width = &area->size.w;
  height = &area->size.h;

  /* set minimum possible */
  *width = MAX(1, *width);
  *height = MAX(1, *height);
  if (interact) {
    if (*x > screen_width){
      *x = screen_width - WIDTH(client);
	  }
    if (*y > screen_height){
      *y = screen_height - HEIGHT(client);
	  }
    if (*x + *width + 2 * client->border_width < 0){
      *x = 0;
	  }
    if (*y + *height + 2 * client->border_width < 0){
      *y = 0;
	  }
  }

  else {
    if (*x >= monitor->window_area.position.x + monitor->window_area.size.w){
      *x = monitor->window_area.position.x + monitor->window_area.size.w - WIDTH(client);
	  }
    if (*y >= monitor->window_area.position.y + monitor->window_area.size.h){
      *y = monitor->window_area.position.y + monitor->window_area.size.h - HEIGHT(client);
	  }
    if (*x + *width + 2 * client->border_width <= monitor->window_area.position.x){
      *x = monitor->window_area.position.x;
	  }
    if (*y + *height + 2 * client->border_width <= monitor->window_area.position.y){
      *y = monitor->window_area.position.y;
	  }
  }

  // clamp height
  if (*height < bar_height){
    *height = bar_height;
  }

  // clamp width
  if (*width < bar_height){
    *width = bar_height;
  }

  // (?) deal with floating windows
  if (resize_hints_enabled || client->is_floating || !client->monitor->layout[client->monitor->selected_layout]->arrange_func) {
    if (!client->hintsvalid){
      updatesizehints(client);
	  }
    /* see last two sentences in ICCCM 4.1.2.3 */
    baseismin = client->base.w == client->min.w && client->base.h == client->min.h;

    /* temporarily remove base dimensions */
    if (!baseismin) {
      *width -= client->base.w;
      *height -= client->base.h;
    }

    /* adjust for aspect limits */
    if (client->aspect.min > 0 && client->aspect.max > 0) {
      if (client->aspect.max < (float)*width / *height)
        *width = *height * client->aspect.max + 0.5;
      else if (client->aspect.min < (float)*height / *width)
        *height = *width * client->aspect.min + 0.5;
    }

    /* increment calculation requires this */
    if (baseismin) {
      *width -= client->base.w;
      *height -= client->base.h;
    }

    /* adjust for increment value */
    if (client->inc.w){
      *width -= *width % client->inc.w;
	  }

    /* adjust for increment value */
    if (client->inc.h){
      *height -= *height % client->inc.h;
	  }

    /* restore base dimensions */
    *width = MAX(*width + client->base.w, client->min.w);
    *height = MAX(*height + client->base.h, client->min.h);

    if (client->max.w){
      *width = MIN(*width, client->max.w);
	  }

    if (client->max.h){
      *height = MIN(*height, client->max.h);
	  }
  }
  return *x != client->area.position.x || *y != client->area.position.y || *width != client->area.size.w || *height != client->area.size.h;
}

void apply_config_rules(Client *client) {
  const char *class;
  const char *instance;
  unsigned int i;
  const Rule *rule;
  Monitor *monitor;
  XClassHint class_hint = {NULL, NULL};

  bool is_title_match;
  bool is_class_match;
  bool is_instance_match;

  /* rule matching */
  client->is_floating = 0;
  client->tags = 0;
  XGetClassHint(display, client->window, &class_hint);
  class = class_hint.res_class ? class_hint.res_class : broken;
  instance = class_hint.res_name ? class_hint.res_name : broken;

  // for every rule in config
  // FIXME: RULE_COUNT was actually 'LENGTH(rules)' before, but it broke while separating source files
  #define RULE_COUNT 1
  for (i = 0; i < RULE_COUNT; i++) {
    rule = &rules[i];

    is_title_match = (!rule->title || strstr(client->name, rule->title));
    is_class_match = (!rule->class_name || strstr(class, rule->class_name));
    is_instance_match = (!rule->instance || strstr(instance, rule->instance));

    if (is_title_match && is_class_match && is_instance_match) {
      client->is_floating = rule->is_floating;
      client->tags |= rule->tags;

      // find monitor the rule applies to
      for (monitor = monitors; monitor && monitor->num != rule->monitor; monitor = monitor->next);

      if (monitor){
        client->monitor = monitor;
	    }
    }
  }

  if (class_hint.res_class){
    XFree(class_hint.res_class);
  }

  if (class_hint.res_name){
    XFree(class_hint.res_name);
  }

  client->tags = client->tags & TAGMASK ? client->tags & TAGMASK : client->monitor->tag_set[client->monitor->selected_tags];
}

void attach(Client *client) {
  client->next = client->monitor->clients;
  client->monitor->clients = client;
}

void attachstack(Client *client) {
  client->next_stack = client->monitor->stack;
  client->monitor->stack = client;
}

void configure(Client *client) {
  XConfigureEvent event;

  event.type = ConfigureNotify;
  event.display = display;
  event.event = client->window;
  event.window = client->window;
  event.x = client->area.position.x;
  event.y = client->area.position.y;
  event.width = client->area.size.w;
  event.height = client->area.size.h;
  event.border_width = client->border_width;
  event.above = None;
  event.override_redirect = False;

  XSendEvent(display, client->window, False, StructureNotifyMask, (XEvent *)&event);
}

void detach(Client *client) {
  Client **tc;

  for (tc = &client->monitor->clients; *tc && *tc != client; tc = &(*tc)->next);
  *tc = client->next;
}

void detachstack(Client *client) {
  Client **tc, *t;

  for (tc = &client->monitor->stack; *tc && *tc != client; tc = &(*tc)->next_stack);
  *tc = client->next_stack;

  if (client == client->monitor->selected_client) {
    for (t = client->monitor->stack; t && !ISVISIBLE(t); t = t->next_stack);
    client->monitor->selected_client = t;
  }
}

void focus(Client *client) {
  if (!client || !ISVISIBLE(client)) {
    for (client = selected_monitor->stack; client && !ISVISIBLE(client); client = client->next_stack);
  }
  if (selected_monitor->selected_client && selected_monitor->selected_client != client) {
    unfocus(selected_monitor->selected_client, 0);
  }
  if (client) {
    if (client->monitor != selected_monitor) {
      selected_monitor = client->monitor;
	  }
    if (client->is_urgent) {
      seturgent(client, 0);
	  }

    detachstack(client);
    attachstack(client);
    grab_buttons(client, 1);
    XSetWindowBorder(display, client->window, scheme[SchemeSel][ColBorder].pixel);
    setfocus(client);
  } else {
    XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(display, root, netatom[NetActiveWindow]);
  }
  selected_monitor->selected_client = client;
  draw_bars();
}

Client *nexttiled(Client *client) {
  for (; client && (client->is_floating || !ISVISIBLE(client)); client = client->next);
  return client;
}

void pop(Client *client) {
  detach(client);
  attach(client);
  focus(client);
  arrange(client->monitor);
}

void resize(Client *client, Area *area, int interact) {
  if (applysizehints(client, area, interact)) {
    resizeclient(client, area);
  }
}

void resizeclient(Client *client, Area *area) {
  XWindowChanges window_changes;

  client->old_area.position.x = client->area.position.x;
  client->area.position.x = window_changes.x = area->position.x;

  client->old_area.position.y = client->area.position.y;
  client->area.position.y = window_changes.y = area->position.y;

  client->old_area.size.w = client->area.size.w;
  client->area.size.w = window_changes.width = area->size.w;

  client->old_area.size.h = client->area.size.h;
  client->area.size.h = window_changes.height = area->size.h;

  window_changes.border_width = client->border_width;
  XConfigureWindow(display, client->window, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &window_changes);
  configure(client);
  XSync(display, False);
}

void sendmon(Client *client, Monitor *monitor){
  if (client->monitor == monitor) {
    return;
  }

  unfocus(client, 1);
  detach(client);
  detachstack(client);
  client->monitor = monitor;
  client->tags = monitor->tag_set[monitor->selected_tags]; /* assign tags of target monitor */
  attach(client);
  attachstack(client);
  focus(NULL);
  arrange(NULL);
}

void setclientstate(Client *client, long state){
  long data[] = {state, None};

  XChangeProperty(display, client->window, wmatom[WMState], wmatom[WMState], 32, PropModeReplace, (unsigned char *)data, 2);
}

void setfocus(Client *client) {
  if (!client->never_focus) {
    XSetInputFocus(display, client->window, RevertToPointerRoot, CurrentTime);
    XChangeProperty(display, root, netatom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&(client->window), 1);
  }
  send_event(client->window, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

void setfullscreen(Client *client, int fullscreen) {
  Area area;

  if (fullscreen && !client->is_fullscreen) {
    XChangeProperty(display, client->window, netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)&netatom[NetWMFullscreen], 1);
    client->is_fullscreen = 1;
    client->old_state = client->is_floating;
    client->old_border_width = client->border_width;
    client->border_width = 0;
    client->is_floating = 1;

    area.position.x = client->monitor->monitor_area.position.x;
    area.position.y = client->monitor->monitor_area.position.y;
    area.size.w = client->monitor->monitor_area.size.w;
    area.size.h = client->monitor->monitor_area.size.h;

    resizeclient(client, &area);
    XRaiseWindow(display, client->window);
  } else if (!fullscreen && client->is_fullscreen) {
    XChangeProperty(display, client->window, netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)0, 0);
    client->is_fullscreen = 0;
    client->is_floating = client->old_state;
    client->border_width = client->old_border_width;
    client->area.position.x = client->old_area.position.x;
    client->area.position.y = client->old_area.position.y;
    client->area.size.w = client->old_area.size.w;
    client->area.size.h = client->old_area.size.h;

    area.position.x = client->area.position.x;;
    area.position.y = client->area.position.y;;
    area.size.w = client->area.size.w;
    area.size.h = client->area.size.h;

    resizeclient(client, &area);
    arrange(client->monitor);
  }
}

void seturgent(Client *client, int urgency_state) {
  XWMHints *hints;

  client->is_urgent = urgency_state;
  if (!(hints = XGetWMHints(display, client->window))) {
    return;
  }

  hints->flags = urgency_state ? (hints->flags | XUrgencyHint) : (hints->flags & ~XUrgencyHint);
  XSetWMHints(display, client->window, hints);
  XFree(hints);
}

void showhide(Client *client) {
  Area area;

  if (!client){
    return;
  }

  if (ISVISIBLE(client)) {
    /* show clients top down */
    XMoveWindow(display, client->window, client->area.position.x, client->area.position.y);
    if ((!client->monitor->layout[client->monitor->selected_layout]->arrange_func || client->is_floating) && !client->is_fullscreen) {
      area.position.x = client->area.position.x;
      area.position.y = client->area.position.y;
      area.size.w = client->area.size.w;
      area.size.h = client->area.size.h;

      resize(client, &area, 0);
	  }
    showhide(client->next_stack);
  } else {
    /* hide clients bottom up */
    showhide(client->next_stack);
    XMoveWindow(display, client->window, WIDTH(client) * -2, client->area.position.y);
  }
}

void unfocus(Client *client, int setfocus) {
  if (!client) {
    return;
  }

  grab_buttons(client, 0);
  XSetWindowBorder(display, client->window, scheme[SchemeNorm][ColBorder].pixel);
  if (setfocus) {
    XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(display, root, netatom[NetActiveWindow]);
  }
}

void updatesizehints(Client *client) {
  long master_size;
  XSizeHints size;

  /* if size is uninitialized, ensure that size.flags aren't used */
  if (!XGetWMNormalHints(display, client->window, &size, &master_size)) {
    size.flags = PSize;
  }


  if (size.flags & PBaseSize) {
    client->base.w = size.base_width;
    client->base.h = size.base_height;
  } else if (size.flags & PMinSize) {
    client->base.w = size.min_width;
    client->base.h = size.min_height;
  } else {
    client->base.w = client->base.h = 0;
  }
  if (size.flags & PResizeInc) {
    client->inc.w = size.width_inc;
    client->inc.h = size.height_inc;
  } else {
    client->inc.w = client->inc.h = 0;
  }
  if (size.flags & PMaxSize) {
    client->max.w = size.max_width;
    client->max.h = size.max_height;
  } else {
    client->max.w = client->max.h = 0;
  }
  if (size.flags & PMinSize) {
    client->min.w = size.min_width;
    client->min.h = size.min_height;
  } else if (size.flags & PBaseSize) {
    client->min.w = size.base_width;
    client->min.h = size.base_height;
  } else {
    client->min.w = client->min.h = 0;
  }
  if (size.flags & PAspect) {
    client->aspect.min = (float)size.min_aspect.y / size.min_aspect.x;
    client->aspect.max = (float)size.max_aspect.x / size.max_aspect.y;
  } else {
    client->aspect.max = client->aspect.min = 0.0;
  }

  client->is_fixed = (client->max.w && client->max.h && client->max.w == client->min.w && client->max.h == client->min.h);
  client->hintsvalid = 1;
}

void updatetitle(Client *client) {
  if (!get_text_prop(client->window, netatom[NetWMName], client->name, sizeof client->name)) {
    get_text_prop(client->window, XA_WM_NAME, client->name, sizeof client->name);
  }

  /* hack to mark broken clients */
  if (client->name[0] == '\0') {
    strcpy(client->name, broken);
  }
}

void updatewindowtype(Client *client) {
  Atom state = get_atom_prop(client, netatom[NetWMState]);
  Atom type = get_atom_prop(client, netatom[NetWMWindowType]);

  if (state == netatom[NetWMFullscreen]) {
    setfullscreen(client, 1);
  }
  if (type == netatom[NetWMWindowTypeDialog]) {
    client->is_floating = 1;
  }
}

void updatewmhints(Client *client) {
  XWMHints *hints;

  if ((hints = XGetWMHints(display, client->window))) {
    if (client == selected_monitor->selected_client && hints->flags & XUrgencyHint){
      hints->flags &= ~XUrgencyHint;
      XSetWMHints(display, client->window, hints);
    } else {
      client->is_urgent = (hints->flags & XUrgencyHint) ? 1 : 0;
	}

    if (hints->flags & InputHint) {
      client->never_focus = !hints->input;
	}
    else {
      client->never_focus = 0;
	}

    XFree(hints);
  }
}