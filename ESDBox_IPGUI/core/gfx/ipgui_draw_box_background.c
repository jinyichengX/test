#include "ipgui_draw_box_background.h"
#include "ipgui_ring_mask.h"
#include "ipgui_mask_buf.h"
#include "ipgui_debug.h"

__IPGUI_STATIC__ void draw_one_corner(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_aabb_t         * cdraw,
    ipgui_aabb_t         * corner,
    ipgui_box_bg_style_t * bg_style,
    ipgui_coord_t          er,
    ipgui_coord_t          ir,
    s8_t                   x_step,
    u8_t                   y_flip);

void get_max_radius(
    ipgui_aabb_t * padding_box, ipgui_box_style_t * style,
    ipgui_coord_t * r_lt, ipgui_coord_t * r_rt,
    ipgui_coord_t * r_lb, ipgui_coord_t * r_rb)
{
    ipgui_coord_t rmax_w = ipgui_aabb_width (padding_box) >> 1;
    ipgui_coord_t rmax_h = ipgui_aabb_height(padding_box) >> 1;
    ipgui_coord_t rmax   = IPGUI_MIN(rmax_w, rmax_h);

    * r_lt = IPGUI_MIN(rmax, style ? style->left_top_radius     : 0);
    * r_rt = IPGUI_MIN(rmax, style ? style->right_top_radius    : 0);
    * r_lb = IPGUI_MIN(rmax, style ? style->left_bottom_radius  : 0);
    * r_rb = IPGUI_MIN(rmax, style ? style->right_bottom_radius : 0);
    
    * r_lt = IPGUI_MAX(0, * r_lt);
    * r_rt = IPGUI_MAX(0, * r_rt);
    * r_lb = IPGUI_MAX(0, * r_lb);
    * r_rb = IPGUI_MAX(0, * r_rb);
}

__IPGUI_API__ void ipgui_draw_box_background(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_aabb_t         * box,
    ipgui_box_style_t    * style,
    ipgui_box_bg_style_t * bg_style)
{
    if (!surf || !box || !bg_style || bg_style->opacity < 3)
        return;

    /* 判断是否需要绘制，draw只在画圆角起作用 */
    ipgui_aabb_t draw;
    if (clip) {
        if (0 != ipgui_aabb_overlap(&draw, clip, &surf->surf))
            return;
    } else draw = surf->surf;

    ipgui_aabb_t padding_box;
    padding_box.start.x = box->start.x - (style ? style->left_padding   : 0);
    padding_box.start.y = box->start.y - (style ? style->top_padding    : 0);
    padding_box.end.x   = box->end.x   + (style ? style->right_padding  : 0);
    padding_box.end.y   = box->end.y   + (style ? style->bottom_padding : 0);

    if (0 != ipgui_aabb_overlap(&draw, &draw, &padding_box))
        return;

    /* 规整圆角半径，不超过padding box的半宽/半高的最小值 */
    ipgui_coord_t r_lt, r_rt, r_lb, r_rb;
    get_max_radius(&padding_box, style, &r_lt, &r_rt, &r_lb, &r_rb);

    ipgui_coord_t l_max = IPGUI_MAX(r_lt, r_lb);
    ipgui_coord_t r_max = IPGUI_MAX(r_rt, r_rb);
    ipgui_coord_t l_min = IPGUI_MIN(r_lt, r_lb);
    ipgui_coord_t r_min = IPGUI_MIN(r_rt, r_rb);

#define draw_without_corner(f) \
    ipgui_blend(surf, clip, &(f), &(bg_style->paint), \
        bg_style->opacity, (u8_t *)0, (ipgui_aabb_t *)0, bg_style->blend_mode)

    ipgui_aabb_t fill; 

    /* part1: 中间竖条，贯穿全高，左右避开圆角列 */
    fill.start.x = padding_box.start.x + l_max;
    fill.end.x   = padding_box.end.x   - r_max;
    if (fill.end.x >= fill.start.x) {
        fill.start.y = padding_box.start.y;
        fill.end.y   = padding_box.end.y;
        draw_without_corner(fill);
    }

    /* part2: 左侧竖条，高度避开圆角列，宽度为左侧最小半径*/
    if (l_min > 0) {
        fill.start.y = padding_box.start.y + r_lt;
        fill.end.y   = padding_box.end.y   - r_lb;
        if (fill.end.y >= fill.start.y) {
            fill.start.x = padding_box.start.x;
            fill.end.x   = padding_box.start.x + l_min - 1;
            draw_without_corner(fill);
        }
    }

    /* part3: 左侧竖条，高度避开圆角列，宽度为左侧最大半径减最小半径，起始位置需要判断上下两个圆角哪个更大 */
    if (l_max > l_min) {
        if (r_lt >= r_lb) {
            /* 左上角更大：只避开顶部，底部可延伸到 padding_box 底边 */
            fill.start.y = padding_box.start.y + r_lt;
            fill.end.y   = padding_box.end.y;
        } else {
            /* 左下角更大：只避开底部，顶部可延伸到 padding_box 顶边 */
            fill.start.y = padding_box.start.y;
            fill.end.y   = padding_box.end.y - r_lb;
        }
        if (fill.end.y >= fill.start.y) {
            fill.start.x = padding_box.start.x + l_min;
            fill.end.x   = padding_box.start.x + l_max - 1;
            draw_without_corner(fill);
        }
    }

    /* part4: 右侧竖条，宽度为右侧两圆角半径的较小值（r_min） */
    if (r_min > 0) {
        fill.start.y = padding_box.start.y + r_rt;
        fill.end.y   = padding_box.end.y   - r_rb;
        if (fill.end.y >= fill.start.y) {
            fill.start.x = padding_box.end.x - r_min + 1;
            fill.end.x   = padding_box.end.x;
            draw_without_corner(fill);
        }
    }

    /* part5: 右侧竖条，宽度为r_max - r_min */
    if (r_max > r_min) {
        if (r_rt >= r_rb) {
            /* 右上角更大：只避开顶部 */
            fill.start.y = padding_box.start.y + r_rt;
            fill.end.y   = padding_box.end.y;
        } else {
            /* 右下角更大：只避开底部 */
            fill.start.y = padding_box.start.y;
            fill.end.y   = padding_box.end.y - r_rb;
        }
        if (fill.end.y >= fill.start.y) {
            fill.start.x = padding_box.end.x - r_max + 1;
            fill.end.x   = padding_box.end.x - r_min;
            draw_without_corner(fill);
        }
    }
#undef draw_without_corner

    /* 分别画四个角 */

    ipgui_aabb_t corner;
    ipgui_aabb_t cdraw;
    /* 左上角 */
    if (r_lt > 0) {
        corner.start.x = padding_box.start.x;
        corner.end.x   = padding_box.start.x + r_lt - 1;
        corner.start.y = padding_box.start.y;
        corner.end.y   = padding_box.start.y + r_lt - 1;
        if (0 == ipgui_aabb_overlap(&cdraw, &draw, &corner))
            draw_one_corner(surf, clip, &cdraw, &corner, bg_style, r_lt, 0, -1, 1);
    }

    /* 右上角 */
    if (r_rt > 0) {
        corner.start.x = padding_box.end.x - r_rt + 1;
        corner.end.x   = padding_box.end.x;
        corner.start.y = padding_box.start.y;
        corner.end.y   = padding_box.start.y + r_rt - 1;
        if (0 == ipgui_aabb_overlap(&cdraw, &draw, &corner))
            draw_one_corner(surf, clip, &cdraw, &corner, bg_style, r_rt, 0, 1, 1);
    }

    /* 左下角 */
    if (r_lb > 0) {
        corner.start.x = padding_box.start.x;
        corner.end.x   = padding_box.start.x + r_lb - 1;
        corner.start.y = padding_box.end.y - r_lb + 1;
        corner.end.y   = padding_box.end.y;
        if (0 == ipgui_aabb_overlap(&cdraw, &draw, &corner))
            draw_one_corner(surf, clip, &cdraw, &corner, bg_style, r_lb, 0, -1, 0);
    }

    /* 右下角 */
    if (r_rb > 0) {
        corner.start.x = padding_box.end.x - r_rb + 1;
        corner.end.x   = padding_box.end.x;
        corner.start.y = padding_box.end.y - r_rb + 1;
        corner.end.y   = padding_box.end.y;
        if (0 == ipgui_aabb_overlap(&cdraw, &draw, &corner))
            draw_one_corner(surf, clip, &cdraw, &corner, bg_style, r_rb, 0, 1, 0);
    }
}

__IPGUI_STATIC__ void draw_one_corner(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_aabb_t         * cdraw,
    ipgui_aabb_t         * corner,
    ipgui_box_bg_style_t * bg_style,
    ipgui_coord_t          er,
    ipgui_coord_t          ir,
    s8_t                   x_step,
    u8_t                   y_flip)
{
    ipgui_coord_t w, h;
    w = ipgui_aabb_width (cdraw);
    h = ipgui_aabb_height(cdraw);

    ipgui_coord_t res_h;
    u8_t * mbuf = ipgui_mask_buf_acquire(w, h, &res_h);
    if ((!mbuf) || (res_h == 0)) {
        ipgui_dbg_error("error: box background rasterization failed to acquire mask buffer\r\n");
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
            
            ipgui_fetch_ring_mask(circle_x_start, circle_y, 0, er, x_step, row_mask, w);
        }

        /* blend */
        mask_aabb.start.y = cdraw->start.y + drawn_h;
        mask_aabb.end.y   = mask_aabb.start.y + current_h - 1;

        ipgui_blend(surf, clip, &mask_aabb, &(bg_style->paint),
                    bg_style->opacity, mbuf, &mask_aabb, bg_style->blend_mode);

        drawn_h += current_h;
        h       -= current_h;
    }

    ipgui_mask_buf_free(mbuf);
}