#ifndef IPGUI_DRAW_BOX_BACKGROUND_H
#define IPGUI_DRAW_BOX_BACKGROUND_H

#include "ipgui_box_style.h"
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
}ipgui_box_bg_style_t;

extern __IPGUI_API__ void ipgui_draw_box_background(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_aabb_t         * box, 
    ipgui_box_style_t    * style,
    ipgui_box_bg_style_t * bg_style);

#ifdef __cplusplus
}
#endif

#endif