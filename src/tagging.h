#ifndef TAGGING_H
#define TAGGING_H

#include "definitions.h"
#include "client.h"
#include "monitor.h"
#include "macro.h"

extern Monitor *selected_monitor;
extern const char *tags[15];

void tag(const Arg *arg);                                                // show clients with specified tag
void tagmon(const Arg *arg);                                             // move to next/prev monitor
void toggletag(const Arg *arg);
void toggleview(const Arg *arg);

#endif