#ifndef TAGGING_H
#define TAGGING_H

#include "monitor.h"

// @descr: Set currently active client tag
// @param: arg -> tag number
void tag(const Arg *arg);

// @descr: Move client to another monitor
// @param: arg -> direction (left=-1, right=1)
void tag_monitor(const Arg *arg);

// @descr: Toggle tags for current client
// @param: arg -> tag number
void toggle_tag(const Arg *arg);

// @descr: Toggle if clients with this tag are shown
// @param: arg -> tag number
void toggle_view(const Arg *arg);

#endif