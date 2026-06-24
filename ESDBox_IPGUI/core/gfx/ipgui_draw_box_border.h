#ifndef IPGUI_DRAW_BOX_BORDER_H
#define IPGUI_DRAW_BOX_BORDER_H

#include "ipgui_blend.h"
#include "ipgui_box_style.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ipgui_paint_t      paint;
    u8_t               opacity;
    ipgui_coord_t      width;
    ipgui_blend_mode_t blend_mode;
}ipgui_box_border_style_t;

extern __IPGUI_API__ void ipgui_draw_box_border(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_aabb_t             * box, 
    ipgui_box_style_t        * style,
    ipgui_box_border_style_t * border_style);

#ifdef __cplusplus
}
#endif

#endif