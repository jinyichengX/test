#include "ipgui_draw_box_border.h"
#include "ipgui_ring_mask.h"
#include "ipgui_debug.h"

extern void get_max_radius(
    ipgui_aabb_t      * padding_box, 
    ipgui_box_style_t * style,
    ipgui_coord_t     * r_lt, 
    ipgui_coord_t     * r_rt,
    ipgui_coord_t     * r_lb, 
    ipgui_coord_t     * r_rb);

__IPGUI_STATIC__ void draw_one_corner(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_aabb_t             * cdraw,
    ipgui_aabb_t             * corner,
    ipgui_box_border_style_t * border_style,
    ipgui_coord_t              er,
    ipgui_coord_t              ir,
    s8_t                       x_step,
    u8_t                       y_flip);

__IPGUI_API__ void ipgui_draw_box_border(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_aabb_t             * box,
    ipgui_box_style_t        * style,
    ipgui_box_border_style_t * border_style)
{
    if (!surf || !box || !border_style || border_style->opacity < 3)
        return;

    if (border_style->width <= 0)
        return;

    /* 判断是否需要绘制，draw只在画圆角起作用 */
    ipgui_aabb_t draw;
    if (clip) {
        if (0 != ipgui_aabb_overlap(&draw, clip, &surf->surf))
            return;
    } else draw = surf->surf;

    ipgui_coord_t bw = border_style->width;

    ipgui_aabb_t padding_box;
    padding_box.start.x = box->start.x - (style ? style->left_padding   : 0);
    padding_box.start.y = box->start.y - (style ? style->top_padding    : 0);
    padding_box.end.x   = box->end.x   + (style ? style->right_padding  : 0);
    padding_box.end.y   = box->end.y   + (style ? style->bottom_padding : 0);

    ipgui_aabb_t border_box;
    border_box.start.x = padding_box.start.x - bw;
    border_box.start.y = padding_box.start.y - bw;
    border_box.end.x   = padding_box.end.x   + bw;
    border_box.end.y   = padding_box.end.y   + bw;

    if (0 != ipgui_aabb_overlap(&draw, &draw, &border_box))
        return;

    /* 规整圆角半径，不超过padding box的半宽/半高的最小值 */
    ipgui_coord_t r_lt, r_rt, r_lb, r_rb;
    get_max_radius(&padding_box, style, &r_lt, &r_rt, &r_lb, &r_rb);

#define draw_border_rect(f) \
    ipgui_blend(surf, clip, &(f), &(border_style->paint), \
        border_style->opacity, (u8_t *)0, (ipgui_aabb_t *)0, border_style->blend_mode)

    ipgui_aabb_t fill;
    /* 上边：在左上/右上圆角之间 */
    fill.start.x = padding_box.start.x + r_lt;
    fill.end.x   = padding_box.end.x   - r_rt;
    if (fill.end.x >= fill.start.x) {
        fill.start.y = padding_box.start.y - bw;
        fill.end.y   = padding_box.start.y - 1;
        draw_border_rect(fill);
    }

    /* 下边：在左下/右下圆角之间 */
    fill.start.x = padding_box.start.x + r_lb;
    fill.end.x   = padding_box.end.x   - r_rb;
    if (fill.end.x >= fill.start.x) {
        fill.start.y = padding_box.end.y + 1;
        fill.end.y   = padding_box.end.y + bw;
        draw_border_rect(fill);
    }

    /* 左边：在左上/左下圆角之间 */
    fill.start.y = padding_box.start.y + r_lt;
    fill.end.y   = padding_box.end.y   - r_lb;
    if (fill.end.y >= fill.start.y) {
        fill.start.x = padding_box.start.x - bw;
        fill.end.x   = padding_box.start.x - 1;
        draw_border_rect(fill);
    }

    /* 右边：在右上/右下圆角之间 */
    fill.start.y = padding_box.start.y + r_rt;
    fill.end.y   = padding_box.end.y   - r_rb;
    if (fill.end.y >= fill.start.y) {
        fill.start.x = padding_box.end.x + 1;
        fill.end.x   = padding_box.end.x + bw;
        draw_border_rect(fill);
    }

    /* 补全radius为0时的缺角 */
    /* 左上角 */
    if (r_lt == 0) {
        fill.start.x = padding_box.start.x - bw;
        fill.end.x   = padding_box.start.x - 1;
        fill.start.y = padding_box.start.y - bw;
        fill.end.y   = padding_box.start.y - 1;
        draw_border_rect(fill);
    }

    /* 右上角 */
    if (r_rt == 0) {
        fill.start.x = padding_box.end.x + 1;
        fill.end.x   = padding_box.end.x + bw;
        fill.start.y = padding_box.start.y - bw;
        fill.end.y   = padding_box.start.y - 1;
        draw_border_rect(fill);
    }

    /* 左下角 */
    if (r_lb == 0) {
        fill.start.x = padding_box.start.x - bw;
        fill.end.x   = padding_box.start.x - 1;
        fill.start.y = padding_box.end.y + 1;
        fill.end.y   = padding_box.end.y + bw;
        draw_border_rect(fill);
    }

    /* 右下角 */
    if (r_rb == 0) {
        fill.start.x = padding_box.end.x + 1;
        fill.end.x   = padding_box.end.x + bw;
        fill.start.y = padding_box.end.y + 1;
        fill.end.y   = padding_box.end.y + bw;
        draw_border_rect(fill);
    }
#undef draw_border_rect

    ipgui_aabb_t corner, cdraw;
    ipgui_coord_t er, ir;
    /* 左上角，圆心在 (padding_box.start.x + r_lt, padding_box.start.y + r_lt) */
    if (r_lt > 0) {
        er = r_lt + bw;
        ir = r_lt;
        corner.start.x = padding_box.start.x - bw;
        corner.end.x   = padding_box.start.x + r_lt - 1;
        corner.start.y = padding_box.start.y - bw;
        corner.end.y   = padding_box.start.y + r_lt - 1;
        if (0 == ipgui_aabb_overlap(&cdraw, &draw, &corner))
            draw_one_corner(surf, clip, &cdraw, &corner, border_style, er, ir, -1, 1);
    }

    /* 右上角，圆心在 (padding_box.end.x - r_rt, padding_box.start.y + r_rt) */
    if (r_rt > 0) {
        er = r_rt + bw;
        ir = r_rt;
        corner.start.x = padding_box.end.x - r_rt + 1;
        corner.end.x   = padding_box.end.x + bw;
        corner.start.y = padding_box.start.y - bw;
        corner.end.y   = padding_box.start.y + r_rt - 1;
        if (0 == ipgui_aabb_overlap(&cdraw, &draw, &corner))
            draw_one_corner(surf, clip, &cdraw, &corner, border_style, er, ir,  1, 1);
    }

    /* 左下角，圆心在 (padding_box.start.x + r_lb, padding_box.end.y - r_lb) */
    if (r_lb > 0) {
        er = r_lb + bw;
        ir = r_lb;
        corner.start.x = padding_box.start.x - bw;
        corner.end.x   = padding_box.start.x + r_lb - 1;
        corner.start.y = padding_box.end.y - r_lb + 1;
        corner.end.y   = padding_box.end.y + bw;
        if (0 == ipgui_aabb_overlap(&cdraw, &draw, &corner))
            draw_one_corner(surf, clip, &cdraw, &corner, border_style, er, ir, -1, 0);
    }

    /* 右下角，圆心在 (padding_box.end.x - r_rb, padding_box.end.y - r_rb) */
    if (r_rb > 0) {
        er = r_rb + bw;
        ir = r_rb;
        corner.start.x = padding_box.end.x - r_rb + 1;
        corner.end.x   = padding_box.end.x + bw;
        corner.start.y = padding_box.end.y - r_rb + 1;
        corner.end.y   = padding_box.end.y + bw;
        if (0 == ipgui_aabb_overlap(&cdraw, &draw, &corner))
            draw_one_corner(surf, clip, &cdraw, &corner, border_style, er, ir,  1, 0);
    }
}

__IPGUI_STATIC__ void draw_one_corner(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_aabb_t             * cdraw,
    ipgui_aabb_t             * corner,
    ipgui_box_border_style_t * border_style,
    ipgui_coord_t              er,
    ipgui_coord_t              ir,
    s8_t                       x_step,
    u8_t                       y_flip)
{
    ipgui_coord_t w, h;
    w = ipgui_aabb_width (cdraw);
    h = ipgui_aabb_height(cdraw);

    ipgui_coord_t res_h;
    u8_t * mbuf = ipgui_mask_buf_acquire(w, h, &res_h);
    if ((!mbuf) || (res_h == 0)) {
        ipgui_dbg_error("error: box corner rasterization failed to acquire mask buffer\r\n");
        return;
    }

    ipgui_coord_t drawn_h = 0;

    ipgui_aabb_t mask_aabb;
    mask_aabb.start.x = cdraw->start.x;
    mask_aabb.end.x   = cdraw->end.x;
    
    /* 计算水平起点的逻辑 X (1~er)：
     * x_step == -1 (左半区): cdraw->start.x 是此行最左端，离圆心最远，逻辑X最大。
     * x_step ==  1 (右半区): cdraw->start.x 是此行最左端，离圆心最近，逻辑X最小。
     */
    ipgui_coord_t circle_x_start = (x_step == -1) ?
                (corner->end.x + 1 - cdraw->start.x) :
                (cdraw->start.x - corner->start.x + 1);
    while (h > 0) {
        ipgui_coord_t current_h = IPGUI_MIN(h, res_h);
        ipgui_coord_t draw_y, circle_y;
        u8_t * row_mask;

        /* 逐行填充mask */
        for (int i = 0; i < current_h; i ++) {
            draw_y = cdraw->start.y + drawn_h + i;
            
            /* 计算相对于圆心的逻辑 Y (1~er)：
             * y_flip == 1 (上半区): 越往下离圆心越近，逻辑y越小。
             * y_flip == 0 (下半区): 越往下离圆心越远，逻辑y越大。
             */
            circle_y = y_flip ? 
                (corner->end.y + 1 - draw_y) : 
                (draw_y - corner->start.y + 1);

            row_mask = mbuf + i * w;
            
            ipgui_fetch_ring_mask(circle_x_start, circle_y, ir, er, x_step, row_mask, w);
        }

        /* blend */
        mask_aabb.start.y = cdraw->start.y + drawn_h;
        mask_aabb.end.y   = mask_aabb.start.y + current_h - 1;

        ipgui_blend(surf, clip, &mask_aabb, &(border_style->paint),
                    border_style->opacity, mbuf, &mask_aabb, border_style->blend_mode);

        drawn_h += current_h;
        h       -= current_h;
    }

    ipgui_mask_buf_free(mbuf);
}