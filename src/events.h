#ifndef EVENTS_H
#define EVENTS_H

#include "window.h"

extern int button_count;
extern int key_count;

#define EVENT_COUNT LASTEvent
extern void (*handler[EVENT_COUNT])(XEvent *);

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

#endif