#ifndef MACRO_H
#define MACRO_H

/* macros */
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask) (mask & ~(numlockmask | LockMask) &  (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))
#define INTERSECT(x, y, w, h, m) (MAX(0, MIN((x) + (w), (m)->window_area.position.x + (m)->window_area.size.h) - MAX((x), (m)->window_area.position.x)) * MAX(0, MIN((y) + (h), (m)->window_area.position.y + (m)->window_area.size.h) - MAX((y), (m)->window_area.position.y)))
#define ISVISIBLE(C) ((C->tags & C->monitor->tag_set[C->monitor->selected_tags]))
#define LENGTH(X) (sizeof X / sizeof X[0])
#define MOUSEMASK (BUTTONMASK | PointerMotionMask)
#define WIDTH(X) ((X)->area.size.w + 2 * (X)->bw)
#define HEIGHT(X) ((X)->area.size.h + 2 * (X)->bw)
#define TAGMASK ((1 << LENGTH(tags)) - 1)
#define TEXTW(X) (drw_fontset_getwidth(drw, (X)) + padding)

#endif