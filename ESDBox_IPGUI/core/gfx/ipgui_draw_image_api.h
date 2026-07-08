#ifndef IPGUI_DRAW_IMAGE_API_H
#define IPGUI_DRAW_IMAGE_API_H

#include "ipgui_draw_image.h"

/* 图片对齐方式 — 九宫格（在目标矩形内的9点定位）
 * TOP_LEFT      TOP       TOP_RIGHT
 * LEFT         CENTER      RIGHT
 * BOTTOM_LEFT  BOTTOM     BOTTOM_RIGHT
 */
typedef enum {
    IPGUI_IMG_ALIGN_TOP_LEFT,
    IPGUI_IMG_ALIGN_TOP,
    IPGUI_IMG_ALIGN_TOP_RIGHT,
    IPGUI_IMG_ALIGN_LEFT,
    IPGUI_IMG_ALIGN_CENTER,
    IPGUI_IMG_ALIGN_RIGHT,
    IPGUI_IMG_ALIGN_BOTTOM_LEFT,
    IPGUI_IMG_ALIGN_BOTTOM,
    IPGUI_IMG_ALIGN_BOTTOM_RIGHT,
} ipgui_image_align_t;

/* 图片缩放模式
 * NONE   : 原图大小，不缩放，仅按对齐放置
 * FIT    : 等比缩放至完全容纳在目标矩形内（不裁剪，可能留空）
 * FILL   : 等比缩放至完全覆盖目标矩形（超出部分自动裁剪）
 * STRETCH: 拉伸填满目标矩形，不保持宽高比
 */
typedef enum {
    IPGUI_IMG_FIT_NONE,
    IPGUI_IMG_FIT_FIT,
    IPGUI_IMG_FIT_FILL,
    IPGUI_IMG_FIT_STRETCH,
} ipgui_image_fit_t;

__IPGUI_API__ void ipgui_draw_image_at(
    ipgui_surf_t                   * surf,
    ipgui_image_data_t             * img,
    ipgui_coord_t                    x,
    ipgui_coord_t                    y,
    const ipgui_image_draw_style_t * style,
    ipgui_image_quality_t            quality);

__IPGUI_API__ void ipgui_draw_image_centered(
    ipgui_surf_t                   * surf,
    ipgui_image_data_t             * img,
    ipgui_coord_t                    cx,
    ipgui_coord_t                    cy,
    const ipgui_image_draw_style_t * style,
    ipgui_image_quality_t            quality);

__IPGUI_API__ void ipgui_draw_image_in_rect(
    ipgui_surf_t                   * surf,
    ipgui_image_data_t             * img,
    const ipgui_aabb_t             * target,
    ipgui_image_align_t              align,
    ipgui_image_fit_t                fit,
    const ipgui_image_draw_style_t * style,
    ipgui_image_quality_t            quality);

#endif
