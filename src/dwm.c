#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>


#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */

#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

#include "macro.h"

#include "definitions.h"

#include "dwm.h"
#include "globals.h"

/* configuration, allows nested code to access above variables */
#include "../config.h"

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags {
  char limitexceeded[LENGTH(tags) > 31 ? -1 : 1];
};

/* -- function implementations -- */

// apply the rules in config to client
void applyrules(Client *client) {
  const char *class;
  const char *instance;
  unsigned int i;
  const Rule *rule;
  Monitor *monitor;
  XClassHint class_hint = {NULL, NULL};

  /* rule matching */
  client->isfloating = 0;
  client->tags = 0;
  XGetClassHint(display, client->window, &class_hint);
  class = class_hint.res_class ? class_hint.res_class : broken;
  instance = class_hint.res_name ? class_hint.res_name : broken;

  for (i = 0; i < LENGTH(rules); i++) {
    rule = &rules[i];
    if ((!rule->title || strstr(client->name, rule->title)) && (!rule->class_name || strstr(class, rule->class_name)) && (!rule->instance || strstr(instance, rule->instance))) {
      client->isfloating = rule->isfloating;
      client->tags |= rule->tags;
      for (monitor = mons; monitor && monitor->num != rule->monitor; monitor = monitor->next);
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
  client->tags = client->tags & TAGMASK ? client->tags & TAGMASK : client->monitor->tagset[client->monitor->seltags];
}

// apply size hints to client
int applysizehints(Client *client, int *x, int *y, int *width, int *height, int interact) {
  int baseismin;
  Monitor *m = client->monitor;

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
    if (*x + *width + 2 * client->bw < 0){
      *x = 0;
	  }
    if (*y + *height + 2 * client->bw < 0){
      *y = 0;
	  }
  } else {
    if (*x >= m->wx + m->ww){
      *x = m->wx + m->ww - WIDTH(client);
	  }
    if (*y >= m->wy + m->wh){
      *y = m->wy + m->wh - HEIGHT(client);
	  }
    if (*x + *width + 2 * client->bw <= m->wx){
      *x = m->wx;
	  }
    if (*y + *height + 2 * client->bw <= m->wy){
      *y = m->wy;
	  }
  }
  if (*height < bar_height){
    *height = bar_height;
  }
  if (*width < bar_height){
    *width = bar_height;
  }
  if (resizehints || client->isfloating || !client->monitor->lt[client->monitor->sellt]->arrange) {
    if (!client->hintsvalid){
      updatesizehints(client);
	  }
    /* see last two sentences in ICCCM 4.1.2.3 */
    baseismin = client->basew == client->minw && client->baseh == client->minh;

    /* temporarily remove base dimensions */
    if (!baseismin) {
      *width -= client->basew;
      *height -= client->baseh;
    }
    /* adjust for aspect limits */
    if (client->mina > 0 && client->maxa > 0) {
      if (client->maxa < (float)*width / *height)
        *width = *height * client->maxa + 0.5;
      else if (client->mina < (float)*height / *width)
        *height = *width * client->mina + 0.5;
    }
    if (baseismin) { /* increment calculation requires this */
      *width -= client->basew;
      *height -= client->baseh;
    }
    /* adjust for increment value */
    if (client->incw){
      *width -= *width % client->incw;
	  }
    if (client->inch){
      *height -= *height % client->inch;
	  }
    /* restore base dimensions */
    *width = MAX(*width + client->basew, client->minw);
    *height = MAX(*height + client->baseh, client->minh);
    if (client->maxw){
      *width = MIN(*width, client->maxw);
	  }
    if (client->maxh){
      *height = MIN(*height, client->maxh);
	  }
  }
  return *x != client->x || *y != client->y || *width != client->w || *height != client->h;
}

void arrange(Monitor *monitor) {
  if (monitor){
    showhide(monitor->stack);
  } else {
    for (monitor = mons; monitor; monitor = monitor->next){
      showhide(monitor->stack);
	  }
  }


  if (monitor) {
    arrangemon(monitor);
    restack(monitor);
  } else {
    for (monitor = mons; monitor; monitor = monitor->next){
      arrangemon(monitor);
	  }
  }
}

void arrangemon(Monitor *monitor) {
  strncpy(monitor->ltsymbol, monitor->lt[monitor->sellt]->symbol, sizeof monitor->ltsymbol);
  if (monitor->lt[monitor->sellt]->arrange){
    monitor->lt[monitor->sellt]->arrange(monitor);
  }
}

void attach(Client *c) {
  c->next = c->monitor->clients;
  c->monitor->clients = c;
}

void attachstack(Client *c) {
  c->snext = c->monitor->stack;
  c->monitor->stack = c;
}

void buttonpress(XEvent *e) {
  unsigned int i, x, click;
  Arg arg = {0};
  Client *c;
  Monitor *m;
  XButtonPressedEvent *ev = &e->xbutton;

  click = ClkRootWin;
  /* focus monitor if necessary */
  if ((m = wintomon(ev->window)) && m != selmon) {
    unfocus(selmon->sel, 1);
    selmon = m;
    focus(NULL);
  }
  if (ev->window == selmon->barwin) {
    i = x = 0;
    do {
      x += TEXTW(tags[i]);
	}
    while (ev->x >= x && ++i < LENGTH(tags));

    if (i < LENGTH(tags)) {
      click = ClkTagBar;
      arg.ui = 1 << i;
    } else if (ev->x < x + TEXTW(selmon->ltsymbol)){
      click = ClkLtSymbol;
	}
    else if (ev->x > selmon->ww - (int)TEXTW(stext) - getsystraywidth()){
      click = ClkStatusText;
	} else {
      click = ClkWinTitle;
	}
  } else if ((c = wintoclient(ev->window))) {
    focus(c);
    restack(selmon);
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
  Arg a = {.ui = ~0};
  Layout foo = {"", NULL};
  Monitor *m;
  size_t i;

  view(&a);
  selmon->lt[selmon->sellt] = &foo;
  for (m = mons; m; m = m->next){
    while (m->stack){
      unmanage(m->stack, 0);
	  }
  }
  XUngrabKey(display, AnyKey, AnyModifier, root);
  while (mons){
    cleanupmon(mons);
  }

  if (showsystray) {
    XUnmapWindow(display, systray->win);
    XDestroyWindow(display, systray->win);
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

void cleanupmon(Monitor *mon) {
  Monitor *m;

  if (mon == mons){
    mons = mons->next;
  }
  else {
    for (m = mons; m && m->next != mon; m = m->next);
    m->next = mon->next;
  }
  XUnmapWindow(display, mon->barwin);
  XDestroyWindow(display, mon->barwin);
  free(mon);
}

void clientmessage(XEvent *e) {
  XWindowAttributes wa;
  XSetWindowAttributes swa;
  XClientMessageEvent *cme = &e->xclient;
  Client *c = wintoclient(cme->window);

  if (showsystray && cme->window == systray->win && cme->message_type == netatom[NetSystemTrayOP]) {
    /* add systray icons */
    if (cme->data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {
      if (!(c = (Client *)calloc(1, sizeof(Client)))){
        die("fatal: could not malloc() %u bytes\n", sizeof(Client));
	    }
      if (!(c->window = cme->data.l[2])) {
        free(c);
        return;
      }
      c->monitor = selmon;
      c->next = systray->icons;
      systray->icons = c;
      if (!XGetWindowAttributes(display, c->window, &wa)) {
        /* use sane defaults */
        wa.width = bar_height;
        wa.height = bar_height;
        wa.border_width = 0;
      }
      c->x = c->oldx = c->y = c->oldy = 0;
      c->w = c->oldw = wa.width;
      c->h = c->oldh = wa.height;
      c->oldbw = wa.border_width;
      c->bw = 0;
      c->isfloating = True;
      /* reuse tags field as mapped status */
      c->tags = 1;
      updatesizehints(c);
      updatesystrayicongeom(c, wa.width, wa.height);
      XAddToSaveSet(display, c->window);
      XSelectInput(display, c->window, StructureNotifyMask | PropertyChangeMask | ResizeRedirectMask);
      XReparentWindow(display, c->window, systray->win, 0, 0);
      /* use parents background color */
      swa.background_pixel = scheme[SchemeNorm][ColBg].pixel;
      XChangeWindowAttributes(display, c->window, CWBackPixel, &swa);
      sendevent(c->window, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0, systray->win, XEMBED_EMBEDDED_VERSION);
      /* FIXME not sure if I have to send these events, too */
      sendevent(c->window, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_FOCUS_IN, 0, systray->win, XEMBED_EMBEDDED_VERSION);
      sendevent(c->window, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, systray->win, XEMBED_EMBEDDED_VERSION);
      sendevent(c->window, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_MODALITY_ON, 0, systray->win, XEMBED_EMBEDDED_VERSION);
      XSync(display, False);
      resizebarwin(selmon);
      updatesystray();
      setclientstate(c, NormalState);
    }
    return;
  }

  if (!c){
    return;
  }
  if (cme->message_type == netatom[NetWMState]) {
    if (cme->data.l[1] == netatom[NetWMFullscreen] || cme->data.l[2] == netatom[NetWMFullscreen]){
      setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
                        || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ &&
                            !c->isfullscreen)));
	  }
  } else if (cme->message_type == netatom[NetActiveWindow]) {
    if (c != selmon->sel && !c->isurgent){
      seturgent(c, 1);
	  }
  }
}

void configure(Client *c) {
  XConfigureEvent ce;

  ce.type = ConfigureNotify;
  ce.display = display;
  ce.event = c->window;
  ce.window = c->window;
  ce.x = c->x;
  ce.y = c->y;
  ce.width = c->w;
  ce.height = c->h;
  ce.border_width = c->bw;
  ce.above = None;
  ce.override_redirect = False;
  XSendEvent(display, c->window, False, StructureNotifyMask, (XEvent *)&ce);
}

void configurenotify(XEvent *e) {
  Monitor *m;
  Client *c;
  XConfigureEvent *ev = &e->xconfigure;
  int dirty;

  /* TODO: updategeom handling sucks, needs to be simplified */
  if (ev->window == root) {
    dirty = (screen_width != ev->width || screen_height != ev->height);
    screen_width = ev->width;
    screen_height = ev->height;
    if (updategeom() || dirty) {
      drw_resize(drw, screen_width, bar_height);
      updatebars();
      for (m = mons; m; m = m->next) {
        for (c = m->clients; c; c = c->next){
          if (c->isfullscreen){
            resizeclient(c, m->mx, m->my, m->mw, m->mh);
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
      c->bw = ev->border_width;
	  }
    else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
      m = c->monitor;
      if (ev->value_mask & CWX) {
        c->oldx = c->x;
        c->x = m->mx + ev->x;
      }
      if (ev->value_mask & CWY) {
        c->oldy = c->y;
        c->y = m->my + ev->y;
      }
      if (ev->value_mask & CWWidth) {
        c->oldw = c->w;
        c->w = ev->width;
      }
      if (ev->value_mask & CWHeight) {
        c->oldh = c->h;
        c->h = ev->height;
      }
      if ((c->x + c->w) > m->mx + m->mw && c->isfloating){
        c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
	  }
      if ((c->y + c->h) > m->my + m->mh && c->isfloating){
        c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
	  }
      if ((ev->value_mask & (CWX | CWY)) && !(ev->value_mask & (CWWidth | CWHeight))){
        configure(c);
	  }
      if (ISVISIBLE(c)){
        XMoveResizeWindow(display, c->window, c->x, c->y, c->w, c->h);
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

Monitor *createmon(void) {
  Monitor *monitor;

  monitor = ecalloc(1, sizeof(Monitor));
  monitor->tagset[0] = monitor->tagset[1] = 1;
  monitor->mfact = mfact;
  monitor->nmaster = nmaster;
  monitor->showbar = showbar;
  monitor->topbar = topbar;
  monitor->gappx = gappx;
  monitor->lt[0] = &layouts[0];
  monitor->lt[1] = &layouts[1 % LENGTH(layouts)];
  strncpy(monitor->ltsymbol, layouts[0].symbol, sizeof monitor->ltsymbol);

  return monitor;
}

void destroynotify(XEvent *e) {
  Client *c;
  XDestroyWindowEvent *ev = &e->xdestroywindow;

  if ((c = wintoclient(ev->window))){
    unmanage(c, 1);
  }
  else if ((c = wintosystrayicon(ev->window))) {
    removesystrayicon(c);
    resizebarwin(selmon);
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

  for (tc = &c->monitor->stack; *tc && *tc != c; tc = &(*tc)->snext);
  *tc = c->snext;

  if (c == c->monitor->sel) {
    for (t = c->monitor->stack; t && !ISVISIBLE(t); t = t->snext);
    c->monitor->sel = t;
  }
}

Monitor *dirtomon(int dir) {
  Monitor *monitor = NULL;

  if (dir > 0) {
    if (!(monitor = selmon->next)){
      monitor = mons;
	  }
  } else if (selmon == mons){
    for (monitor = mons; monitor->next; monitor = monitor->next);
  }
  else {
    for (monitor = mons; monitor->next != selmon; monitor = monitor->next);
  }

  return monitor;
}

void drawbar(Monitor *m) {
  int x, w, tw = 0, trayWidth = 0;
  int boxs = drw->fonts->h / 9;
  int boxw = drw->fonts->h / 6 + 2;
  unsigned int i, occ = 0, urg = 0;
  Client *c;

  if (!m->showbar){
    return;
  }

  if (showsystray && m == systraytomon(m) && !systrayonleft){
    trayWidth = getsystraywidth();
  }

  /* draw status first so it can be overdrawn by tags later */
  drw_setscheme(drw, scheme[SchemeNorm]);
  tw = TEXTW(stext) - lrpad / 2 + 2; /* 2px extra right padding */
  drw_text(drw, m->ww - tw - trayWidth, 0, tw, bar_height, lrpad / 2 - 2, stext, 0);

  resizebarwin(m);

  // mark urgent tags
  for (c = m->clients; c; c = c->next) {
    occ |= c->tags;
    if (c->isurgent){
      urg |= c->tags;
	  }
  }

  // render tags
  x = 0;
  for (i = 0; i < LENGTH(tags); i++) {
    w = TEXTW(tags[i]);

    // render the tag
    drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << i ? SchemeSel : SchemeNorm]);
    drw_text(drw, x, 0, w, bar_height, lrpad / 2, tags[i], urg & 1 << i);

    // invert tag if urgent
    if (occ & 1 << i) {
      drw_rect(drw, x + boxs, boxs, boxw, boxw, m == selmon && selmon->sel && selmon->sel->tags & 1 << i, urg & 1 << i);
	  }

    // set position for next tag
    x += w;
  }

  // render layout
  w = TEXTW(m->ltsymbol);
  drw_setscheme(drw, scheme[SchemeNorm]);
  x = drw_text(drw, x, 0, w, bar_height, lrpad / 2, m->ltsymbol, 0);

  // render title of last highlighted client
  if ((w = m->ww - tw - trayWidth - x) > bar_height) {
    if (m->sel) {
      drw_setscheme(drw, scheme[m == selmon ? SchemeSel : SchemeNorm]);
      drw_text(drw, x, 0, w, bar_height, lrpad / 2, m->sel->name, 0);

      // render the small indicator when client is floating
      if (m->sel->isfloating){
        drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
	    }
    } else {
      drw_setscheme(drw, scheme[SchemeNorm]);
      drw_rect(drw, x, 0, w, bar_height, 1, 1);
    }
  }

  // not sure what this does...
  drw_map(drw, m->barwin, 0, 0, m->ww - trayWidth, bar_height);
}

void drawbars(void) {
  Monitor *m;

  for (m = mons; m; m = m->next){
    drawbar(m);
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
  if (m != selmon) {
    unfocus(selmon->sel, 1);
    selmon = m;
  } else if (!c || c == selmon->sel) {
    return;
  }
  focus(c);
}

void expose(XEvent *e) {
  Monitor *m;
  XExposeEvent *ev = &e->xexpose;

  if (ev->count == 0 && (m = wintomon(ev->window))) {
    drawbar(m);
    if (m == selmon){
      updatesystray();
	  }
  }
}

void focus(Client *c) {
  if (!c || !ISVISIBLE(c)) {
    for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext);
  }
  if (selmon->sel && selmon->sel != c) {
    unfocus(selmon->sel, 0);
  }
  if (c) {
    if (c->monitor != selmon) {
      selmon = c->monitor;
	  }
    if (c->isurgent) {
      seturgent(c, 0);
	  }

    detachstack(c);
    attachstack(c);
    grabbuttons(c, 1);
    XSetWindowBorder(display, c->window, scheme[SchemeSel][ColBorder].pixel);
    setfocus(c);
  } else {
    XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(display, root, netatom[NetActiveWindow]);
  }
  selmon->sel = c;
  drawbars();
}

/* there are some broken focus acquiring clients needing extra handling */
void focusin(XEvent *e) {
  XFocusChangeEvent *ev = &e->xfocus;

  if (selmon->sel && ev->window != selmon->sel->window) {
    setfocus(selmon->sel);
  }
}

void focusmon(const Arg *arg) {
  Monitor *m;

  if (!mons->next) {
    return;
  }
  if ((m = dirtomon(arg->i)) == selmon) {
    return;
  }

  unfocus(selmon->sel, 0);
  selmon = m;
  focus(NULL);
}

void focusstack(const Arg *arg) {
  Client *c = NULL, *i;

  if (!selmon->sel || (selmon->sel->isfullscreen && lockfullscreen)) {
    return;
  }

  if (arg->i > 0) {
    for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
    if (!c){
      for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
    }
  } else {
    for (i = selmon->clients; i != selmon->sel; i = i->next){
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
    restack(selmon);
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

unsigned int getsystraywidth() {
  unsigned int w = 0;
  Client *i;

  if (showsystray) {
    for (i = systray->icons; i; w += i->w + systrayspacing, i = i->next);
  }

  return w ? w + systrayspacing : 1;
}

int getrootptr(int *x, int *y) {
  int di;
  unsigned int dui;
  Window dummy;

  return XQueryPointer(display, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long getstate(Window w) {
  int format;
  long result = -1;
  unsigned char *p = NULL;
  unsigned long n, extra;
  Atom real;

  if (XGetWindowProperty(display, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState], &real, &format, &n, &extra, (unsigned char **)&p) != Success) {
    return -1;
  }

  if (n != 0) {
    result = *p;
  }

  XFree(p);
  return result;
}

int gettextprop(Window w, Atom atom, char *text, unsigned int size) {
  char **list = NULL;
  int n;
  XTextProperty name;

  if (!text || size == 0) {
    return 0;
  }

  text[0] = '\0';
  if (!XGetTextProperty(display, w, &name, atom) || !name.nitems) {
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
    unsigned int modifiers[] = {0, LockMask, numlockmask, numlockmask | LockMask};
    int start, end, skip;
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
  selmon->nmaster = MAX(selmon->nmaster + arg->i, 0);
  arrange(selmon);
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
  if (!selmon->sel) {
    return;
  }

  if (!sendevent(selmon->sel->window, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0, 0, 0)) {
    XGrabServer(display);
    XSetErrorHandler(xerrordummy);
    XSetCloseDownMode(display, DestroyAll);
    XKillClient(display, selmon->sel->window);
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
  c->x = c->oldx = wa->x;
  c->y = c->oldy = wa->y;
  c->w = c->oldw = wa->width;
  c->h = c->oldh = wa->height;
  c->oldbw = wa->border_width;

  updatetitle(c);
  if (XGetTransientForHint(display, w, &trans) && (t = wintoclient(trans))){
    c->monitor = t->monitor;
    c->tags = t->tags;
  } else {
    c->monitor = selmon;
    applyrules(c);
  }

  if (c->x + WIDTH(c) > c->monitor->wx + c->monitor->ww){
    c->x = c->monitor->wx + c->monitor->ww - WIDTH(c);
  }
  if (c->y + HEIGHT(c) > c->monitor->wy + c->monitor->wh){
    c->y = c->monitor->wy + c->monitor->wh - HEIGHT(c);
  }
  c->x = MAX(c->x, c->monitor->wx);
  c->y = MAX(c->y, c->monitor->wy);
  c->bw = borderpx;

  wc.border_width = c->bw;
  XConfigureWindow(display, w, CWBorderWidth, &wc);
  XSetWindowBorder(display, w, scheme[SchemeNorm][ColBorder].pixel);
  configure(c); /* propagates border_width, if size doesn't change */
  updatewindowtype(c);
  updatesizehints(c);
  updatewmhints(c);

  // set windows to center (patch)
  c->x = c->monitor->mx + (c->monitor->mw - WIDTH(c)) / 2;
  c->y = c->monitor->my + (c->monitor->mh - HEIGHT(c)) / 2;

  XSelectInput(display, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
  grabbuttons(c, 0);
  if (!c->isfloating) {
    c->isfloating = c->oldstate = trans != None || c->isfixed;
  }
  if (c->isfloating) {
    XRaiseWindow(display, c->window);
  }
  attach(c);
  attachstack(c);
  XChangeProperty(display, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->window), 1);
  XMoveResizeWindow(display, c->window, c->x + 2 * screen_width, c->y, c->w, c->h); /* some windows require this */
  setclientstate(c, NormalState);
  if (c->monitor == selmon) {
    unfocus(selmon->sel, 0);
  }
  c->monitor->sel = c;
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
    sendevent(i->window, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, systray->win, XEMBED_EMBEDDED_VERSION);
    resizebarwin(selmon);
    updatesystray();
  }

  if (!XGetWindowAttributes(display, ev->window, &wa) || wa.override_redirect) {
    return;
  }
  if (!wintoclient(ev->window)) {
    manage(ev->window, &wa);
  }
}

void
monocle(Monitor *m)
{
	unsigned int n = 0;
	Client *c;

	for (c = m->clients; c; c = c->next)
		if (ISVISIBLE(c))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
}

void motionnotify(XEvent *e) {
  static Monitor *mon = NULL;
  Monitor *m;
  XMotionEvent *ev = &e->xmotion;

  if (ev->window != root) {
    return;
  }
  if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
    unfocus(selmon->sel, 1);
    selmon = m;
    focus(NULL);
  }
  mon = m;
}

void movemouse(const Arg *arg) {
  int x, y, ocx, ocy, nx, ny;
  Client *c;
  Monitor *m;
  XEvent ev;
  Time lasttime = 0;

  if (!(c = selmon->sel)) {
    return;
  }

  /* no support moving fullscreen windows by mouse */
  if (c->isfullscreen){
    return;
  }

  restack(selmon);
  ocx = c->x;
  ocy = c->y;
  if (XGrabPointer(display, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess) {
    return;
  }

  if (!getrootptr(&x, &y)) {
    return;
  }

  do {
    XMaskEvent(display, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
    switch (ev.type) {
    case ConfigureRequest:
    case Expose:
    case MapRequest:
      handler[ev.type](&ev);
      break;
    case MotionNotify:
      if ((ev.xmotion.time - lasttime) <= (1000 / 60)) {
        continue;
	    }
      lasttime = ev.xmotion.time;

      nx = ocx + (ev.xmotion.x - x);
      ny = ocy + (ev.xmotion.y - y);
      if (abs(selmon->wx - nx) < snap) {
        nx = selmon->wx;
	    }
      else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap) {
        nx = selmon->wx + selmon->ww - WIDTH(c);
	    }
      if (abs(selmon->wy - ny) < snap) {
        ny = selmon->wy;
	    }
      else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap) {
        ny = selmon->wy + selmon->wh - HEIGHT(c);
	    }
      if (!c->isfloating && selmon->lt[selmon->sellt]->arrange && (abs(nx - c->x) > snap || abs(ny - c->y) > snap)) {
        togglefloating(NULL);
	    }
      if (!selmon->lt[selmon->sellt]->arrange || c->isfloating) {
        resize(c, nx, ny, c->w, c->h, 1);
	    }

      break;
    }
  } while (ev.type != ButtonRelease);

  XUngrabPointer(display, CurrentTime);
  if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
    sendmon(c, m);
    selmon = m;
    focus(NULL);
  }
}

Client *nexttiled(Client *c) {
  for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
  return c;
}

void pop(Client *c) {
  detach(c);
  attach(c);
  focus(c);
  arrange(c->monitor);
}

void propertynotify(XEvent *e) {
  Client *c;
  Window trans;
  XPropertyEvent *ev = &e->xproperty;

  if ((c = wintosystrayicon(ev->window))) {
    if (ev->atom == XA_WM_NORMAL_HINTS) {
      updatesizehints(c);
      updatesystrayicongeom(c, c->w, c->h);
    } else {
      updatesystrayiconstate(c, ev);
	}

    resizebarwin(selmon);
    updatesystray();
  }

  if ((ev->window == root) && (ev->atom == XA_WM_NAME)) {
    updatestatus();
  }
  else if (ev->state == PropertyDelete) {
    return; /* ignore */
  } else if ((c = wintoclient(ev->window))) {
    switch (ev->atom) {
    default:
      break;
    case XA_WM_TRANSIENT_FOR:
      if (!c->isfloating && (XGetTransientForHint(display, c->window, &trans)) && (c->isfloating = (wintoclient(trans)) != NULL)) arrange(c->monitor);
      break;
    case XA_WM_NORMAL_HINTS:
      c->hintsvalid = 0;
      break;
    case XA_WM_HINTS:
      updatewmhints(c);
      drawbars();
      break;
    }
    if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
      updatetitle(c);
      if (c == c->monitor->sel) {
        drawbar(c->monitor);
	  }
    }
    if (ev->atom == netatom[NetWMWindowType]) {
      updatewindowtype(c);
	  }
  }
}

void quit(const Arg *arg) {
  if (arg->i) {
    restart = 1;
  }

  running = 0;
}

Monitor *recttomon(int x, int y, int w, int h) {
  Monitor *monitor, *r = selmon;
  int a, area = 0;

  for (monitor = mons; monitor; monitor = monitor->next) {
    if ((a = INTERSECT(x, y, w, h, monitor)) > area) {
      area = a;
      r = monitor;
    }
  }

  return r;
}

void removesystrayicon(Client *i) {
  Client **ii;

  if (!showsystray || !i) {
    return;
  }
  for (ii = &systray->icons; *ii && *ii != i; ii = &(*ii)->next);
  if (ii) {
    *ii = i->next;
  }
  free(i);
}

void resize(Client *c, int x, int y, int w, int h, int interact) {
  if (applysizehints(c, &x, &y, &w, &h, interact)) {
    resizeclient(c, x, y, w, h);
  }
}

void resizebarwin(Monitor *m) {
  unsigned int w = m->ww;

  if (showsystray && m == systraytomon(m) && !systrayonleft) {
    w -= getsystraywidth();
  }

  XMoveResizeWindow(display, m->barwin, m->wx, m->by, w, bar_height);
}

void resizeclient(Client *c, int x, int y, int w, int h) {
  XWindowChanges wc;

  c->oldx = c->x;
  c->x = wc.x = x;
  c->oldy = c->y;
  c->y = wc.y = y;
  c->oldw = c->w;
  c->w = wc.width = w;
  c->oldh = c->h;
  c->h = wc.height = h;
  wc.border_width = c->bw;
  XConfigureWindow(display, c->window, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
  configure(c);
  XSync(display, False);
}

void resizerequest(XEvent *e) {
  XResizeRequestEvent *ev = &e->xresizerequest;
  Client *i;

  if ((i = wintosystrayicon(ev->window))) {
    updatesystrayicongeom(i, ev->width, ev->height);
    resizebarwin(selmon);
    updatesystray();
  }
}

void resizemouse(const Arg *arg) {
  int ocx, ocy, nw, nh;
  Client *c;
  Monitor *m;
  XEvent ev;
  Time lasttime = 0;

  if (!(c = selmon->sel)) {
    return;
  }

  /* no support resizing fullscreen windows by mouse */
  if (c->isfullscreen) {
    return;
  }

  restack(selmon);
  ocx = c->x;
  ocy = c->y;
  if (XGrabPointer(display, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess) {
    return;
  }

  XWarpPointer(display, None, c->window, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
  do {
    XMaskEvent(display, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
    switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60)){
				continue;
			}
			lasttime = ev.xmotion.time;

			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
			if (c->monitor->wx + nw >= selmon->wx && c->monitor->wx + nw <= selmon->wx + selmon->ww && c->monitor->wy + nh >= selmon->wy && c->monitor->wy + nh <= selmon->wy + selmon->wh) {
				if (!c->isfloating && selmon->lt[selmon->sellt]->arrange && (abs(nw - c->w) > snap || abs(nh - c->h) > snap)) {
				togglefloating(NULL);
				}
			}
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating) {
				resize(c, c->x, c->y, nw, nh, 1);
			}

			break;
    }
  } while (ev.type != ButtonRelease);
  XWarpPointer(display, None, c->window, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
  XUngrabPointer(display, CurrentTime);
  while (XCheckMaskEvent(display, EnterWindowMask, &ev));
  if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
    sendmon(c, m);
    selmon = m;
    focus(NULL);
  }
}

void restack(Monitor *m) {
  Client *c;
  XEvent ev;
  XWindowChanges wc;

  drawbar(m);
  if (!m->sel){
    return;
  }

  if (m->sel->isfloating || !m->lt[m->sellt]->arrange){
    XRaiseWindow(display, m->sel->window);
  }
  if (m->lt[m->sellt]->arrange) {
    wc.stack_mode = Below;
    wc.sibling = m->barwin;
    for (c = m->stack; c; c = c->snext) {
      if (!c->isfloating && ISVISIBLE(c)) {
        XConfigureWindow(display, c->window, CWSibling | CWStackMode, &wc);
        wc.sibling = c->window;
      }
	  }
  }
  XSync(display, False);
  while (XCheckMaskEvent(display, EnterWindowMask, &ev));
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

void sendmon(Client *c, Monitor *m){
  if (c->monitor == m) {
    return;
  }

  unfocus(c, 1);
  detach(c);
  detachstack(c);
  c->monitor = m;
  c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
  attach(c);
  attachstack(c);
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

void setfocus(Client *c) {
  if (!c->neverfocus) {
    XSetInputFocus(display, c->window, RevertToPointerRoot, CurrentTime);
    XChangeProperty(display, root, netatom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&(c->window), 1);
  }
  sendevent(c->window, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

void setfullscreen(Client *c, int fullscreen) {
  if (fullscreen && !c->isfullscreen) {
    XChangeProperty(display, c->window, netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)&netatom[NetWMFullscreen], 1);
    c->isfullscreen = 1;
    c->oldstate = c->isfloating;
    c->oldbw = c->bw;
    c->bw = 0;
    c->isfloating = 1;
    resizeclient(c, c->monitor->mx, c->monitor->my, c->monitor->mw, c->monitor->mh);
    XRaiseWindow(display, c->window);
  } else if (!fullscreen && c->isfullscreen) {
    XChangeProperty(display, c->window, netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)0, 0);
    c->isfullscreen = 0;
    c->isfloating = c->oldstate;
    c->bw = c->oldbw;
    c->x = c->oldx;
    c->y = c->oldy;
    c->w = c->oldw;
    c->h = c->oldh;
    resizeclient(c, c->x, c->y, c->w, c->h);
    arrange(c->monitor);
  }
}

void setgaps(const Arg *arg) {
  if ((arg->i == 0) || (selmon->gappx + arg->i < 0)) {
    selmon->gappx = 0;
  } else {
    selmon->gappx += arg->i;
  }

  arrange(selmon);
}

void setlayout(const Arg *arg) {
  if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt]) {
    selmon->sellt ^= 1;
  }
  if (arg && arg->v) {
    selmon->lt[selmon->sellt] = (Layout *)arg->v;
  }

  strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
  if (selmon->sel) {
    arrange(selmon);
  } else {
    drawbar(selmon);
  }
}

/* arg > 1.0 will set mfact absolutely */
void setmfact(const Arg *arg) {
  float f;

  if (!arg || !selmon->lt[selmon->sellt]->arrange) {
    return;
  }

  f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
  if (f < 0.05 || f > 0.95) {
    return;
  }

  selmon->mfact = f;
  arrange(selmon);
}

void setup(void) {
  int i;
  XSetWindowAttributes wa;
  Atom utf8string;
  struct sigaction sa;

  /* do not transform children into zombies when they terminate */
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
  sa.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &sa, NULL);

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

  lrpad = drw->fonts->h;
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
  wa.cursor = cursor[CurNormal]->cursor;
  wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
  XChangeWindowAttributes(display, root, CWEventMask | CWCursor, &wa);
  XSelectInput(display, root, wa.event_mask);
  grabkeys();
  focus(NULL);
}

void seturgent(Client *c, int urg) {
  XWMHints *wmh;

  c->isurgent = urg;
  if (!(wmh = XGetWMHints(display, c->window))) {
    return;
  }

  wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
  XSetWMHints(display, c->window, wmh);
  XFree(wmh);
}

void showhide(Client *c) {
  if (!c){
    return;
  }

  if (ISVISIBLE(c)) {
    /* show clients top down */
    XMoveWindow(display, c->window, c->x, c->y);
    if ((!c->monitor->lt[c->monitor->sellt]->arrange || c->isfloating) && !c->isfullscreen) {
      resize(c, c->x, c->y, c->w, c->h, 0);
	  }
    showhide(c->snext);
  } else {
    /* hide clients bottom up */
    showhide(c->snext);
    XMoveWindow(display, c->window, WIDTH(c) * -2, c->y);
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
  if (arg->v == dmenucmd) {
    dmenumon[0] = '0' + selmon->num;
  }
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
  if (selmon->sel && arg->ui & TAGMASK) {
    selmon->sel->tags = arg->ui & TAGMASK;
    focus(NULL);
    arrange(selmon);
  }
}

void tagmon(const Arg *arg) {
  if (!selmon->sel || !mons->next){
    return;
  }

  sendmon(selmon->sel, dirtomon(arg->i));
}

void tile(Monitor *m) {
  unsigned int i, n, h, mw, my, ty;
  Client *c;

  for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);

  if (n == 0){
    return;
  }

  if (n > m->nmaster){
    mw = m->nmaster ? m->ww * m->mfact : 0;
  } else {
    mw = m->ww - m->gappx;
  }

  for (i = 0, my = ty = m->gappx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
    if (i < m->nmaster) {
      h = (m->wh - my) / (MIN(n, m->nmaster) - i) - m->gappx;
      resize(c, m->wx + m->gappx, m->wy + my, mw - (2 * c->bw) - m->gappx, h - (2 * c->bw), 0);
      if (my + HEIGHT(c) + m->gappx < m->wh){
        my += HEIGHT(c) + m->gappx;
	    }
    } else {
      h = (m->wh - ty) / (n - i) - m->gappx;
      resize(c, m->wx + mw + m->gappx, m->wy + ty, m->ww - mw - (2 * c->bw) - 2 * m->gappx, h - (2 * c->bw), 0);
      if (ty + HEIGHT(c) + m->gappx < m->wh){
        ty += HEIGHT(c) + m->gappx;
	    }
    }
  }
}

void togglebar(const Arg *arg) {
  selmon->showbar = !selmon->showbar;
  updatebarpos(selmon);
  resizebarwin(selmon);

  if (showsystray) {
    XWindowChanges wc;
    if (!selmon->showbar) {
      wc.y = -bar_height;
	  }
    else if (selmon->showbar) {
      wc.y = 0;
      if (!selmon->topbar){
        wc.y = selmon->mh - bar_height;
	    }
    }
    XConfigureWindow(display, systray->win, CWY, &wc);
  }
  arrange(selmon);
}

void togglefloating(const Arg *arg) {
  if (!selmon->sel) {
    return;
  }

  /* no support for fullscreen windows */
  if (selmon->sel->isfullscreen) {
    return;
  }

  selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
  if (selmon->sel->isfloating) {
    resize(selmon->sel, selmon->sel->x, selmon->sel->y, selmon->sel->w, selmon->sel->h, 0);
  }

  arrange(selmon);
}

void togglefullscreen(const Arg *arg) {
  if (selmon->sel) {
    setfullscreen(selmon->sel, !selmon->sel->isfullscreen);
  }
}

void toggletag(const Arg *arg) {
  unsigned int newtags;

  if (!selmon->sel) {
    return;
  }

  newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
  if (newtags) {
    selmon->sel->tags = newtags;
    focus(NULL);
    arrange(selmon);
  }
}

void toggleview(const Arg *arg) {
  unsigned int newtagset =
      selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);

  if (newtagset) {
    selmon->tagset[selmon->seltags] = newtagset;
    focus(NULL);
    arrange(selmon);
  }
}

void unfocus(Client *c, int setfocus) {
  if (!c) {
    return;
  }

  grabbuttons(c, 0);
  XSetWindowBorder(display, c->window, scheme[SchemeNorm][ColBorder].pixel);
  if (setfocus) {
    XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(display, root, netatom[NetActiveWindow]);
  }
}

void unmanage(Client *c, int destroyed) {
  Monitor *m = c->monitor;
  XWindowChanges wc;

  detach(c);
  detachstack(c);
  if (!destroyed) {
    wc.border_width = c->oldbw;
    XGrabServer(display); /* avoid race conditions */
    XSetErrorHandler(xerrordummy);
    XSelectInput(display, c->window, NoEventMask);
    XConfigureWindow(display, c->window, CWBorderWidth, &wc); /* restore border */
    XUngrabButton(display, AnyButton, AnyModifier, c->window);
    setclientstate(c, WithdrawnState);
    XSync(display, False);
    XSetErrorHandler(xerror);
    XUngrabServer(display);
  }
  free(c);
  focus(NULL);
  updateclientlist();
  arrange(m);
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

void updatebars(void) {
  unsigned int width;
  Monitor *monitor;
  XSetWindowAttributes window_attributes = {.override_redirect = True, .background_pixmap = ParentRelative, .event_mask = ButtonPressMask | ExposureMask};
  XClassHint class_hint = {"dwm", "dwm"};

  // loop monitors
  for (monitor = mons; monitor; monitor = monitor->next) {

    // check if monitor has bar
    if (monitor->barwin){
      continue;
	  }

    // calculate how long the bar is without the systray
    width = monitor->ww;
    if (showsystray && monitor == systraytomon(monitor)){
      width -= getsystraywidth();
	  }

    // create bar
    monitor->barwin = XCreateWindow(display, root, monitor->wx, monitor->by, width, bar_height, 0, DefaultDepth(display, screen), CopyFromParent, DefaultVisual(display, screen), CWOverrideRedirect | CWBackPixmap | CWEventMask, &window_attributes);

    // set cursor
    XDefineCursor(display, monitor->barwin, cursor[CurNormal]->cursor);

    // raise bar parts
    if (showsystray && monitor == systraytomon(monitor)){
      XMapRaised(display, systray->win);
	  }
    XMapRaised(display, monitor->barwin);

    XSetClassHint(display, monitor->barwin, &class_hint);
  }
}

void updatebarpos(Monitor *m) {
  m->wy = m->my;
  m->wh = m->mh;

  if (m->showbar) {
    m->wh -= bar_height;
    m->by = m->topbar ? m->wy : m->wy + m->wh;
    m->wy = m->topbar ? m->wy + bar_height : m->wy;
  } else {
    m->by = -bar_height;
  }
}

void updateclientlist() {
  Client *c;
  Monitor *m;

  XDeleteProperty(display, root, netatom[NetClientList]);
  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      XChangeProperty(display, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->window), 1);
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

    for (n = 0, m = mons; m; m = m->next, n++)
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
      for (m = mons; m && m->next; m = m->next)
        ;
      if (m) {
        m->next = createmon();
	  } else {
        mons = createmon();
	  }
    }
    for (i = 0, m = mons; i < nn && m; m = m->next, i++){
      if (i >= n || unique[i].x_org != m->mx || unique[i].y_org != m->my || unique[i].width != m->mw || unique[i].height != m->mh) {
        dirty = 1;
        m->num = i;
        m->mx = m->wx = unique[i].x_org;
        m->my = m->wy = unique[i].y_org;
        m->mw = m->ww = unique[i].width;
        m->mh = m->wh = unique[i].height;
        updatebarpos(m);
      }
	}

    /* removed monitors if n > nn */
    for (i = nn; i < n; i++) {
      for (m = mons; m && m->next; m = m->next)
        ;
      while ((c = m->clients)) {
        dirty = 1;
        m->clients = c->next;
        detachstack(c);
        c->monitor = mons;
        attach(c);
        attachstack(c);
      }
      if (m == selmon) {
        selmon = mons;
	  }
      cleanupmon(m);
    }
    free(unique);
  } else
#endif /* XINERAMA */
  {    /* default monitor setup */
    if (!mons) {
      mons = createmon();
	}
    if (mons->mw != screen_width || mons->mh != screen_height) {
      dirty = 1;
      mons->mw = mons->ww = screen_width;
      mons->mh = mons->wh = screen_height;
      updatebarpos(mons);
    }
  }
  if (dirty) {
    selmon = mons;
    selmon = wintomon(root);
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
  long msize;
  XSizeHints size;

  if (!XGetWMNormalHints(display, c->window, &size, &msize)) {
    /* size is uninitialized, ensure that size.flags aren't used */
    size.flags = PSize;
  }
  if (size.flags & PBaseSize) {
    c->basew = size.base_width;
    c->baseh = size.base_height;
  } else if (size.flags & PMinSize) {
    c->basew = size.min_width;
    c->baseh = size.min_height;
  } else {
    c->basew = c->baseh = 0;
  }
  if (size.flags & PResizeInc) {
    c->incw = size.width_inc;
    c->inch = size.height_inc;
  } else {
    c->incw = c->inch = 0;
  }
  if (size.flags & PMaxSize) {
    c->maxw = size.max_width;
    c->maxh = size.max_height;
  } else {
    c->maxw = c->maxh = 0;
  }
  if (size.flags & PMinSize) {
    c->minw = size.min_width;
    c->minh = size.min_height;
  } else if (size.flags & PBaseSize) {
    c->minw = size.base_width;
    c->minh = size.base_height;
  } else {
    c->minw = c->minh = 0;
  }
  if (size.flags & PAspect) {
    c->mina = (float)size.min_aspect.y / size.min_aspect.x;
    c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
  } else {
    c->maxa = c->mina = 0.0;
  }

  c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
  c->hintsvalid = 1;
}

void updatestatus(void) {
  // default status text
  if (!gettextprop(root, XA_WM_NAME, stext, sizeof(stext))) {
    strcpy(stext, "dwm-" VERSION);
  }

  // show status on all screens
  if (showstatuson) {
    Monitor *m;

    for (m = mons; m; m = m->next) {
      drawbar(m);
    }
  }

  // show bar on selected screen
  else {
    drawbar(selmon);
  }

  updatesystray();
}

void updatesystrayicongeom(Client *i, int w, int h) {
  if (i) {
    i->h = bar_height;
    if (w == h) {
      i->w = bar_height;
	  } else if (h == bar_height) {
      i->w = w;
	  } else {
      i->w = (int)((float)bar_height * ((float)w / (float)h));
	  }

    applysizehints(i, &(i->x), &(i->y), &(i->w), &(i->h), False);
    /* force icons into the systray dimensions if they don't want to */
    if (i->h > bar_height) {
      if (i->w == i->h) {
        i->w = bar_height;
	    } else {
        i->w = (int)((float)bar_height * ((float)i->w / (float)i->h));
	    }

      i->h = bar_height;
    }
  }
}

void updatesystrayiconstate(Client *i, XPropertyEvent *ev) {
  long flags;
  int code = 0;

  if (!showsystray || !i || ev->atom != xatom[XembedInfo] || !(flags = getatomprop(i, xatom[XembedInfo]))) {
    return;
  }

  if (flags & XEMBED_MAPPED && !i->tags) {
    i->tags = 1;
    code = XEMBED_WINDOW_ACTIVATE;
    XMapRaised(display, i->window);
    setclientstate(i, NormalState);
  } else if (!(flags & XEMBED_MAPPED) && i->tags) {
    i->tags = 0;
    code = XEMBED_WINDOW_DEACTIVATE;
    XUnmapWindow(display, i->window);
    setclientstate(i, WithdrawnState);
  } else {
    return;
  }

  sendevent(i->window, xatom[Xembed], StructureNotifyMask, CurrentTime, code, 0, systray->win, XEMBED_EMBEDDED_VERSION);
}

void updatesystray(void) {
  XSetWindowAttributes wa;
  XWindowChanges wc;
  Client *i;
  Monitor *m = systraytomon(NULL);
  unsigned int x = m->mx + m->mw;
  unsigned int sw = TEXTW(stext) - lrpad + systrayspacing;
  unsigned int w = 1;

  if (!showsystray) {
    return;
  }

  if (systrayonleft) {
    x -= sw + lrpad / 2;
  }

  // init systray
  if (!systray) {
    if (!(systray = (Systray *)calloc(1, sizeof(Systray)))) {
      die("fatal: could not malloc() %u bytes\n", sizeof(Systray));
	  }

    // create tray window
    systray->win = XCreateSimpleWindow(display, root, x, m->by, w, bar_height, 0, 0, scheme[SchemeSel][ColBg].pixel);
    wa.event_mask = ButtonPressMask | ExposureMask;
    wa.override_redirect = True;
    wa.background_pixel = scheme[SchemeNorm][ColBg].pixel;

    // add class to systray
    XClassHint ch = { .res_class="dwm_tray", .res_name="dwm_tray"};
    XSetClassHint(display, systray->win, &ch);

    XSelectInput(display, systray->win, SubstructureNotifyMask);
    XChangeProperty(display, systray->win, netatom[NetSystemTrayOrientation], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&netatom[NetSystemTrayOrientationHorz], 1);
    XChangeWindowAttributes(display, systray->win, CWEventMask | CWOverrideRedirect | CWBackPixel, &wa);
    XMapRaised(display, systray->win);
    XSetSelectionOwner(display, netatom[NetSystemTray], systray->win, CurrentTime);

    if (XGetSelectionOwner(display, netatom[NetSystemTray]) == systray->win) {
      sendevent(root, xatom[Manager], StructureNotifyMask, CurrentTime, netatom[NetSystemTray], systray->win, 0, 0);
      XSync(display, False);
    } else {
      fprintf(stderr, "dwm: unable to obtain system tray.\n");
      free(systray);
      systray = NULL;
      return;
    }
  }

  for (w = 0, i = systray->icons; i; i = i->next) {
    /* make sure the background color stays the same */
    wa.background_pixel = scheme[SchemeNorm][ColBg].pixel;

    XChangeWindowAttributes(display, i->window, CWBackPixel, &wa);
    XMapRaised(display, i->window);
    w += systrayspacing;
    i->x = w;
    XMoveResizeWindow(display, i->window, i->x, 0, i->w, i->h);
    w += i->w;
    if (i->monitor != m) {
      i->monitor = m;
	  }
  }

  w = w ? w + systrayspacing : 1;
  x -= w;
  XMoveResizeWindow(display, systray->win, x, m->by, w, bar_height);
  wc.x = x;
  wc.y = m->by;
  wc.width = w;
  wc.height = bar_height;
  wc.stack_mode = Above;
  wc.sibling = m->barwin;
  XConfigureWindow(display, systray->win, CWX | CWY | CWWidth | CWHeight | CWSibling | CWStackMode, &wc);
  XMapWindow(display, systray->win);
  XMapSubwindows(display, systray->win);

  /* redraw background */
  XSetForeground(display, drw->gc, scheme[SchemeNorm][ColBg].pixel);
  XFillRectangle(display, systray->win, drw->gc, 0, 0, w, bar_height);
  XSync(display, False);
}

void updatetitle(Client *c) {
  if (!gettextprop(c->window, netatom[NetWMName], c->name, sizeof c->name)) {
    gettextprop(c->window, XA_WM_NAME, c->name, sizeof c->name);
  }

  /* hack to mark broken clients */
  if (c->name[0] == '\0') {
    strcpy(c->name, broken);
  }
}

void updatewindowtype(Client *c) {
  Atom state = getatomprop(c, netatom[NetWMState]);
  Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

  if (state == netatom[NetWMFullscreen]) {
    setfullscreen(c, 1);
  }
  if (wtype == netatom[NetWMWindowTypeDialog]) {
    c->isfloating = 1;
  }
}

void updatewmhints(Client *c) {
  XWMHints *wmh;

  if ((wmh = XGetWMHints(display, c->window))) {
    if (c == selmon->sel && wmh->flags & XUrgencyHint){
      wmh->flags &= ~XUrgencyHint;
      XSetWMHints(display, c->window, wmh);
    } else {
      c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
	}

    if (wmh->flags & InputHint) {
      c->neverfocus = !wmh->input;
	}
    else {
      c->neverfocus = 0;
	}

    XFree(wmh);
  }
}

void view(const Arg *arg) {
  if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags]) {
    return;
  }

  selmon->seltags ^= 1; /* toggle sel tagset */
  if (arg->ui & TAGMASK) {
    selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
  }

  focus(NULL);
  arrange(selmon);
}

Client *wintoclient(Window w) {
  Client *c;
  Monitor *m;

  for (m = mons; m; m = m->next) {
    for (c = m->clients; c; c = c->next) {
      if (c->window == w){
        return c;
	    }
	  }
  }

  return NULL;
}

Client *wintosystrayicon(Window w) {
  Client *i = NULL;

  if (!showsystray || !w) {
    return i;
  }

  for (i = systray->icons; i && i->window != w; i = i->next);

  return i;
}

Monitor *wintomon(Window w) {
  int x, y;
  Client *c;
  Monitor *m;

  if (w == root && getrootptr(&x, &y)) {
    return recttomon(x, y, 1, 1);
  }

  for (m = mons; m; m = m->next) {
    if (w == m->barwin){
      return m;
	  }
  }

  if ((c = wintoclient(w))) {
    return c->monitor;
  }

  return selmon;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int xerror(Display *dpy, XErrorEvent *ee) {
  if (ee->error_code == BadWindow ||
      (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch) ||
      (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable) ||
      (ee->request_code == X_PolyFillRectangle &&
       ee->error_code == BadDrawable) ||
      (ee->request_code == X_PolySegment && ee->error_code == BadDrawable) ||
      (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch) ||
      (ee->request_code == X_GrabButton && ee->error_code == BadAccess) ||
      (ee->request_code == X_GrabKey && ee->error_code == BadAccess) ||
      (ee->request_code == X_CopyArea && ee->error_code == BadDrawable)){
  		  return 0;
	  }

  fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n", ee->request_code, ee->error_code);
  return xerrorxlib(dpy, ee); /* may call exit */
}

int xerrordummy(Display *dpy, XErrorEvent *ee) {
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int xerrorstart(Display *dpy, XErrorEvent *ee) {
  die("dwm: another window manager is already running");
  return -1;
}

Monitor *systraytomon(Monitor *m) {
  Monitor *t;
  int i, n;

  if (!systraypinning) {
    if (!m) {
      return selmon;
	  }

    return m == selmon ? m : NULL;
  }

  for (n = 1, t = mons; t && t->next; n++, t = t->next);
  for (i = 1, t = mons; t && t->next && i < systraypinning; i++, t = t->next);

  if (systraypinningfailfirst && n < systraypinning) {
    return mons;
  }

  return t;
}

void zoom(const Arg *arg) {
  Client *c = selmon->sel;

  if (!selmon->lt[selmon->sellt]->arrange || !c || c->isfloating) {
    return;
  }

  if (c == nexttiled(selmon->clients) && !(c = nexttiled(c->next))) {
    return;
  }

  pop(c);
}

void _debug(){
  printf("debug\n");
}

void parse_args(int argc, char *argv[]){
  if (argc == 2 && !strcmp("-v", argv[1])) {
    die("dwm-" VERSION);
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
