#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdio.h>
#include "macro.h"
#include "definitions.h"
#include "client.h"
#include "util.h"

// layout 'arrange' function callbacks
void monocle(Monitor *monitor);                  // monocle layout callback
void tile(Monitor *monitor);                     // tile layout callback

#endif