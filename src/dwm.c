#include "dwm.h"

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags {
  char limitexceeded[LENGTH(tags) > 31 ? -1 : 1];
};

#ifdef XINERAMA
static int isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info) {
  while (n--)
    if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org &&
        unique[n].width == info->width && unique[n].height == info->height)
      return 0;
  return 1;
}
#endif /* XINERAMA */

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


// -- Events -------------------------------------------------------------------

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
/* there are some broken focus acquiring clients needing extra handling */
void focusin(XEvent *e) {
  XFocusChangeEvent *ev = &e->xfocus;

  if (selected_monitor->selected_client && ev->window != selected_monitor->selected_client->window) {
    setfocus(selected_monitor->selected_client);
  }
}

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


// -- Main ---------------------------------------------------------------------

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
  scan();         // scan for windows and mangage() them
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
