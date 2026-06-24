#ifndef IPGUI_COLOR_H
#define IPGUI_COLOR_H

#include "ipgui_types.h"

typedef union {
    struct {
        u8_t r;
        u8_t g;
        u8_t b;
        u8_t a;
    };
    u32_t v;
}ipgui_color_t;

#define IPGUI_COLOR_R(x) (x.r)
#define IPGUI_COLOR_G(x) (x.g)
#define IPGUI_COLOR_B(x) (x.b)
#define IPGUI_COLOR_A(x) (x.a)

#define IPGUI_COLOR_SET_R(x, r_v) (x.r = r_v)
#define IPGUI_COLOR_SET_G(x, g_v) (x.g = g_v)
#define IPGUI_COLOR_SET_B(x, b_v) (x.b = b_v)
#define IPGUI_COLOR_SET_A(x, a_v) (x.a = a_v)

#define IPGUI_COLOR_SET(x, alpha, rgb) x.a = alpha; x.r = rgb >> 16; x.g = rgb >> 8; x.b = rgb;

#define IPGUI_COLOR_RGBA(r,g,b,a) (((u32_t)a << 24) | ((u32_t)r << 16) | ((u32_t)g << 8) | (u32_t)b)

#endif