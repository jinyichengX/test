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


// #ifndef __IPGUI_DRAW_BOX_SHADOW_H__
// #define __IPGUI_DRAW_BOX_SHADOW_H__

// #include "ipgui_types.h"       // 确保引入了基础类型 u8_t, s32_t 等
// #include "ipgui_geometry.h"    // 确保引入了 ipgui_aabb_t, ipgui_coord_t 等
// #include "ipgui_surface.h"     // 确保引入了 ipgui_surf_t
// #include "ipgui_paint.h"       // 确保引入了 ipgui_paint_t 和 ipgui_blend_mode_t
// #include "ipgui_draw_box_background.h" // 确保引入了 ipgui_box_style_t

// #ifdef __cplusplus
// extern "C" {
// #endif

// /**
//  * @brief 矩形阴影样式结构体定义
//  */
// typedef struct {
//     ipgui_paint_t      paint;        // 阴影的填充画刷（通常为纯色，支持渐变）
//     u8_t               opacity;      // 阴影的全局不透明度 (0 ~ 255)
//     ipgui_coord_t      offset_x;     // 阴影在 X 轴的偏移量（正数向右，负数向左）
//     ipgui_coord_t      offset_y;     // 阴影在 Y 轴的偏移量（正数向下，负数向上）
//     ipgui_coord_t      blur_radius;  // 模糊半径 (Blur)
//     ipgui_coord_t      spread;       // 阴影扩张大小 (Spread)
//     ipgui_blend_mode_t blend_mode;   // 混合模式
// } ipgui_box_shadow_style_t;

// /**
//  * @brief 绘制带有抗锯齿圆角的高品质软光栅矩形阴影
//  * * @param surf         目标画布结构体指针
//  * @param clip         裁剪区域（可为 NULL，则使用画布自带默认区域）
//  * @param box          原始主体矩形的 AABB 框（Padding 之前的基本大小）
//  * @param style        盒子样式（主要提取其圆角 radius 和 padding 信息）
//  * @param shadow_style 阴影样式参数
//  */
// __IPGUI_API__ void ipgui_draw_box_shadow(
//     ipgui_surf_t             * surf,
//     ipgui_aabb_t             * clip,
//     ipgui_aabb_t             * box,
//     ipgui_box_style_t        * style,
//     ipgui_box_shadow_style_t * shadow_style);

// #ifdef __cplusplus
// }
// #endif

// #endif /* __IPGUI_DRAW_BOX_SHADOW_H__ */