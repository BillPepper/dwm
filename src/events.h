#ifndef EVENTS_H
#define EVENTS_H

#include <X11/Xlib.h>


void buttonpress(XEvent *event);
void clientmessage(XEvent *event);
void configurenotify(XEvent *event);
void configurerequest(XEvent *event);
void destroynotify(XEvent *event);
void enternotify(XEvent *event);
void expose(XEvent *event);
void focusin(XEvent *event);
void keypress(XEvent *event);
void mappingnotify(XEvent *event);
void maprequest(XEvent *event);
void motionnotify(XEvent *event);
void propertynotify(XEvent *event);
void resizerequest(XEvent *event);
void unmapnotify(XEvent *event);
int sendevent(Window window, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);

// Array of function pointers
#define EVENT_COUNT LASTEvent
void (*handler[EVENT_COUNT])(XEvent *) = {
  // handler[index] = callback
  [ButtonPress] = buttonpress,
  [ClientMessage] = clientmessage,
  [ConfigureRequest] = configurerequest,
  [ConfigureNotify] = configurenotify,
  [DestroyNotify] = destroynotify,
  [EnterNotify] = enternotify,
  [Expose] = expose,
  [FocusIn] = focusin,
  [KeyPress] = keypress,
  [MappingNotify] = mappingnotify,
  [MapRequest] = maprequest,
  [MotionNotify] = motionnotify,
  [PropertyNotify] = propertynotify,
  [ResizeRequest] = resizerequest,
  [UnmapNotify] = unmapnotify
};


#endif