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


void detach(Client *c) {
  Client **tc;

  for (tc = &c->monitor->clients; *tc && *tc != c; tc = &(*tc)->next);
  *tc = c->next;
}

void detachstack(Client *c) {
  Client **tc, *t;

  for (tc = &c->monitor->stack; *tc && *tc != c; tc = &(*tc)->next_stack);
  *tc = c->next_stack;

  if (c == c->monitor->selected_client) {
    for (t = c->monitor->stack; t && !ISVISIBLE(t); t = t->next_stack);
    c->monitor->selected_client = t;
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
    grabbuttons(client, 1);
    XSetWindowBorder(display, client->window, scheme[SchemeSel][ColBorder].pixel);
    setfocus(client);
  } else {
    XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(display, root, netatom[NetActiveWindow]);
  }
  selected_monitor->selected_client = client;
  drawbars();
}
