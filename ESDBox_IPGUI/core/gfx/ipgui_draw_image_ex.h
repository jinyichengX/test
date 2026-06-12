#ifndef IPGUI_DRAW_IMAGE_EX_H
#define IPGUI_DRAW_IMAGE_EX_H

#include "ipgui_image_mask.h"
#include "ipgui_draw_image.h"

extern __IPGUI_API__ void ipgui_draw_image_ex(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_img_mask_t         * img_mask,
    ipgui_image_data_t       * img_data,
    ipgui_point_t            * pivot,    /* 相对于图片的变换点 如果是子图那么就是相对于子图的 */
    ipgui_point_t            * anchor,
    ipgui_trans_mat_t        * trans,
    ipgui_image_draw_style_t * style);

#endif