#ifndef IPGUI_DRAW_BOX_SHADOW_H
#define IPGUI_DRAW_BOX_SHADOW_H

#include "ipgui_types.h"
#include "ipgui_core.h"
#include "ipgui_blend.h"
#include "ipgui_box_style.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ipgui_color_t      color;
    ipgui_coord_t      blur;
    ipgui_coord_t      spread;
    ipgui_coord_t      offset_x;
    ipgui_coord_t      offset_y;
    ipgui_coord_t      corner_radius;
    u8_t               opacity;
    ipgui_blend_mode_t blend_mode;
} ipgui_box_shadow_style_t;

__IPGUI_API__ void ipgui_draw_box_shadow(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_aabb_t             * content_box,
    ipgui_box_shadow_style_t * style);
__IPGUI_API__ void ipgui_draw_inner_shadow(
    ipgui_surf_t             *surf,
    ipgui_aabb_t             *clip,
    ipgui_aabb_t             *content_box,
    ipgui_box_shadow_style_t *style);

#ifdef __cplusplus
}
#endif

#endif