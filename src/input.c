#include "input.h"

int get_root_ptr(int *x, int *y) {
  int di;
  unsigned int dui;
  Window dummy;

  return XQueryPointer(display, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

void move_mouse(const Arg *arg) {
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

  if (!get_root_ptr(&x, &y)) {
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

void resize_mouse(const Arg *arg) {
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

void grab_buttons(Client *client, int focused) {
  update_numlock_mask();
  {
    unsigned int i, j;
    unsigned int modifiers[] = {0, LockMask, numlockmask, numlockmask | LockMask};
    XUngrabButton(display, AnyButton, AnyModifier, client->window);
    if (!focused){
      XGrabButton(display, AnyButton, AnyModifier, client->window, False, BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
	  }

    for (i = 0; i < button_count; i++){
      if (buttons[i].click == ClkClientWin){
        for (j = 0; j < LENGTH(modifiers); j++) {
          XGrabButton(display, buttons[i].button, buttons[i].mask | modifiers[j], client->window, False, BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
		    }
      }
	  }
  }
}

void grab_keys(void) {
  update_numlock_mask();
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
      for (i = 0; i < key_count; i++){
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

void update_numlock_mask(void) {
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