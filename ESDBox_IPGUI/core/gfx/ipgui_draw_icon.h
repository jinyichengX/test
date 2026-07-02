#ifndef IPGUI_DRAW_ICON_H
#define IPGUI_DRAW_ICON_H

#include "ipgui_draw_image.h"

/* 和画图片不同，画图标不支持子图标 */
typedef struct {
    /* icon宽度和高度 */
    ipgui_coord_t       w, h;

    /*
     * 掩码数据，w*h 字节，行主序（row-major），每字节 [0,255] = alpha
     *
     * !!! 必须连续存储，无行尾 padding !!!
     * stride 隐式 = w 字节，渲染管线按 mask[y*w + x] 寻址。
     * 如果加了 padding 会导致行偏移错乱、渲染结果错位。
     */
    u8_t              * mask;
}ipgui_icon_data_t;

typedef struct {
    u8_t               opacity;
    ipgui_paint_t      paint;
    ipgui_blend_mode_t blend_mode;
}ipgui_draw_icon_style_t;

__IPGUI_API__ void ipgui_draw_icon(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_icon_data_t        * icon_data,
    ipgui_point_t            * pivot,    /* 相对于图标的变换点*/
    ipgui_point_t            * anchor,
    ipgui_trans_mat_t        * trans,
    ipgui_draw_icon_style_t  * style);
    
#endif
