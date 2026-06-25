#ifndef IPGUI_TIME_H
#define IPGUI_TIME_H

#include "ipgui_utils.h"
#include "ipgui_conf.h"

extern __IPGUI_API__ u32_t ipgui_tick2millis(ipgui_tick_t unTicks);
extern __IPGUI_API__ ipgui_tick_t ipgui_millis2tick(u32_t unMillis);
extern __IPGUI_API__ ipgui_tick_t ipgui_sec2tick(u32_t unSecs);
extern __IPGUI_API__ void ipgui_tick_inc(void);
extern __IPGUI_API__ ipgui_tick_t ipgui_tick_now(void);
extern __IPGUI_API__ ipgui_tick_t ipgui_tick_passed_last(void);

extern ipgui_tick_t ipgui_sys_tick;
extern ipgui_tick_t ipgui_sys_tick_last;

#endif
