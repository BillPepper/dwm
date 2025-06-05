#include "tagging.h"

void tag(const Arg *arg) {
  if (selected_monitor->selected_client && arg->ui & TAGMASK) {
    selected_monitor->selected_client->tags = arg->ui & TAGMASK;
    focus(NULL);
    arrange(selected_monitor);
  }
}

void tagmon(const Arg *arg) {
  if (!selected_monitor->selected_client || !monitors->next){
    return;
  }

  sendmon(selected_monitor->selected_client, dir_to_monitor(arg->i));
}

void toggletag(const Arg *arg) {
  unsigned int newtags;

  if (!selected_monitor->selected_client) {
    return;
  }

  newtags = selected_monitor->selected_client->tags ^ (arg->ui & TAGMASK);
  if (newtags) {
    selected_monitor->selected_client->tags = newtags;
    focus(NULL);
    arrange(selected_monitor);
  }
}

void toggleview(const Arg *arg) {
  unsigned int newtagset = selected_monitor->tag_set[selected_monitor->selected_tags] ^ (arg->ui & TAGMASK);

  if (newtagset) {
    selected_monitor->tag_set[selected_monitor->selected_tags] = newtagset;
    focus(NULL);
    arrange(selected_monitor);
  }
}