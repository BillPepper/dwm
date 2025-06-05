#ifndef EVENTS_H
#define EVENTS_H

#include "window.h"

extern int button_count;
extern int key_count;

#define EVENT_COUNT LASTEvent
extern void (*handler[EVENT_COUNT])(XEvent *);

void button_press(XEvent *event);
void client_message(XEvent *event);
void configure_notify(XEvent *event);
void configure_request(XEvent *event);
void destroy_notify(XEvent *event);
void enter_notify(XEvent *event);
void expose(XEvent *event);
void focus_in(XEvent *event);
void key_press(XEvent *event);
void mapping_notify(XEvent *event);
void map_request(XEvent *event);
void motion_notify(XEvent *event);
void property_notify(XEvent *event);
void resize_request(XEvent *event);
void unmap_notify(XEvent *event);
int send_event(Window window, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);

#endif