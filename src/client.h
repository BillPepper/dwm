#ifndef CLIENT_H
#define CLIENT_H

#include "definitions.h"

static void killclient(const Arg *arg);                                         // close client window
static void togglefloating(const Arg *arg);                                     // toggle floating for current client (arg unused)
static void togglefullscreen(const Arg *arg);                                   // toggle fullscreen for current client (arg unused)
static void updateclientlist(void);                                             // update all clients on all monitors
static int applysizehints(Client *client, Area *area, int interact);
static void applyrules(Client *client);                                         // apply client rules defined in config
static void attach(Client *client);                                             // attach new client to client list
static void attachstack(Client *client);                                        // attach client to it's monitors stack
static void configure(Client *client);                                          // configure new client
static void detach(Client *client);                                             // remove client from client list
static void detachstack(Client *client);                                        // remove client from it's monitors stack
static void focus(Client *client);                                              // focus given client
static Client *nexttiled(Client *client);                                       // get next tiled client
static void pop(Client *client);                                                // remove client from stack?
static void resize(Client *client, Area *area, int interact);                   // apply size hints
static void resizeclient(Client *client, Area *area);                           // resize client
static void sendmon(Client *client, Monitor *m);                                // send client to montior
static void setclientstate(Client *client, long state);                         // set the client state (normal/icon/withdrawn)
static void setfocus(Client *client);                                           // focus given client if focusable
static void setfullscreen(Client *client, int fullscreen);                      // set clients fullscreen state
static void seturgent(Client *client, int urgency_state);                       // set windows urgency state
static void showhide(Client *client);                                           // recursively show and hide windows in stack of given window
static void unfocus(Client *client, int setfocus);                              // ungrab button and update border
static void updatesizehints(Client *client);                                    // re-calculate client size hints
static void updatetitle(Client *client);                                        // get title from client, and set it to dwm title
static void updatewindowtype(Client *client);                                   // update fullscreen and floating window types
static void updatewmhints(Client *client);                                      // update urgency and input hints

#endif