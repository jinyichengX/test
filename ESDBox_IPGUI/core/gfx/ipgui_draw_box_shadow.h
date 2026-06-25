#ifndef IPGUI_DRAW_BOX_SHADOW_H
#define IPGUI_DRAW_BOX_SHADOW_H

#include "ipgui_blend.h"
#include "ipgui_box_style.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 盒阴影样式 */
typedef struct {
    ipgui_color_t      color;          /* 阴影颜色 */
    u8_t               opacity;        /* 阴影不透明度 */
    ipgui_coord_t      blur;           /* 模糊半径 */
    ipgui_coord_t      spread;         /* 扩展半径 */
    ipgui_coord_t      offset_x;       /* 水平偏移 */
    ipgui_coord_t      offset_y;       /* 垂直偏移 */
} ipgui_box_shadow_style_t;

/* 绘制盒阴影
 * @surf:     目标绘制表面
 * @clip:     裁剪区域（可为 NULL = surf 边界）
 * @box:      控件边界盒（padding_box，全局坐标）
 * @style:    圆角样式（可为 NULL = 无圆角）
 * @shadow_style: 阴影样式
 */
extern __IPGUI_API__ void ipgui_draw_box_shadow(
    ipgui_surf_t              * surf,
    ipgui_aabb_t              * clip,
    ipgui_aabb_t              * box,
    ipgui_box_style_t         * style,
    ipgui_box_shadow_style_t  * shadow_style);

#ifdef __cplusplus
}
#endif

#endif
