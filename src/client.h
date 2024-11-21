#ifndef CLIENT_H
#define CLIENT_H

#include "definitions.h"

void killclient(const Arg *arg);                                         // close client window
void togglefloating(const Arg *arg);                                     // toggle floating for current client (arg unused)
void togglefullscreen(const Arg *arg);                                   // toggle fullscreen for current client (arg unused)
void updateclientlist(void);                                             // update all clients on all monitors
int applysizehints(Client *client, Area *area, int interact);
void applyrules(Client *client);                                         // apply client rules defined in config
void attach(Client *client);                                             // attach new client to client list
void attachstack(Client *client);                                        // attach client to it's monitors stack
void configure(Client *client);                                          // configure new client
void detach(Client *client);                                             // remove client from client list
void detachstack(Client *client);                                        // remove client from it's monitors stack
void focus(Client *client);                                              // focus given client
Client *nexttiled(Client *client);                                       // get next tiled client
void pop(Client *client);                                                // remove client from stack?
void resize(Client *client, Area *area, int interact);                   // apply size hints
void resizeclient(Client *client, Area *area);                           // resize client
void sendmon(Client *client, Monitor *m);                                // send client to montior
void setclientstate(Client *client, long state);                         // set the client state (normal/icon/withdrawn)
void setfocus(Client *client);                                           // focus given client if focusable
void setfullscreen(Client *client, int fullscreen);                      // set clients fullscreen state
void seturgent(Client *client, int urgency_state);                       // set windows urgency state
void showhide(Client *client);                                           // recursively show and hide windows in stack of given window
void unfocus(Client *client, int setfocus);                              // ungrab button and update border
void updatesizehints(Client *client);                                    // re-calculate client size hints
void updatetitle(Client *client);                                        // get title from client, and set it to dwm title
void updatewindowtype(Client *client);                                   // update fullscreen and floating window types
void updatewmhints(Client *client);                                      // update urgency and input hints

#endif