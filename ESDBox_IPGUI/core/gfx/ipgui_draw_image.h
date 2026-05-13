#ifndef IPGUI_DRAW_IMAGE_H
#define IPGUI_DRAW_IMAGE_H

#include "ipgui_box_style.h"
#include "ipgui_core.h"
#include "ipgui_blend.h"

typedef struct {
    /* T = |a  b|
     *     |c  d|
     */
    ipgui_scoord_t      a, b, c, d;
}ipgui_trans_mat_t;


typedef struct {
    /* 像素格式 */
    ipgui_image_fomat_t fmt;

    u8_t                px_size;    /* 每像素大小（单位：字节），必须大于等于像素格式对应的字节数 */

    /* 图片宽度和高度 */
    ipgui_coord_t       w, h;

    /* per line stride
     * 这个参数是用于加速用，一般是每像素大小乘以宽度 + padding
     * 可以让一个ipgui_img_raw_t指向大图的中间的部分子图
     * 只要stride依然是大图的宽度，采样器就能正确换行
     */
    u32_t              stride;

    /* 像素数据 */
    u8_t             * pixmap;
}ipgui_image_data_t;

typedef struct {

    //ipgui_gradient_mask_t mask_image;
    u8_t                opacity;
    ipgui_blend_mode_t  blend_mode;
}ipgui_image_draw_style_t;

extern __IPGUI_API__ void ipgui_draw_image(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_image_data_t       * img_data,
    ipgui_point_t            * pivot,    /* 相对于图片的变换点 如果是子图那么就是相对于子图的 */
    ipgui_point_t            * anchor,
    ipgui_trans_mat_t        * trans,
    ipgui_image_draw_style_t * style);

#endif