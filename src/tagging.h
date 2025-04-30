#ifndef TAGGING_H
#define TAGGING_H

#include "definitions.h"
#include "client.h"
#include "monitor.h"
#include "macro.h"

extern Monitor *selected_monitor;
extern const char *tags[15];

// @descr: Set currently active client tag
// @param: arg -> tag number
void tag(const Arg *arg);

// @descr: Move client to another monitor
// @param: arg -> direction (left=-1, right=1)
void tagmon(const Arg *arg);

// @descr: Toggle tags for current client
// @param: arg -> tag number
void toggletag(const Arg *arg);

// @descr: Toggle if clients with this tag are shown
// @param: arg -> tag number
void toggleview(const Arg *arg);

#endif