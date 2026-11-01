#ifndef IPGUI_DRAW_BOX_SHADOW_H
#define IPGUI_DRAW_BOX_SHADOW_H

#include "ipgui_blend.h"
#include "ipgui_box_style.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 盒子外阴影样式，作用于盒子，不区分是带了边框的本体还是不带边框的本体 */
typedef struct {
    ipgui_paint_t      paint;          /* 阴影 */
    u8_t               opacity;        /* 阴影不透明度 */
    ipgui_coord_t      blur;           /* 模糊半径 */
    ipgui_coord_t      spread;         /* 扩展半径 */
    ipgui_coord_t      offset_x;       /* 水平偏移 */
    ipgui_coord_t      offset_y;       /* 垂直偏移 */
} ipgui_box_outer_shadow_style_t;

extern __IPGUI_API__ void ipgui_draw_box_outer_shadow(
    ipgui_surf_t              * surf,
    ipgui_aabb_t              * clip,
    ipgui_aabb_t              * box,
    ipgui_box_style_t         * style,
    ipgui_box_outer_shadow_style_t  * shadow_style);

#ifdef __cplusplus
}
#endif

#endif
