#ifndef IPGUI_DRAW_IMAGE_H
#define IPGUI_DRAW_IMAGE_H

#include "ipgui_blend.h"

typedef struct {
    /* T = |a  b|
     *     |c  d|
     */
    ipgui_scoord_t      a, b, c, d;
}ipgui_trans_mat_t;

typedef struct {

    //ipgui_gradient_mask_t mask_image;
    u8_t                opacity;
    ipgui_blend_mode_t  blend_mode;
}ipgui_image_draw_style_t;

typedef enum {
    IPGUI_IMAGE_QUALITY_LOW,        /* 低质量 */
    IPGUI_IMAGE_QUALITY_MEDIUM,     /* 中等质量 */
    IPGUI_IMAGE_QUALITY_HIGH,       /* 质量 */
    // IPGUI_IMAGE_QUALITY_ULTRA,      /* 极致 */
   }ipgui_image_quality_t;

extern __IPGUI_API__ void ipgui_draw_image(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_image_data_t       * img_data,
    ipgui_point_t            * pivot,    /* 相对于图片的变换点 如果是子图那么就是相对于子图的 */
    ipgui_point_t            * anchor,
    ipgui_trans_mat_t        * trans,
    ipgui_image_draw_style_t * style,
    ipgui_image_quality_t      quality);
    
extern __IPGUI_API__ void ipgui_draw_image_tiled(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_image_data_t       * img_data,
    ipgui_point_t            * pivot,    /* 相对于图片的变换点 如果是子图那么就是相对于子图的 */
    ipgui_point_t            * anchor,
    ipgui_trans_mat_t        * trans,
    ipgui_image_draw_style_t * style,
    ipgui_image_quality_t      quality);

#endif