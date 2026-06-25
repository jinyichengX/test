/*
 * MIT License
 *
 * Copyright (c) 2025 JinYiCheng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
    return (unTicks * (IPGUI_MILLISECOND_PER_SECOND / IPGUI_TICK_PER_SECOND));
}

__IPGUI_API__ ipgui_tick_t ipgui_millis2tick(u32_t unMillis)
{
    return (ipgui_tick_t)(unMillis * (IPGUI_TICK_PER_SECOND / IPGUI_MILLISECOND_PER_SECOND));
}

__IPGUI_API__ ipgui_tick_t ipgui_sec2tick(u32_t unSecs)
{
    return ipgui_millis2tick(sec2millis(unSecs));
}

/* this function need to be called periodically by user */
__IPGUI_API__ void ipgui_tick_inc(void)
{
    ipgui_sys_tick_last = ipgui_sys_tick;
    ipgui_sys_tick ++;
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