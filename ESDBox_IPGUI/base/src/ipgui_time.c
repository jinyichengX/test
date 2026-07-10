#include "ipgui_time.h"

ipgui_tick_t ipgui_sys_tick = 0;
ipgui_tick_t ipgui_sys_tick_last = 0;

#define IPGUI_MILLISECOND_PER_SECOND 1000u

__IPGUI_STATIC__ u32_t sec2millis(u32_t unSecs)
{
    return unSecs * IPGUI_MILLISECOND_PER_SECOND;
}

__IPGUI_API__ u32_t ipgui_tick2millis(ipgui_tick_t unTicks)
{
    return (unTicks * IPGUI_MILLISECOND_PER_SECOND / IPGUI_TICK_PER_SECOND);
}

__IPGUI_API__ ipgui_tick_t ipgui_millis2tick(u32_t unMillis)
{
    return (ipgui_tick_t)(unMillis * IPGUI_TICK_PER_SECOND / IPGUI_MILLISECOND_PER_SECOND);
}

__IPGUI_API__ ipgui_tick_t ipgui_sec2tick(u32_t unSecs)
{
    return ipgui_millis2tick(sec2millis(unSecs));
}
// #include <SDL.h>
/* this function need to be called periodically by user */
__IPGUI_API__ void ipgui_tick_inc(void)
{
    ipgui_sys_tick_last = ipgui_sys_tick;
    ipgui_sys_tick ++;
    // ipgui_sys_tick = SDL_GetTicks();
}

__IPGUI_API__ ipgui_tick_t ipgui_tick_now(void)
{
    return ipgui_sys_tick;
}

__IPGUI_API__ ipgui_tick_t ipgui_tick_passed(ipgui_tick_t last)
{
    return ipgui_tick_now() - last;
}

__IPGUI_API__ ipgui_tick_t ipgui_tick_passed_last(void)
{
    return ipgui_tick_passed(ipgui_sys_tick_last);
}