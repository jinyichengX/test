#ifndef IPGUI_DRAW_ICON_API_H
#define IPGUI_DRAW_ICON_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_draw_icon.h"

/* 图标对齐方式 — 九宫格（在目标矩形内的9点定位）
 * TOP_LEFT      TOP       TOP_RIGHT
 * LEFT         CENTER      RIGHT
 * BOTTOM_LEFT  BOTTOM     BOTTOM_RIGHT
 */
typedef enum {
    IPGUI_ICON_ALIGN_TOP_LEFT,
    IPGUI_ICON_ALIGN_TOP,
    IPGUI_ICON_ALIGN_TOP_RIGHT,
    IPGUI_ICON_ALIGN_LEFT,
    IPGUI_ICON_ALIGN_CENTER,
    IPGUI_ICON_ALIGN_RIGHT,
    IPGUI_ICON_ALIGN_BOTTOM_LEFT,
    IPGUI_ICON_ALIGN_BOTTOM,
    IPGUI_ICON_ALIGN_BOTTOM_RIGHT,
} ipgui_icon_align_t;

/* 图标缩放模式
 * NONE   : 原图大小，不缩放，仅按对齐放置
 * FIT    : 等比缩放至完全容纳在目标矩形内（不裁剪，可能留空）
 * FILL   : 等比缩放至完全覆盖目标矩形（超出部分自动裁剪）
 * STRETCH: 拉伸填满目标矩形，不保持宽高比
 */
typedef enum {
    IPGUI_ICON_FIT_NONE,
    IPGUI_ICON_FIT_FIT,
    IPGUI_ICON_FIT_FILL,
    IPGUI_ICON_FIT_STRETCH,
} ipgui_icon_fit_t;

__IPGUI_API__ void ipgui_draw_icon_at(
    ipgui_surf_t                    * surf,
    ipgui_icon_data_t               * icon,
    ipgui_coord_t                     x,
    ipgui_coord_t                     y,
    const ipgui_draw_icon_style_t   * style);

__IPGUI_API__ void ipgui_draw_icon_centered(
    ipgui_surf_t                    * surf,
    ipgui_icon_data_t               * icon,
    ipgui_coord_t                     cx,
    ipgui_coord_t                     cy,
    const ipgui_draw_icon_style_t   * style);

__IPGUI_API__ void ipgui_draw_icon_in_rect(
    ipgui_surf_t                    * surf,
    ipgui_icon_data_t               * icon,
    const ipgui_aabb_t              * target,
    ipgui_icon_align_t                align,
    ipgui_icon_fit_t                  fit,
    const ipgui_draw_icon_style_t   * style);

#ifdef __cplusplus
}
#endif

#endif
