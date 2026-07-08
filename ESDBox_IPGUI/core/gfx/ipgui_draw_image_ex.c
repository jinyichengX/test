#include "ipgui_draw_image_ex.h"

__IPGUI_API__ void ipgui_draw_image_ex(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_img_mask_t         * img_mask,
    ipgui_image_data_t       * img_data,
    ipgui_point_t            * pivot,    /* 相对于图片的变换点 如果是子图那么就是相对于子图的 */
    ipgui_point_t            * anchor,
    ipgui_trans_mat_t        * trans,
    ipgui_image_draw_style_t * style,
    ipgui_image_quality_t      quality)
{
    if ((!surf) || (!img_data) || (!pivot) || (!anchor) || (!style))
        return;

    if (style->opacity < 3)
        return;
    
    if (!img_mask) {
        ipgui_draw_image(
            surf,
            clip,
            img_data,
            pivot,
            anchor,
            trans,
            style,
            quality);
    }

    /* use mask to clip image */
    /* firstly zoom image mask */

    /* then apply to mask */
    
}