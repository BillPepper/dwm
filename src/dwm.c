#include "dwm.h"

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags {
  char limitexceeded[LENGTH(tags) > 31 ? -1 : 1];
};

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
  for (i = 0; i < LENGTH(rules); i++) {
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

// apply size hints to client
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
  } else {
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
  if (*height < bar_height){
    *height = bar_height;
  }
  if (*width < bar_height){
    *width = bar_height;
  }
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
    if (baseismin) { /* increment calculation requires this */
      *width -= client->base.w;
      *height -= client->base.h;
    }
    /* adjust for increment value */
    if (client->inc.w){
      *width -= *width % client->inc.w;
	  }
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

void attach(Client *client) {
  client->next = client->monitor->clients;
  client->monitor->clients = client;
}

void attachstack(Client *client) {
  client->next_stack = client->monitor->stack;
  client->monitor->stack = client;
}

void buttonpress(XEvent *e) {
  unsigned int i, x, click;
  Arg arg = {0};
  Client *c;
  Monitor *m;
  XButtonPressedEvent *ev = &e->xbutton;

  click = ClkRootWin;
  /* focus monitor if necessary */
  if ((m = wintomon(ev->window)) && m != selected_monitor) {
    unfocus(selected_monitor->selected_client, 1);
    selected_monitor = m;
    focus(NULL);
  }
  if (ev->window == selected_monitor->bar_window) {
    i = x = 0;
    do {
      x += TEXTW(tags[i]);
	}
    while (ev->x >= x && ++i < LENGTH(tags));

    if (i < LENGTH(tags)) {
      click = ClkTagBar;
      arg.ui = 1 << i;
    } else if (ev->x < x + TEXTW(selected_monitor->layout_symbol)){
      click = ClkLtSymbol;
	}
    else if (ev->x > selected_monitor->window_area.size.w - (int)TEXTW(status_text) - getsystraywidth()){
      click = ClkStatusText;
	} else {
      click = ClkWinTitle;
	}
  } else if ((c = wintoclient(ev->window))) {
    focus(c);
    restack(selected_monitor);
    XAllowEvents(display, ReplayPointer, CurrentTime);
    click = ClkClientWin;
  }
  for (i = 0; i < LENGTH(buttons); i++){
    if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state)){
      buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
	  }
  }
}

void checkotherwm(void) {
  xerrorxlib = XSetErrorHandler(xerrorstart);
  /* this causes an error if some other window manager is running */
  XSelectInput(display, DefaultRootWindow(display), SubstructureRedirectMask);
  XSync(display, False);
  XSetErrorHandler(xerror);
  XSync(display, False);
}

void cleanup(void) {
  Arg arg = {.ui = ~0};
  Layout layout = {"", NULL};
  Monitor *monitor;
  size_t i;

  view(&arg);
  selected_monitor->layout[selected_monitor->selected_layout] = &layout;
  for (monitor = monitors; monitor; monitor = monitor->next){
    while (monitor->stack){
      unmanage(monitor->stack, 0);
	  }
  }
  XUngrabKey(display, AnyKey, AnyModifier, root);
  while (monitors){
    cleanupmon(monitors);
  }

  if (systray_enabled) {
    XUnmapWindow(display, systray->window);
    XDestroyWindow(display, systray->window);
    free(systray);
  }

  for (i = 0; i < CurLast; i++){
    drw_cur_free(drw, cursor[i]);
  }

  for (i = 0; i < LENGTH(colors); i++){
    free(scheme[i]);
  }

  free(scheme);
  XDestroyWindow(display, wmcheckwin);
  drw_free(drw);
  XSync(display, False);
  XSetInputFocus(display, PointerRoot, RevertToPointerRoot, CurrentTime);
  XDeleteProperty(display, root, netatom[NetActiveWindow]);
}

void clientmessage(XEvent *event) {
  XWindowAttributes window_attributes;
  XSetWindowAttributes set_window_attributes;
  XClientMessageEvent *client_msg = &event->xclient;
  Client *client = wintoclient(client_msg->window);
  Size size;

  if (systray_enabled && client_msg->window == systray->window && client_msg->message_type == netatom[NetSystemTrayOP]) {
    /* add systray icons */
    if (client_msg->data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {
      if (!(client = (Client *)calloc(1, sizeof(Client)))){
        die("fatal: could not malloc() %u bytes\n", sizeof(Client));
	    }
      if (!(client->window = client_msg->data.l[2])) {
        free(client);
        return;
      }
      client->monitor = selected_monitor;
      client->next = systray->icons;
      systray->icons = client;
      if (!XGetWindowAttributes(display, client->window, &window_attributes)) {
        /* use sane defaults */
        window_attributes.width = bar_height;
        window_attributes.height = bar_height;
        window_attributes.border_width = 0;
      }
      client->area.position.x = client->old_area.position.x = client->area.position.y = client->old_area.position.y = 0;
      client->area.size.w = client->old_area.size.w = window_attributes.width;
      client->area.size.h = client->old_area.size.h = window_attributes.height;
      client->old_border_width = window_attributes.border_width;
      client->border_width = 0;
      client->is_floating = True;
      /* reuse tags field as mapped status */
      client->tags = 1;
      updatesizehints(client);
      size.w = window_attributes.width;
      size.h = window_attributes.height;
      updatesystrayicongeom(client, &size);
      XAddToSaveSet(display, client->window);
      XSelectInput(display, client->window, StructureNotifyMask | PropertyChangeMask | ResizeRedirectMask);
      XReparentWindow(display, client->window, systray->window, 0, 0);
      /* use parents background color */
      set_window_attributes.background_pixel = scheme[SchemeNorm][ColBg].pixel;
      XChangeWindowAttributes(display, client->window, CWBackPixel, &set_window_attributes);
      sendevent(client->window, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0, systray->window, XEMBED_EMBEDDED_VERSION);
      /* FIXME not sure if I have to send these events, too */
      sendevent(client->window, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_FOCUS_IN, 0, systray->window, XEMBED_EMBEDDED_VERSION);
      sendevent(client->window, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, systray->window, XEMBED_EMBEDDED_VERSION);
      sendevent(client->window, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_MODALITY_ON, 0, systray->window, XEMBED_EMBEDDED_VERSION);
      XSync(display, False);
      resizebarwin(selected_monitor);
      updatesystray();
      setclientstate(client, NormalState);
    }
    return;
  }

  if (!client){
    return;
  }
  if (client_msg->message_type == netatom[NetWMState]) {
    if (client_msg->data.l[1] == netatom[NetWMFullscreen] || client_msg->data.l[2] == netatom[NetWMFullscreen]){
      setfullscreen(client, (client_msg->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
                        || (client_msg->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ &&
                            !client->is_fullscreen)));
	  }
  } else if (client_msg->message_type == netatom[NetActiveWindow]) {
    if (client != selected_monitor->selected_client && !client->is_urgent){
      seturgent(client, 1);
	  }
  }
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

void configurenotify(XEvent *e) {
  Monitor *m;
  Client *c;
  XConfigureEvent *ev = &e->xconfigure;
  int dirty;
  Area area;

  /* TODO: updategeom handling sucks, needs to be simplified */
  if (ev->window == root) {
    dirty = (screen_width != ev->width || screen_height != ev->height);
    screen_width = ev->width;
    screen_height = ev->height;
    if (updategeom() || dirty) {
      drw_resize(drw, screen_width, bar_height);
      updatebars();
      for (m = monitors; m; m = m->next) {
        for (c = m->clients; c; c = c->next){
          if (c->is_fullscreen){
            area.position.x = m->monitor_area.position.x;
            area.position.y = m->monitor_area.position.y;
            area.size.w = m->monitor_area.size.w;
            area.size.h = m->monitor_area.size.h;
            resizeclient(c, &area);
		      }
		    }

        resizebarwin(m);
      }

      focus(NULL);
      arrange(NULL);
    }
  }
}

void configurerequest(XEvent *e) {
  Client *c;
  Monitor *m;
  XConfigureRequestEvent *ev = &e->xconfigurerequest;
  XWindowChanges wc;

  if ((c = wintoclient(ev->window))) {
    if (ev->value_mask & CWBorderWidth){
      c->border_width = ev->border_width;
	  }
    else if (c->is_floating || !selected_monitor->layout[selected_monitor->selected_layout]->arrange_func) {
      m = c->monitor;
      if (ev->value_mask & CWX) {
        c->old_area.position.x = c->area.position.x;
        c->area.position.x = m->monitor_area.position.x + ev->x;
      }
      if (ev->value_mask & CWY) {
        c->old_area.position.y = c->area.position.y;
        c->area.position.y = m->monitor_area.position.y + ev->y;
      }
      if (ev->value_mask & CWWidth) {
        c->old_area.size.w = c->area.size.w;
        c->area.size.w = ev->width;
      }
      if (ev->value_mask & CWHeight) {
        c->old_area.size.h = c->area.size.h;
        c->area.size.h = ev->height;
      }
      if ((c->area.position.x + c->area.size.w) > m->monitor_area.position.x + m->monitor_area.size.w && c->is_floating){
        c->area.position.x = m->monitor_area.position.x + (m->monitor_area.size.w / 2 - WIDTH(c) / 2); /* center in x direction */
	  }
      if ((c->area.position.y + c->area.size.h) > m->monitor_area.position.y + m->monitor_area.size.h && c->is_floating){
        c->area.position.y = m->monitor_area.position.y + (m->monitor_area.size.h / 2 - HEIGHT(c) / 2); /* center in y direction */
	  }
      if ((ev->value_mask & (CWX | CWY)) && !(ev->value_mask & (CWWidth | CWHeight))){
        configure(c);
	  }
      if (ISVISIBLE(c)){
        XMoveResizeWindow(display, c->window, c->area.position.x, c->area.position.y, c->area.size.w, c->area.size.h);
	  }
    } else {
      configure(c);
	  }
  } else {
    wc.x = ev->x;
    wc.y = ev->y;
    wc.width = ev->width;
    wc.height = ev->height;
    wc.border_width = ev->border_width;
    wc.sibling = ev->above;
    wc.stack_mode = ev->detail;
    XConfigureWindow(display, ev->window, ev->value_mask, &wc);
  }
  XSync(display, False);
}

void destroynotify(XEvent *e) {
  Client *c;
  XDestroyWindowEvent *ev = &e->xdestroywindow;

  if ((c = wintoclient(ev->window))){
    unmanage(c, 1);
  }
  else if ((c = wintosystrayicon(ev->window))) {
    removesystrayicon(c);
    resizebarwin(selected_monitor);
    updatesystray();
  }
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

void enternotify(XEvent *e) {
  Client *c;
  Monitor *m;
  XCrossingEvent *ev = &e->xcrossing;

  if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root) {
    return;
  }

  c = wintoclient(ev->window);
  m = c ? c->monitor : wintomon(ev->window);
  if (m != selected_monitor) {
    unfocus(selected_monitor->selected_client, 1);
    selected_monitor = m;
  } else if (!c || c == selected_monitor->selected_client) {
    return;
  }
  focus(c);
}

void expose(XEvent *e) {
  Monitor *m;
  XExposeEvent *ev = &e->xexpose;

  if (ev->count == 0 && (m = wintomon(ev->window))) {
    drawbar(m);
    if (m == selected_monitor){
      updatesystray();
	  }
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

/* there are some broken focus acquiring clients needing extra handling */
void focusin(XEvent *e) {
  XFocusChangeEvent *ev = &e->xfocus;

  if (selected_monitor->selected_client && ev->window != selected_monitor->selected_client->window) {
    setfocus(selected_monitor->selected_client);
  }
}

void focusstack(const Arg *arg) {
  Client *c = NULL, *i;

  if (!selected_monitor->selected_client || (selected_monitor->selected_client->is_fullscreen && is_fullscreen_locked)) {
    return;
  }

  if (arg->i > 0) {
    for (c = selected_monitor->selected_client->next; c && !ISVISIBLE(c); c = c->next);
    if (!c){
      for (c = selected_monitor->clients; c && !ISVISIBLE(c); c = c->next);
    }
  } else {
    for (i = selected_monitor->clients; i != selected_monitor->selected_client; i = i->next){
      if (ISVISIBLE(i)){
        c = i;
      }
    }
    if (!c) {
      for (; i; i = i->next) {
        if (ISVISIBLE(i)) {
          c = i;
        }
      }
    }
  }
  if (c) {
    focus(c);
    restack(selected_monitor);
  }
}

Atom getatomprop(Client *c, Atom prop) {
  int di;
  unsigned long dl;
  unsigned char *p = NULL;
  Atom da, atom = None;

  /* FIXME getatomprop should return the number of items and a pointer to
   * the stored data instead of this workaround */
  Atom req = XA_ATOM;
  if (prop == xatom[XembedInfo]){
    req = xatom[XembedInfo];
  }

  if (XGetWindowProperty(display, c->window, prop, 0L, sizeof atom, False, req, &da, &di, &dl, &dl, &p) == Success && p) {
    atom = *(Atom *)p;
    if (da == xatom[XembedInfo] && dl == 2) {
      atom = ((Atom *)p)[1];
	  }

    XFree(p);
  }

  return atom;
}

int getrootptr(int *x, int *y) {
  int di;
  unsigned int dui;
  Window dummy;

  return XQueryPointer(display, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long getstate(Window window) {
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

int gettextprop(Window window, Atom atom, char *text, unsigned int size) {
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

void grabbuttons(Client *c, int focused) {
  updatenumlockmask();
  {
    unsigned int i, j;
    unsigned int modifiers[] = {0, LockMask, numlockmask, numlockmask | LockMask};
    XUngrabButton(display, AnyButton, AnyModifier, c->window);
    if (!focused){
      XGrabButton(display, AnyButton, AnyModifier, c->window, False, BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
	  }

    for (i = 0; i < LENGTH(buttons); i++){
      if (buttons[i].click == ClkClientWin){
        for (j = 0; j < LENGTH(modifiers); j++) {
          XGrabButton(display, buttons[i].button, buttons[i].mask | modifiers[j], c->window, False, BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
		    }
      }
	  }
  }
}

void grabkeys(void) {
  updatenumlockmask();
  {
    unsigned int i, j, k;
    unsigned int modifiers[] = {
      0, LockMask, numlockmask, numlockmask | LockMask
    };
    int start;
    int end;
    int skip;
    KeySym *syms;

    XUngrabKey(display, AnyKey, AnyModifier, root);
    XDisplayKeycodes(display, &start, &end);
    syms = XGetKeyboardMapping(display, start, end - start + 1, &skip);
    if (!syms) {
      return;
	  }

    for (k = start; k <= end; k++){
      for (i = 0; i < LENGTH(keys); i++){
        /* skip modifier codes, we do that ourselves */
        if (keys[i].keysym == syms[(k - start) * skip]){
          for (j = 0; j < LENGTH(modifiers); j++){
            XGrabKey(display, k, keys[i].mod | modifiers[j], root, True, GrabModeAsync, GrabModeAsync);
		      }
		    }
	    }
	  }

    XFree(syms);
  }
}

void incnmaster(const Arg *arg) {
  selected_monitor->master_count = MAX(selected_monitor->master_count + arg->i, 0);
  arrange(selected_monitor);
}

#ifdef XINERAMA
static int isuniquegeom(XineramaScreenInfo *unique, size_t n,
                        XineramaScreenInfo *info) {
  while (n--)
    if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org &&
        unique[n].width == info->width && unique[n].height == info->height)
      return 0;
  return 1;
}
#endif /* XINERAMA */

void keypress(XEvent *e) {
  unsigned int i;
  KeySym keysym;
  XKeyEvent *ev;

  ev = &e->xkey;
  keysym = XKeycodeToKeysym(display, (KeyCode)ev->keycode, 0);
  for (i = 0; i < LENGTH(keys); i++){
    if (keysym == keys[i].keysym && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state) && keys[i].func){
      keys[i].func(&(keys[i].arg));
	  }
  }
}

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

void manage(Window w, XWindowAttributes *wa) {
  Client *c, *t = NULL;
  Window trans = None;
  XWindowChanges wc;

  c = ecalloc(1, sizeof(Client));
  c->window = w;
  /* geometry */
  c->area.position.x = c->old_area.position.x = wa->x;
  c->area.position.y = c->old_area.position.y = wa->y;
  c->area.size.w = c->old_area.size.w = wa->width;
  c->area.size.h = c->old_area.size.h = wa->height;
  c->old_border_width = wa->border_width;

  updatetitle(c);
  if (XGetTransientForHint(display, w, &trans) && (t = wintoclient(trans))){
    c->monitor = t->monitor;
    c->tags = t->tags;
  } else {
    c->monitor = selected_monitor;
    apply_config_rules(c);
  }

  if (c->area.position.x + WIDTH(c) > c->monitor->window_area.position.x + c->monitor->window_area.size.w){
    c->area.position.x = c->monitor->window_area.position.x + c->monitor->window_area.size.w - WIDTH(c);
  }
  if (c->area.position.y + HEIGHT(c) > c->monitor->window_area.position.y + c->monitor->window_area.size.h){
    c->area.position.y = c->monitor->window_area.position.y + c->monitor->window_area.size.h - HEIGHT(c);
  }
  c->area.position.x = MAX(c->area.position.x, c->monitor->window_area.position.x);
  c->area.position.y = MAX(c->area.position.y, c->monitor->window_area.position.y);
  c->border_width = border_width;

  wc.border_width = c->border_width;
  XConfigureWindow(display, w, CWBorderWidth, &wc);
  XSetWindowBorder(display, w, scheme[SchemeNorm][ColBorder].pixel);
  configure(c); /* propagates border_width, if size doesn't change */
  updatewindowtype(c);
  updatesizehints(c);
  updatewmhints(c);

  // set windows to center (patch)
  c->area.position.x = c->monitor->monitor_area.position.x + (c->monitor->monitor_area.size.w - WIDTH(c)) / 2;
  c->area.position.y = c->monitor->monitor_area.position.y + (c->monitor->monitor_area.size.h - HEIGHT(c)) / 2;

  XSelectInput(display, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
  grabbuttons(c, 0);
  if (!c->is_floating) {
    c->is_floating = c->old_state = trans != None || c->is_fixed;
  }
  if (c->is_floating) {
    XRaiseWindow(display, c->window);
  }
  attach(c);
  attachstack(c);
  XChangeProperty(display, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->window), 1);
  XMoveResizeWindow(display, c->window, c->area.position.x + 2 * screen_width, c->area.position.y, c->area.size.w, c->area.size.h); /* some windows require this */
  setclientstate(c, NormalState);
  if (c->monitor == selected_monitor) {
    unfocus(selected_monitor->selected_client, 0);
  }
  c->monitor->selected_client = c;
  arrange(c->monitor);
  XMapWindow(display, c->window);
  focus(NULL);
}

void mappingnotify(XEvent *e) {
  XMappingEvent *ev = &e->xmapping;

  XRefreshKeyboardMapping(ev);
  if (ev->request == MappingKeyboard) {
    grabkeys();
  }
}

void maprequest(XEvent *e) {
  static XWindowAttributes wa;
  XMapRequestEvent *ev = &e->xmaprequest;

  Client *i;
  if ((i = wintosystrayicon(ev->window))) {
    sendevent(i->window, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, systray->window, XEMBED_EMBEDDED_VERSION);
    resizebarwin(selected_monitor);
    updatesystray();
  }

  if (!XGetWindowAttributes(display, ev->window, &wa) || wa.override_redirect) {
    return;
  }
  if (!wintoclient(ev->window)) {
    manage(ev->window, &wa);
  }
}

void motionnotify(XEvent *e) {
  static Monitor *mon = NULL;
  Monitor *m;
  XMotionEvent *ev = &e->xmotion;
  Area area;

  if (ev->window != root) {
    return;
  }

  area.position.x = ev->x_root;
  area.position.y = ev->y_root;
  area.size.w = 1;
  area.size.h = 1;

  if ((m = recttomon(&area)) != mon && mon) {
    unfocus(selected_monitor->selected_client, 1);
    selected_monitor = m;
    focus(NULL);
  }
  mon = m;
}

void movemouse(const Arg *arg) {
  // arg not used?

  int x, y, old_client_x, old_client_y;
  Client *client;
  Monitor *monitor;
  XEvent event;
  Time last_time = 0;
  Area area;

  if (!(client = selected_monitor->selected_client)) {
    return;
  }

  /* no support moving fullscreen windows by mouse */
  if (client->is_fullscreen){
    return;
  }

  restack(selected_monitor);
  old_client_x = client->area.position.x;
  old_client_y = client->area.position.y;

  // grab coursor for the new root window of screen
  if (XGrabPointer(display, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess) {
    return;
  }

  if (!getrootptr(&x, &y)) {
    return;
  }

  do {
    XMaskEvent(display, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &event);
    switch (event.type) {
    case ConfigureRequest:
    case Expose:
    case MapRequest:
      handler[event.type](&event);
      break;
    case MotionNotify:
      if ((event.xmotion.time - last_time) <= (1000 / 60)) {
        continue;
	    }
      last_time = event.xmotion.time;

      area.position.x = old_client_x + (event.xmotion.x - x);
      area.position.y = old_client_y + (event.xmotion.y - y);
      area.size.w = client->area.size.w;
      area.size.h = client->area.size.h;

      if (abs(selected_monitor->window_area.position.x - area.position.x) < snap) {
        area.position.x = selected_monitor->window_area.position.x;
	    }
      else if (abs((selected_monitor->window_area.position.x + selected_monitor->window_area.size.w) - (area.position.x + WIDTH(client))) < snap) {
        area.position.x = selected_monitor->window_area.position.x + selected_monitor->window_area.size.w - WIDTH(client);
	    }

      if (abs(selected_monitor->window_area.position.y - area.position.y) < snap) {
        area.position.y = selected_monitor->window_area.position.y;
	    }
      else if (abs((selected_monitor->window_area.position.y + selected_monitor->window_area.size.h) - (area.position.y + HEIGHT(client))) < snap) {
        area.position.y = selected_monitor->window_area.position.y + selected_monitor->window_area.size.h - HEIGHT(client);
	    }

      if (!client->is_floating && selected_monitor->layout[selected_monitor->selected_layout]->arrange_func && (abs(area.position.x - client->area.position.x) > snap || abs(area.position.x - client->area.position.y) > snap)) {
        togglefloating(NULL);
	    }

      if (!selected_monitor->layout[selected_monitor->selected_layout]->arrange_func || client->is_floating) {
        resize(client, &area, 1);
	    }

      break;
    }
  } while (event.type != ButtonRelease);

  XUngrabPointer(display, CurrentTime);

  area.position.x = client->area.position.x;
  area.position.y = client->area.position.y;
  area.size.w = client->area.size.w;
  area.size.h = client->area.size.h;

  if ((monitor = recttomon(&area)) != selected_monitor) {
    sendmon(client, monitor);
    selected_monitor = monitor;
    focus(NULL);
  }
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

void propertynotify(XEvent *event) {
  Client *client;
  Window trans;
  XPropertyEvent *event_prop = &event->xproperty;
  Size size;

  if ((client = wintosystrayicon(event_prop->window))) {
    if (event_prop->atom == XA_WM_NORMAL_HINTS) {
      updatesizehints(client);
      size.w = client->area.size.w;
      size.h = client->area.size.h;
      updatesystrayicongeom(client, &size);
    } else {
      updatesystrayiconstate(client, event_prop);
	}

    resizebarwin(selected_monitor);
    updatesystray();
  }

  if ((event_prop->window == root) && (event_prop->atom == XA_WM_NAME)) {
    updatestatus();
  }
  else if (event_prop->state == PropertyDelete) {
    return; /* ignore */
  } else if ((client = wintoclient(event_prop->window))) {
    switch (event_prop->atom) {
    default:
      break;
    case XA_WM_TRANSIENT_FOR:
      if (!client->is_floating && (XGetTransientForHint(display, client->window, &trans)) && (client->is_floating = (wintoclient(trans)) != NULL)) arrange(client->monitor);
      break;
    case XA_WM_NORMAL_HINTS:
      client->hintsvalid = 0;
      break;
    case XA_WM_HINTS:
      updatewmhints(client);
      drawbars();
      break;
    }
    if (event_prop->atom == XA_WM_NAME || event_prop->atom == netatom[NetWMName]) {
      updatetitle(client);
      if (client == client->monitor->selected_client) {
        drawbar(client->monitor);
	  }
    }
    if (event_prop->atom == netatom[NetWMWindowType]) {
      updatewindowtype(client);
	  }
  }
}

void quit(const Arg *arg) {
  if (arg->i) {
    restart = 1;
  }

  running = 0;
}

void resize(Client *c, Area *area, int interact) {
  if (applysizehints(c, area, interact)) {
    resizeclient(c, area);
  }
}

void resizeclient(Client *c, Area *area) {
  XWindowChanges window_changes;

  c->old_area.position.x = c->area.position.x;
  c->area.position.x = window_changes.x = area->position.x;

  c->old_area.position.y = c->area.position.y;
  c->area.position.y = window_changes.y = area->position.y;

  c->old_area.size.w = c->area.size.w;
  c->area.size.w = window_changes.width = area->size.w;

  c->old_area.size.h = c->area.size.h;
  c->area.size.h = window_changes.height = area->size.h;

  window_changes.border_width = c->border_width;
  XConfigureWindow(display, c->window, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &window_changes);
  configure(c);
  XSync(display, False);
}

void resizerequest(XEvent *event) {
  XResizeRequestEvent *request_event = &event->xresizerequest;
  Client *icon;
  Size size;

  size.w = request_event->width;
  size.h = request_event->height;

  if ((icon = wintosystrayicon(request_event->window))) {

    updatesystrayicongeom(icon, &size);
    resizebarwin(selected_monitor);
    updatesystray();
  }
}

void resizemouse(const Arg *arg) {
  int ocx, ocy, nw, nh;
  Client *client;
  Monitor *monitor;
  XEvent event;
  Time last_time = 0;
  Area area;

  area.position.x = 0;
  area.position.y = 0;
  area.size.w = 0;
  area.size.h = 0;

  if (!(client = selected_monitor->selected_client)) {
    return;
  }

  /* no support resizing fullscreen windows by mouse */
  if (client->is_fullscreen) {
    return;
  }

  restack(selected_monitor);
  ocx = client->area.position.x;
  ocy = client->area.position.y;
  if (XGrabPointer(display, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess) {
    return;
  }

  XWarpPointer(display, None, client->window, 0, 0, 0, 0, client->area.size.w + client->border_width - 1, client->area.size.h + client->border_width - 1);
  do {
    XMaskEvent(display, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &event);
    switch (event.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[event.type](&event);
			break;
		case MotionNotify:
			if ((event.xmotion.time - last_time) <= (1000 / 60)){
				continue;
			}
			last_time = event.xmotion.time;

			nw = MAX(event.xmotion.x - ocx - 2 * client->border_width + 1, 1);
			nh = MAX(event.xmotion.y - ocy - 2 * client->border_width + 1, 1);
      area.position.x = client->area.position.x;
      area.position.y = client->area.position.y;
			area.size.w = MAX(event.xmotion.x - ocx - 2 * client->border_width + 1, 1);
			area.size.h = MAX(event.xmotion.y - ocy - 2 * client->border_width + 1, 1);

			if (client->monitor->window_area.position.x + nw >= selected_monitor->window_area.position.x && client->monitor->window_area.position.x + nw <= selected_monitor->window_area.position.x + selected_monitor->window_area.size.w && client->monitor->window_area.position.y + nh >= selected_monitor->window_area.position.y && client->monitor->window_area.position.y + nh <= selected_monitor->window_area.position.y + selected_monitor->window_area.size.h) {
				if (!client->is_floating && selected_monitor->layout[selected_monitor->selected_layout]->arrange_func && (abs(nw - client->area.size.w) > snap || abs(nh - client->area.size.h) > snap)) {
				togglefloating(NULL);
				}
			}
			if (!selected_monitor->layout[selected_monitor->selected_layout]->arrange_func || client->is_floating) {
				resize(client, &area, 1);
			}

			break;
    }
  } while (event.type != ButtonRelease);
  XWarpPointer(display, None, client->window, 0, 0, 0, 0, client->area.size.w + client->border_width - 1, client->area.size.h + client->border_width - 1);
  XUngrabPointer(display, CurrentTime);
  while (XCheckMaskEvent(display, EnterWindowMask, &event));

  area.position.x = client->area.position.x;
  area.position.y = client->area.position.y;
  area.size.w = client->area.size.w;
  area.size.h = client->area.size.h;

  if ((monitor = recttomon(&area)) != selected_monitor) {
    sendmon(client, monitor);
    selected_monitor = monitor;
    focus(NULL);
  }
}

void restack(Monitor *monitor) {
  Client *client;
  XEvent event;
  XWindowChanges window_changes;

  drawbar(monitor);

  if (!monitor->selected_client){
    return;
  }

  // floating clients. if layout callback is NULL, float by default
  if (monitor->selected_client->is_floating || !monitor->layout[monitor->selected_layout]->arrange_func){
    XRaiseWindow(display, monitor->selected_client->window);
  }

  // re-stack clients
  if (monitor->layout[monitor->selected_layout]->arrange_func) {
    window_changes.stack_mode = Below;
    window_changes.sibling = monitor->bar_window;

    for (client = monitor->stack; client; client = client->next_stack) {
      if (!client->is_floating && ISVISIBLE(client)) {
        XConfigureWindow(display, client->window, CWSibling | CWStackMode, &window_changes);
        window_changes.sibling = client->window;
      }
	  }
  }

  XSync(display, False);
  while (XCheckMaskEvent(display, EnterWindowMask, &event));
}

void run(void) {
  XEvent ev;
  /* main event loop */
  XSync(display, False);
  while (running && !XNextEvent(display, &ev)) {
    if (handler[ev.type]) {
      handler[ev.type](&ev); /* call handler */
	  }
  }
}

void scan(void) {
  unsigned int i, num;
  Window d1, d2, *wins = NULL;
  XWindowAttributes wa;

  if (XQueryTree(display, root, &d1, &d2, &wins, &num)) {
    for (i = 0; i < num; i++) {
      if (!XGetWindowAttributes(display, wins[i], &wa) || wa.override_redirect || XGetTransientForHint(display, wins[i], &d1)) {
        continue;
	    }
      if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState) {
        manage(wins[i], &wa);
	    }
    }

    /* now the transients */
    for (i = 0; i < num; i++) {
      if (!XGetWindowAttributes(display, wins[i], &wa)) {
        continue;
	    }
      if (XGetTransientForHint(display, wins[i], &d1) && (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)) {
        manage(wins[i], &wa);
	    }
    }
    if (wins) {
      XFree(wins);
	}
  }
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

void setclientstate(Client *c, long state){
  long data[] = {state, None};

  XChangeProperty(display, c->window, wmatom[WMState], wmatom[WMState], 32, PropModeReplace, (unsigned char *)data, 2);
}

int sendevent(Window w, Atom proto, int mask, long d0, long d1, long d2, long d3, long d4) {
  int n;
  Atom *protocols, mt;
  int exists = 0;
  XEvent ev;

  if (proto == wmatom[WMTakeFocus] || proto == wmatom[WMDelete]) {
    mt = wmatom[WMProtocols];
    if (XGetWMProtocols(display, w, &protocols, &n)) {
      while (!exists && n--){
        exists = protocols[n] == proto;
	    }
      XFree(protocols);
    }
  } else {
    exists = True;
    mt = proto;
  }

  if (exists) {
    ev.type = ClientMessage;
    ev.xclient.window = w;
    ev.xclient.message_type = mt;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = d0;
    ev.xclient.data.l[1] = d1;
    ev.xclient.data.l[2] = d2;
    ev.xclient.data.l[3] = d3;
    ev.xclient.data.l[4] = d4;
    XSendEvent(display, w, False, mask, &ev);
  }
  return exists;
}

void setfocus(Client *client) {
  if (!client->never_focus) {
    XSetInputFocus(display, client->window, RevertToPointerRoot, CurrentTime);
    XChangeProperty(display, root, netatom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&(client->window), 1);
  }
  sendevent(client->window, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

void setfullscreen(Client *c, int fullscreen) {
  Area area;

  if (fullscreen && !c->is_fullscreen) {
    XChangeProperty(display, c->window, netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)&netatom[NetWMFullscreen], 1);
    c->is_fullscreen = 1;
    c->old_state = c->is_floating;
    c->old_border_width = c->border_width;
    c->border_width = 0;
    c->is_floating = 1;

    area.position.x = c->monitor->monitor_area.position.x;
    area.position.y = c->monitor->monitor_area.position.y;
    area.size.w = c->monitor->monitor_area.size.w;
    area.size.h = c->monitor->monitor_area.size.h;

    resizeclient(c, &area);
    XRaiseWindow(display, c->window);
  } else if (!fullscreen && c->is_fullscreen) {
    XChangeProperty(display, c->window, netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)0, 0);
    c->is_fullscreen = 0;
    c->is_floating = c->old_state;
    c->border_width = c->old_border_width;
    c->area.position.x = c->old_area.position.x;
    c->area.position.y = c->old_area.position.y;
    c->area.size.w = c->old_area.size.w;
    c->area.size.h = c->old_area.size.h;

    area.position.x = c->area.position.x;;
    area.position.y = c->area.position.y;;
    area.size.w = c->area.size.w;
    area.size.h = c->area.size.h;

    resizeclient(c, &area);
    arrange(c->monitor);
  }
}

void setgaps(const Arg *arg) {
  if ((arg->i == 0) || (selected_monitor->gap + arg->i < 0)) {
    selected_monitor->gap = 0;
  } else {
    selected_monitor->gap += arg->i;
  }

  arrange(selected_monitor);
}

void setlayout(const Arg *arg) {
  // has no or invalid arg, reset to default?
  if (!arg || !arg->v || arg->v != selected_monitor->layout[selected_monitor->selected_layout]) {
    selected_monitor->selected_layout ^= 1;
  }

  // has valid arg, set layout
  if (arg && arg->v) {
    selected_monitor->layout[selected_monitor->selected_layout] = (Layout *)arg->v;
  }

  // copy layout name to display
  strncpy(selected_monitor->layout_symbol, selected_monitor->layout[selected_monitor->selected_layout]->symbol, sizeof selected_monitor->layout_symbol);

  // this condition is not clear to me...
  if (selected_monitor->selected_client) {
    arrange(selected_monitor);
  } else {
    drawbar(selected_monitor);
  }
}

void setmfact(const Arg *arg) {
  /* arg > 1.0 will set mfact absolutely */

  float f;

  if (!arg || !selected_monitor->layout[selected_monitor->selected_layout]->arrange_func) {
    return;
  }

  f = arg->f < 1.0 ? arg->f + selected_monitor->master_factor : arg->f - 1.0;
  if (f < 0.05 || f > 0.95) {
    return;
  }

  selected_monitor->master_factor = f;
  arrange(selected_monitor);
}

void setup(void) {
  int i;
  XSetWindowAttributes window_attributes;
  Atom utf8string;
  struct sigaction signal_action;

  /* do not transform children into zombies when they terminate */
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
  signal_action.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &signal_action, NULL);

  /* clean up any zombies (inherited from .xinitrc etc) immediately */
  while (waitpid(-1, NULL, WNOHANG) > 0);

  signal(SIGHUP, sighup);
  signal(SIGTERM, sigterm);

  /* init screen */
  screen = DefaultScreen(display);
  screen_width = DisplayWidth(display, screen);
  screen_height = DisplayHeight(display, screen);
  root = RootWindow(display, screen);
  drw = drw_create(display, screen, root, screen_width, screen_height);

  // font stuff
  if (!drw_fontset_create(drw, fonts, LENGTH(fonts))) {
    die("no fonts could be loaded.");
  }

  padding = drw->fonts->h;
  bar_height = drw->fonts->h + 2;
  updategeom();

  /* init atoms */
  utf8string = XInternAtom(display, "UTF8_STRING", False);
  wmatom[WMProtocols] = XInternAtom(display, "WM_PROTOCOLS", False);
  wmatom[WMDelete] = XInternAtom(display, "WM_DELETE_WINDOW", False);
  wmatom[WMState] = XInternAtom(display, "WM_STATE", False);
  wmatom[WMTakeFocus] = XInternAtom(display, "WM_TAKE_FOCUS", False);

  netatom[NetActiveWindow] = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
  netatom[NetSupported] = XInternAtom(display, "_NET_SUPPORTED", False);
  netatom[NetSystemTray] = XInternAtom(display, "_NET_SYSTEM_TRAY_S0", False);
  netatom[NetSystemTrayOP] = XInternAtom(display, "_NET_SYSTEM_TRAY_OPCODE", False);
  netatom[NetSystemTrayOrientation] = XInternAtom(display, "_NET_SYSTEM_TRAY_ORIENTATION", False);
  netatom[NetSystemTrayOrientationHorz] = XInternAtom(display, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
  netatom[NetWMName] = XInternAtom(display, "_NET_WM_NAME", False);
  netatom[NetWMState] = XInternAtom(display, "_NET_WM_STATE", False);
  netatom[NetWMCheck] = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False);
  netatom[NetWMFullscreen] = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
  netatom[NetWMWindowType] = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
  netatom[NetWMWindowTypeDialog] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
  netatom[NetClientList] = XInternAtom(display, "_NET_CLIENT_LIST", False);

  xatom[Manager] = XInternAtom(display, "MANAGER", False);
  xatom[Xembed] = XInternAtom(display, "_XEMBED", False);
  xatom[XembedInfo] = XInternAtom(display, "_XEMBED_INFO", False);

  /* init cursors */
  cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
  cursor[CurResize] = drw_cur_create(drw, XC_sizing);
  cursor[CurMove] = drw_cur_create(drw, XC_fleur);

  /* init appearance */
  scheme = ecalloc(LENGTH(colors), sizeof(Clr *));
  for (i = 0; i < LENGTH(colors); i++){
    scheme[i] = drw_scm_create(drw, colors[i], 3);
  }

  /* init system tray */
  updatesystray();

  /* init bars */
  updatebars();
  updatestatus();

  /* supporting window for NetWMCheck */
  wmcheckwin = XCreateSimpleWindow(display, root, 0, 0, 1, 1, 0, 0, 0);
  XChangeProperty(display, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wmcheckwin, 1);
  XChangeProperty(display, wmcheckwin, netatom[NetWMName], utf8string, 8, PropModeReplace, (unsigned char *)"dwm", 3);
  XChangeProperty(display, root, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wmcheckwin, 1);

  /* EWMH support per view */
  XChangeProperty(display, root, netatom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *)netatom, NetLast);
  XDeleteProperty(display, root, netatom[NetClientList]);

  /* select events */
  window_attributes.cursor = cursor[CurNormal]->cursor;
  window_attributes.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
  XChangeWindowAttributes(display, root, CWEventMask | CWCursor, &window_attributes);
  XSelectInput(display, root, window_attributes.event_mask);
  grabkeys();
  focus(NULL);
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

void sighup(int unused) {
  Arg a = {.i = 1};
  quit(&a);
}

void sigterm(int unused) {
  Arg a = {.i = 0};
  quit(&a);
}

void spawn(const Arg *arg) {
  // check command
  if (arg->v == dmenucmd) {
    dmenumon[0] = '0' + selected_monitor->num;
  }

  // create new process
  if (fork() == 0) {
    if (display) {
      close(ConnectionNumber(display));
	  }
    setsid();
    execvp(((char **)arg->v)[0], (char **)arg->v);
    die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
  }
}

void tag(const Arg *arg) {
  if (selected_monitor->selected_client && arg->ui & TAGMASK) {
    selected_monitor->selected_client->tags = arg->ui & TAGMASK;
    focus(NULL);
    arrange(selected_monitor);
  }
}

void tagmon(const Arg *arg) {
  if (!selected_monitor->selected_client || !monitors->next){
    return;
  }

  sendmon(selected_monitor->selected_client, dirtomon(arg->i));
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

void toggletag(const Arg *arg) {
  unsigned int newtags;

  if (!selected_monitor->selected_client) {
    return;
  }

  newtags = selected_monitor->selected_client->tags ^ (arg->ui & TAGMASK);
  if (newtags) {
    selected_monitor->selected_client->tags = newtags;
    focus(NULL);
    arrange(selected_monitor);
  }
}

void toggleview(const Arg *arg) {
  unsigned int newtagset = selected_monitor->tag_set[selected_monitor->selected_tags] ^ (arg->ui & TAGMASK);

  if (newtagset) {
    selected_monitor->tag_set[selected_monitor->selected_tags] = newtagset;
    focus(NULL);
    arrange(selected_monitor);
  }
}

void unfocus(Client *client, int setfocus) {
  if (!client) {
    return;
  }

  grabbuttons(client, 0);
  XSetWindowBorder(display, client->window, scheme[SchemeNorm][ColBorder].pixel);
  if (setfocus) {
    XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(display, root, netatom[NetActiveWindow]);
  }
}

void unmanage(Client *client, int destroyed) {
  Monitor *monitor = client->monitor;
  XWindowChanges window_changes;

  detach(client);
  detachstack(client);
  if (!destroyed) {
    window_changes.border_width = client->old_border_width;
    XGrabServer(display); /* avoid race conditions */
    XSetErrorHandler(xerrordummy);
    XSelectInput(display, client->window, NoEventMask);
    XConfigureWindow(display, client->window, CWBorderWidth, &window_changes); /* restore border */
    XUngrabButton(display, AnyButton, AnyModifier, client->window);
    setclientstate(client, WithdrawnState);
    XSync(display, False);
    XSetErrorHandler(xerror);
    XUngrabServer(display);
  }
  free(client);
  focus(NULL);
  updateclientlist();
  arrange(monitor);
}

void unmapnotify(XEvent *e) {
  Client *c;
  XUnmapEvent *ev = &e->xunmap;

  if ((c = wintoclient(ev->window))) {
    if (ev->send_event){
      setclientstate(c, WithdrawnState);
    } else {
        unmanage(c, 0);
    }
  } else if ((c = wintosystrayicon(ev->window))) {
    /* KLUDGE! sometimes icons occasionally unmap their windows, but do
     * _not_ destroy them. We map those windows back */
    XMapRaised(display, c->window);
    updatesystray();
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

int updategeom(void) {
  int dirty = 0;

#ifdef XINERAMA
  if (XineramaIsActive(display)) {
    int i, j, n, nn;
    Client *c;
    Monitor *m;
    XineramaScreenInfo *info = XineramaQueryScreens(display, &nn);
    XineramaScreenInfo *unique = NULL;

    for (n = 0, m = monitors; m; m = m->next, n++)
      ;
    /* only consider unique geometries as separate screens */
    unique = ecalloc(nn, sizeof(XineramaScreenInfo));
    for (i = 0, j = 0; i < nn; i++) {
      if (isuniquegeom(unique, j, &info[i])){
        memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
	  }
	}
    XFree(info);
    nn = j;

    /* new monitors if nn > n */
    for (i = n; i < nn; i++) {
      for (m = monitors; m && m->next; m = m->next)
        ;
      if (m) {
        m->next = createmon();
	  } else {
        monitors = createmon();
	  }
    }
    for (i = 0, m = monitors; i < nn && m; m = m->next, i++){
      if (i >= n || unique[i].x_org != m->monitor_area.position.x || unique[i].y_org != m->monitor_area.position.y || unique[i].width != m->monitor_area.size.w || unique[i].height != m->monitor_area.size.h) {
        dirty = 1;
        m->num = i;
        m->monitor_area.position.x = m->window_area.position.x = unique[i].x_org;
        m->monitor_area.position.y = m->window_area.position.y = unique[i].y_org;
        m->monitor_area.size.w = m->window_area.size.w = unique[i].width;
        m->monitor_area.size.h = m->window_area.size.h = unique[i].height;
        update_bar_position(m);
      }
	}

    /* removed monitors if n > nn */
    for (i = nn; i < n; i++) {
      for (m = monitors; m && m->next; m = m->next)
        ;
      while ((c = m->clients)) {
        dirty = 1;
        m->clients = c->next;
        detachstack(c);
        c->monitor = monitors;
        attach(c);
        attachstack(c);
      }
      if (m == selected_monitor) {
        selected_monitor = monitors;
	  }
      cleanupmon(m);
    }
    free(unique);
  } else
#endif /* XINERAMA */
  {    /* default monitor setup */
    if (!monitors) {
      monitors = createmon();
	}
    if (monitors->monitor_area.size.w != screen_width || monitors->monitor_area.size.h != screen_height) {
      dirty = 1;
      monitors->monitor_area.size.w = monitors->window_area.size.w = screen_width;
      monitors->monitor_area.size.h = monitors->window_area.size.h = screen_height;
      update_bar_position(monitors);
    }
  }
  if (dirty) {
    selected_monitor = monitors;
    selected_monitor = wintomon(root);
  }

  return dirty;
}

void updatenumlockmask(void) {
  unsigned int i, j;
  XModifierKeymap *modmap;

  numlockmask = 0;
  modmap = XGetModifierMapping(display);

  for (i = 0; i < 8; i++) {
    for (j = 0; j < modmap->max_keypermod; j++) {
      if (modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(display, XK_Num_Lock)) {
        numlockmask = (1 << i);
	    }
	  }
  }

  XFreeModifiermap(modmap);
}

void updatesizehints(Client *c) {
  long master_size;
  XSizeHints size;

  if (!XGetWMNormalHints(display, c->window, &size, &master_size)) {
    /* size is uninitialized, ensure that size.flags aren't used */
    size.flags = PSize;
  }
  if (size.flags & PBaseSize) {
    c->base.w = size.base_width;
    c->base.h = size.base_height;
  } else if (size.flags & PMinSize) {
    c->base.w = size.min_width;
    c->base.h = size.min_height;
  } else {
    c->base.w = c->base.h = 0;
  }
  if (size.flags & PResizeInc) {
    c->inc.w = size.width_inc;
    c->inc.h = size.height_inc;
  } else {
    c->inc.w = c->inc.h = 0;
  }
  if (size.flags & PMaxSize) {
    c->max.w = size.max_width;
    c->max.h = size.max_height;
  } else {
    c->max.w = c->max.h = 0;
  }
  if (size.flags & PMinSize) {
    c->min.w = size.min_width;
    c->min.h = size.min_height;
  } else if (size.flags & PBaseSize) {
    c->min.w = size.base_width;
    c->min.h = size.base_height;
  } else {
    c->min.w = c->min.h = 0;
  }
  if (size.flags & PAspect) {
    c->aspect.min = (float)size.min_aspect.y / size.min_aspect.x;
    c->aspect.max = (float)size.max_aspect.x / size.max_aspect.y;
  } else {
    c->aspect.max = c->aspect.min = 0.0;
  }

  c->is_fixed = (c->max.w && c->max.h && c->max.w == c->min.w && c->max.h == c->min.h);
  c->hintsvalid = 1;
}

void updatetitle(Client *client) {
  if (!gettextprop(client->window, netatom[NetWMName], client->name, sizeof client->name)) {
    gettextprop(client->window, XA_WM_NAME, client->name, sizeof client->name);
  }

  /* hack to mark broken clients */
  if (client->name[0] == '\0') {
    strcpy(client->name, broken);
  }
}

void updatewindowtype(Client *client) {
  Atom state = getatomprop(client, netatom[NetWMState]);
  Atom type = getatomprop(client, netatom[NetWMWindowType]);

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

void view(const Arg *arg) {
  if ((arg->ui & TAGMASK) == selected_monitor->tag_set[selected_monitor->selected_tags]) {
    return;
  }

  selected_monitor->selected_tags ^= 1; /* toggle sel tagset */
  if (arg->ui & TAGMASK) {
    selected_monitor->tag_set[selected_monitor->selected_tags] = arg->ui & TAGMASK;
  }

  focus(NULL);
  arrange(selected_monitor);
}

Client *wintoclient(Window window) {
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

Client *wintosystrayicon(Window window) {
  Client *icons = NULL;

  if (!systray_enabled || !window) {
    return icons;
  }

  for (icons = systray->icons; icons && icons->window != window; icons = icons->next);

  return icons;
}

Monitor *wintomon(Window window) {
  int x, y;
  Client *client;
  Monitor *monitor;
  Area area;

  area.position.x = 0;
  area.position.y = 0;
  area.size.w = 1;
  area.size.h = 1;

  if (window == root && getrootptr(&x, &y)) {
    return recttomon(&area);
  }

  for (monitor = monitors; monitor; monitor = monitor->next) {
    if (window == monitor->bar_window){
      return monitor;
	  }
  }

  if ((client = wintoclient(window))) {
    return client->monitor;
  }

  return selected_monitor;
}

void zoom(const Arg *arg) {
  Client *c = selected_monitor->selected_client;

  if (!selected_monitor->layout[selected_monitor->selected_layout]->arrange_func || !c || c->is_floating) {
    return;
  }

  if (c == nexttiled(selected_monitor->clients) && !(c = nexttiled(c->next))) {
    return;
  }

  pop(c);
}

void parse_args(int argc, char *argv[]){
  if (argc == 2 && !strcmp("-v", argv[1])) {
    die("pdwm-" VERSION);
  }
  if (argc != 1) {
    die("usage: dwm [-v]");
  }
}

int main(int argc, char *argv[]) {
  parse_args(argc, argv);

  // locale
  if (!setlocale(LC_CTYPE, "") || !XSupportsLocale()) {
    fputs("warning: no locale support\n", stderr);
  }

  // open display
  if (!(display = XOpenDisplay(NULL))) {
    die("dwm: cannot open display");
  }

  checkotherwm(); // check if another wm is running
  setup();        // init systray, bars, screens, etc.
  scan();
  run(); 			    // event loop

  // relaunch dwm
  if (restart) {
    execvp(argv[0], argv);
  }

  // exit
  cleanup();
  XCloseDisplay(display);

  return EXIT_SUCCESS;
}
