#ifndef EVENTS_H
#define EVENTS_H

#include <X11/Xlib.h>

static void buttonpress(XEvent *event);
static void clientmessage(XEvent *event);
static void configurenotify(XEvent *event);
static void configurerequest(XEvent *event);
static void destroynotify(XEvent *event);
static void enternotify(XEvent *event);
static void expose(XEvent *event);
static void focusin(XEvent *event);
static void keypress(XEvent *event);
static void mappingnotify(XEvent *event);
static void maprequest(XEvent *event);
static void motionnotify(XEvent *event);
static void propertynotify(XEvent *event);
static void resizerequest(XEvent *event);
static void unmapnotify(XEvent *event);
static int sendevent(Window window, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);

#endif