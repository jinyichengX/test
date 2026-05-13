#ifndef IPGUI_TIME_H
#define IPGUI_TIME_H
#include "ipgui_utils.h"

IPGUI_HEADER_BEGIN  _______________MARKER_______________

#include "ipgui_conf.h"
#include "ipgui_types.h"

extern __IPGUI_API__ unsigned int ipgui_tick2millis(ipgui_tick_t unTicks);

extern __IPGUI_API__ ipgui_tick_t ipgui_millis2tick(unsigned int unMillis);

extern __IPGUI_API__ ipgui_tick_t ipgui_sec2tick(unsigned int unSecs);

extern __IPGUI_API__ void ipgui_tick_inc(void);

extern __IPGUI_API__ ipgui_tick_t ipgui_tick_passed_last(void);
IPGUI_HEADER_END    _______________MARKER_______________

#endif
