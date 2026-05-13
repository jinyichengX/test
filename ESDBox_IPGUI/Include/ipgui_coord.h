#ifndef IPGUI_COORD_H
#define IPGUI_COORD_H

#include "ipgui_utils.h"
#include "ipgui_conf.h"
#include "ipgui_types.h"

IPGUI_HEADER_BEGIN

#if (IPGUI_XRES_MAX <= 65535) && (IPGUI_YRES_MAX <= 65535)
typedef s32_t ipgui_coord_t;
typedef float ipgui_coordf_t;
#define IPGUI_COORD_MAX 2147483647
#define IPGUI_COORD_MIN -2147483648
#else
typedef long ipgui_coord_t;
typedef double ipgui_coordf_t;
#define IPGUI_COORD_MAX 9223372036854775807
#define IPGUI_COORD_MIN -9223372036854775808
#endif

/* cartesian coordinates to LCD coordinates (数学中采用的坐标系就是笛卡尔坐标系) */
__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_cartesian2lcd(ipgui_coord_t x, ipgui_coord_t y, ipgui_coord_t * lcd_x, ipgui_coord_t * lcd_y, ipgui_coord_t height)
{
    * lcd_x = x;
    * lcd_y = height - y;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_lcd2cartesian(ipgui_coord_t x, ipgui_coord_t y, ipgui_coord_t * car_x, ipgui_coord_t * car_y, ipgui_coord_t height)
{
    * car_x = x;
    * car_y = height - y;
}

IPGUI_HEADER_END

#endif