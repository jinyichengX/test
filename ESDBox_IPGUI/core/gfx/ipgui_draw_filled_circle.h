#ifndef IPGUI_DRAW_FILLED_CIRCLE_H
#define IPGUI_DRAW_FILLED_CIRCLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_draw_arc.h"

typedef struct {
    ipgui_paint_t             paint;                /* 填充     */
    u8_t                      opacity;              /* 不明度   */
    ipgui_blend_mode_t        blend_mode;           /* 混合类型 */
}ipgui_filled_circle_style_t;

extern __IPGUI_API__ void ipgui_draw_filled_circle(
    ipgui_surf_t                * surf,
    ipgui_aabb_t                * clip,
    ipgui_coord_t                 cx, 
    ipgui_coord_t                 cy, 
    ipgui_coord_t                 r,
    ipgui_filled_circle_style_t * style);

#ifdef __cplusplus
}
#endif

#endif

