#ifndef IPGUI_DRAW_TRIANGLE_H
#define IPGUI_DRAW_TRIANGLE_H

#include "ipgui_blend.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* bg color or bg gradient or bg image*/
    ipgui_paint_t      paint;

    /* bg opacity */
    u8_t               opacity;

    /* bg blend mode */
    ipgui_blend_mode_t blend_mode;
}ipgui_triangle_style_t;

extern __IPGUI_API__ void ipgui_draw_triangle(
    ipgui_surf_t           * surf,
    ipgui_aabb_t           * clip,
    ipgui_point_t          * p1,
    ipgui_point_t          * p2,
    ipgui_point_t          * p3,
    ipgui_triangle_style_t * style);

#ifdef __cplusplus
}
#endif

#endif