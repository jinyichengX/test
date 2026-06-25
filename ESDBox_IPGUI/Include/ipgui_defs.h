#ifndef IPGUI_DEFS_H
#define IPGUI_DEFS_H

#include "ipgui_utils.h"

#define IPGUI_TIME_FOREVER ((ipgui_tick_t)(-1))

/* MAX tick */
#define IPGUI_TIME_TICK_MAX (ipgui_tick_t)(IPGUI_TIME_FOREVER - 1)

/* widget per level capacity */
#define IPGUI_WIDGET_PER_LEVEL_CAPACITY 30

#endif