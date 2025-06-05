#include "error.h"



int x_error(Display *display, XErrorEvent *error_event) {
  /* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */

  if (error_event->error_code == BadWindow ||
      (error_event->request_code == X_SetInputFocus && error_event->error_code == BadMatch) ||
      (error_event->request_code == X_PolyText8 && error_event->error_code == BadDrawable) ||
      (error_event->request_code == X_PolyFillRectangle &&
       error_event->error_code == BadDrawable) ||
      (error_event->request_code == X_PolySegment && error_event->error_code == BadDrawable) ||
      (error_event->request_code == X_ConfigureWindow && error_event->error_code == BadMatch) ||
      (error_event->request_code == X_GrabButton && error_event->error_code == BadAccess) ||
      (error_event->request_code == X_GrabKey && error_event->error_code == BadAccess) ||
      (error_event->request_code == X_CopyArea && error_event->error_code == BadDrawable)){
  		  return 0;
	  }

  fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n", error_event->request_code, error_event->error_code);
  return xerrorxlib(display, error_event); /* may call exit */
}



int x_error_dummy(Display *dpy, XErrorEvent *ee) {
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int x_error_start(Display *dpy, XErrorEvent *ee) {
  die("dwm: another window manager is already running");
  return -1;
}
