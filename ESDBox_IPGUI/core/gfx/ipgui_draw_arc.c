#include "ipgui_draw_arc.h"
#include "ipgui_edge_mask.h"
#include "ipgui_ring_mask.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_debug.h"
#include "ipgui_math.h"

static void draw_round_cap(
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_arc_t        * arc,
    ipgui_arc_style_t  * style,
    ipgui_arc_angle_t    angle,
    edge_halfplane_dir_t dir);

static void generate_angle_edge(
    ipgui_arc_angle_t angle,
    ipgui_coord_t     cx,
    ipgui_coord_t     cy,
    ipgui_coord_t   * x2,
    ipgui_coord_t   * y2)
{
    s32_t sin_val, cos_val;
    
    sin_val = ipgui_sin(angle);
    cos_val = ipgui_cos(angle);

    * x2 = cx + (ipgui_coord_t)cos_val;
    * y2 = cy + (ipgui_coord_t)sin_val;
}

static void edge_clip_ring_mask_no_aa(
    ipgui_edge_param_t   * edge,
    edge_halfplane_dir_t   edge_dir,
    u8_t                 * mask,
    ipgui_aabb_t         * mask_aabb,
    ipgui_coord_t          mask_stride)
{
    if ((!edge->dx) || (!edge->dy)) return; /* 补丁 */

    ipgui_coord_t y = mask_aabb->start.y;
    ipgui_edge_mask_dsc_t dsc;
    u8_t * row_mask = mask;

    if (edge_dir == EDGE_HALFPLANE_DIR_LEFT) {
        for (; y <= mask_aabb->end.y; y ++) {
            ipgui_gen_edge_halfplane_mask(&dsc, edge_dir, edge, y);

            /* * LEFT 方向：保留左侧，抹除右侧。
             * dsc.x_start 是分界线点。
             */
            if (mask_aabb->end.x > dsc.x_start) {
                // 计算需要清除的起始偏移（相对于行掩码起始点）
                ipgui_coord_t clear_start = IPGUI_MAX(0, dsc.x_start + 1 - mask_aabb->start.x);
                if (clear_start < mask_stride) {
                    ipgui_memset(row_mask + clear_start, 0, mask_stride - clear_start);
                }
            }
            row_mask += mask_stride;
        }
    } else if (edge_dir == EDGE_HALFPLANE_DIR_RIGHT) {
        ipgui_coord_t mask_len_tmp;
        for (; y <= mask_aabb->end.y; y ++) {
            mask_len_tmp = mask_stride;
            ipgui_gen_edge_halfplane_mask(&dsc, edge_dir, edge, y);

            /* clip mask no aa */
            if (mask_aabb->end.x >= (dsc.x_start + 1)) {
                ipgui_coord_t mask_255_len;
                mask_255_len = IPGUI_MIN(mask_stride, mask_aabb->end.x - dsc.x_start);
                /* drop right */
                mask_len_tmp -= mask_255_len;
                if (mask_len_tmp == 0) {
                    goto _next_row;
                }
            }
            /* set 0 before */
            ipgui_memset(row_mask, 0, mask_len_tmp);
_next_row:
            row_mask += mask_stride;
        }
    }
}

static void edge_clip_ring_mask_with_aa(
    ipgui_edge_param_t   * edge,
    edge_halfplane_dir_t   edge_dir,
    u8_t                 * mask,
    ipgui_aabb_t         * mask_aabb,
    ipgui_coord_t          mask_stride)
{
    if ((!edge->dx) || (!edge->dy)) return; /* 补丁 */

    ipgui_coord_t y = mask_aabb->start.y;
    ipgui_edge_mask_dsc_t dsc;
    u8_t * row_mask = mask;

    if (edge_dir == EDGE_HALFPLANE_DIR_LEFT) {
        u8_t          m;
        for (; y <= mask_aabb->end.y; y ++) {
            ipgui_gen_edge_halfplane_mask(&dsc, edge_dir, edge, y);

            /* 跳过左侧全255区域 */
            ipgui_coord_t skip_255_len = 0;
            if (mask_aabb->start.x <= (dsc.x_start - 1)) {
                skip_255_len = IPGUI_MIN(mask_stride, dsc.x_start - mask_aabb->start.x);
            }

            /* 从第一个可能不是255的点开始处理 */
            ipgui_coord_t mask_idx = skip_255_len;
            ipgui_coord_t mask_len_rem = mask_stride - skip_255_len;
            ipgui_coord_t mask_mix_x = mask_aabb->start.x + mask_idx;

            while (mask_len_rem > 0) {
                m = ipgui_edge_mask(&dsc, mask_mix_x);
                if (!m) {
                    /* 进入全0区，右侧剩余部分全部抹除 */
                    ipgui_memset(row_mask + mask_idx, 0, mask_len_rem);
                    break;
                } else {
                    row_mask[mask_idx] = ((u16_t)m * row_mask[mask_idx] + 255) >> 8;
                }
                mask_idx ++;
                mask_mix_x ++;
                mask_len_rem --;
            }

            row_mask += mask_stride;
        }
    } else if (edge_dir == EDGE_HALFPLANE_DIR_RIGHT) {
        ipgui_coord_t mask_len_tmp;
        ipgui_coord_t mask_idx;
        ipgui_coord_t mask_mix_x;
        u8_t          m;
        for (; y <= mask_aabb->end.y; y ++) {
            mask_len_tmp = mask_stride;
            ipgui_gen_edge_halfplane_mask(&dsc, edge_dir, edge, y);

            /* clip mask with aa */
            if (mask_aabb->end.x >= (dsc.x_start + 1)) {
                ipgui_coord_t mask_255_len;
                mask_255_len = IPGUI_MIN(mask_stride, mask_aabb->end.x - dsc.x_start);
                /* drop right */
                mask_len_tmp -= mask_255_len;
                if (mask_len_tmp == 0) {
                    goto next_row;
                }
            }
            mask_idx = mask_len_tmp - 1;
            mask_mix_x = mask_aabb->start.x + mask_idx;  /* 不能用mask_mix_x = dsc.x_start;因为整行mask都可能在x_start左边 */
            while (mask_len_tmp) {
                m = ipgui_edge_mask(&dsc, mask_mix_x);
                if (!m) {
                    /* set 0 before */
                    ipgui_memset(row_mask, 0, mask_len_tmp);
                    goto next_row;
                } else {
                    row_mask[mask_idx] = ((u16_t)m * row_mask[mask_idx] + 255) >> 8;
                }
                mask_len_tmp --;
                mask_idx --;
                mask_mix_x --;
            }
next_row:
            row_mask += mask_stride;
        }
    }
}

__IPGUI_STATIC__ void draw_arc_corner(
    ipgui_surf_t      * surf,
    ipgui_aabb_t      * clip,
    ipgui_aabb_t      * cdraw,
    ipgui_aabb_t      * corner,
    ipgui_arc_style_t * arc_style,
    ipgui_coord_t       er,
    ipgui_coord_t       ir,
    s8_t                x_step,
    u8_t                y_flip,
    int                 quarter,
    ipgui_arc_angle_t   os,
    ipgui_arc_angle_t   oe)
{
    ipgui_coord_t w, h;
    w = ipgui_aabb_width (cdraw);
    h = ipgui_aabb_height(cdraw);   

    ipgui_coord_t res_h;
    u8_t * mbuf = ipgui_mask_buf_acquire(w, h, &res_h);
    if ((!mbuf) || (res_h == 0)) {
        ipgui_dbg_error("error: arc corner rasterization failed to acquire mask buffer\r\n");
        return;
    }

    edge_halfplane_dir_t os_dir;
    edge_halfplane_dir_t oe_dir;
    ipgui_coord_t cx, cy, x2, y2;
    if (quarter == 1) {
        os_dir = EDGE_HALFPLANE_DIR_LEFT;
        oe_dir = EDGE_HALFPLANE_DIR_RIGHT;
        cx = corner->start.x;
        cy = corner->start.y;
    } else if (quarter == 2) {
        os_dir = EDGE_HALFPLANE_DIR_LEFT;
        oe_dir = EDGE_HALFPLANE_DIR_RIGHT;
        cx = corner->end.x;
        cy = corner->start.y;
    } else if (quarter == 3) {
        os_dir = EDGE_HALFPLANE_DIR_RIGHT;
        oe_dir = EDGE_HALFPLANE_DIR_LEFT;
        cx = corner->end.x;
        cy = corner->end.y;
    } else if (quarter == 4) {
        os_dir = EDGE_HALFPLANE_DIR_RIGHT;
        oe_dir = EDGE_HALFPLANE_DIR_LEFT;
        cx = corner->start.x;
        cy = corner->end.y;
    }

    ipgui_edge_param_t os_edge_param;
    ipgui_edge_param_t oe_edge_param;
    generate_angle_edge(os, cx, cy, &x2, &y2); os_edge_param = ipgui_edge_param_init(cx * 64, cy * 64, x2 * 64, y2 * 64);
    generate_angle_edge(oe, cx, cy, &x2, &y2); oe_edge_param = ipgui_edge_param_init(cx * 64, cy * 64, x2 * 64, y2 * 64);

    ipgui_coord_t drawn_h = 0;

    ipgui_aabb_t mask_aabb;
    mask_aabb.start.x = cdraw->start.x;
    mask_aabb.end.x   = cdraw->end.x;
    
    ipgui_coord_t circle_x_start = (x_step == -1) ?
                (corner->end.x + 1 - cdraw->start.x) :
                (cdraw->start.x - corner->start.x + 1);
    while (h > 0) {
        ipgui_coord_t current_h = IPGUI_MIN(h, res_h);
        ipgui_coord_t draw_y, circle_y;
        u8_t * row_mask;

        for (int i = 0; i < current_h; i ++) {
            draw_y = cdraw->start.y + drawn_h + i;
            circle_y = y_flip ? 
                (corner->end.y + 1 - draw_y) : 
                (draw_y - corner->start.y + 1);

            row_mask = mbuf + i * w;

            ipgui_fetch_ring_mask(circle_x_start, circle_y, ir, er, x_step, row_mask, w);
        }

        mask_aabb.start.y = cdraw->start.y + drawn_h;
        mask_aabb.end.y   = mask_aabb.start.y + current_h - 1;

        if (arc_style->sep_type == IPGUI_ARC_ENDPOINT_TYPE_BUTT) {
            edge_clip_ring_mask_with_aa(&os_edge_param, os_dir, mbuf, &mask_aabb, w);
        } else {
            edge_clip_ring_mask_no_aa(&os_edge_param, os_dir, mbuf, &mask_aabb, w);
        }

        if (arc_style->eep_type == IPGUI_ARC_ENDPOINT_TYPE_BUTT) {
            edge_clip_ring_mask_with_aa(&oe_edge_param, oe_dir, mbuf, &mask_aabb, w);
        } else {
            edge_clip_ring_mask_no_aa(&oe_edge_param, oe_dir, mbuf, &mask_aabb, w);
        }

        ipgui_blend(surf, clip, &mask_aabb, &(arc_style->paint),
                    arc_style->opacity, mbuf, &mask_aabb, arc_style->blend_mode);

        drawn_h += current_h;
        h       -= current_h;
    }

    ipgui_mask_buf_free(mbuf);
}

void draw_quarter(
    ipgui_surf_t      * surf, 
    ipgui_aabb_t      * draw, 
    ipgui_arc_t       * arc, 
    ipgui_arc_style_t * style, 
    int                 quarter,
    ipgui_arc_angle_t   os, 
    ipgui_arc_angle_t   oe)
{
    ipgui_aabb_t cdraw, corner;
    s8_t         x_step;
    u8_t         y_flip;
    if (quarter == 1) {
        cdraw.start.x = arc->cx;
        cdraw.end.x   = arc->cx + arc->er - 1;
        cdraw.start.y = arc->cy;
        cdraw.end.y   = arc->cy + arc->er - 1;
        x_step = 1;
        y_flip = 0;
    } else if (quarter == 2) {
        cdraw.start.x = arc->cx - arc->er;
        cdraw.end.x   = arc->cx - 1;
        cdraw.start.y = arc->cy;
        cdraw.end.y   = arc->cy + arc->er - 1;
        x_step = -1;
        y_flip = 0;
    } else if (quarter == 3) {
        cdraw.start.x = arc->cx - arc->er;
        cdraw.end.x   = arc->cx - 1;
        cdraw.start.y = arc->cy - arc->er;
        cdraw.end.y   = arc->cy - 1;
        x_step = -1;
        y_flip = 1;
    } else if (quarter == 4) {
        cdraw.start.x = arc->cx;
        cdraw.end.x   = arc->cx + arc->er - 1;
        cdraw.start.y = arc->cy - arc->er;
        cdraw.end.y   = arc->cy - 1;
        x_step = 1;
        y_flip = 1;
    }
    corner = cdraw;

    if (0 != ipgui_aabb_overlap(&cdraw, draw, &cdraw))
        return;

    draw_arc_corner(
        surf,
        (ipgui_aabb_t *)0,
        &cdraw,
        &corner,
        style,
        arc->er,
        arc->ir,
        x_step,
        y_flip,
        quarter,
        os,
        oe);
}

__IPGUI_API__ void ipgui_draw_arc(
    ipgui_surf_t      * surf,
    ipgui_aabb_t      * clip,
    ipgui_arc_t       * arc,
    ipgui_arc_style_t * style)
{
    if ((!surf) || (!arc) || (!style))
        return;

    /* check arc geometry parameters */
    if (arc->angle == 0) return;
    if ((arc->er <= 0) || (arc->ir < 0) || (arc->ir >= arc->er)) return;

    /* check arc style parameters */
    if (style->opacity < 3) return;

    ipgui_aabb_t draw;
    if (clip) {
        if (0 != ipgui_aabb_overlap(&draw, clip, &surf->surf))
            return;
    } else draw = surf->surf;

    ipgui_aabb_t self;
    /* self是外圆的包围盒
     * 如果以(0,0)为中心，半径为10画圆弧，那么
     * self的x范围是-10 - 9（-10~~1和0~9）
     * self的y范围是-10 - 9（-10~-1和0~9）
     */
    self.start.x = arc->cx - arc->er;
    self.end.x   = arc->cx + arc->er - 1;
    self.start.y = arc->cy - arc->er;
    self.end.y   = arc->cy + arc->er - 1;
    if (0 != ipgui_aabb_overlap(&draw, &self, &draw))
        return;

    u16_t arc_angle = arc->angle;
    if (arc_angle > 359) {
        arc_angle = 360;
    }

    /* normalize angle to 0 - 360 */
    ipgui_arc_angle_t start_angle = arc->start;
    while (start_angle < 0)    start_angle += 360;
    while (start_angle >= 360) start_angle -= 360;
    
    /* 统一转换为 CCW 线性区间 [start_angle, end_angle][0~720] */
    ipgui_arc_angle_t end_angle;
    if (arc->dir == IPGUI_ARC_DRAW_DIR_CW) {
        end_angle = start_angle;
        start_angle = start_angle - arc_angle;
        while (start_angle < 0) {
            start_angle += 360;
            end_angle   += 360;
        }
    } else {
        end_angle = start_angle + arc_angle;
    }

    // ipgui_dbg_info("draw arc ccw: [%d, %d]\n", start_angle, end_angle);/* 这里得到了逆时针绘制的起始和终止角度 */

    ipgui_arc_angle_t cur = start_angle, b = 90;
    while (b <= start_angle) b += 90;

    while (cur < end_angle) {
        ipgui_arc_angle_t next = (end_angle < b) ? end_angle : b;

        /* 映射到0-360 */
        ipgui_arc_angle_t os = cur;
        while (os >= 360) os -= 360;

        ipgui_arc_angle_t oe = next;
        while (oe >= 360) oe -= 360;

        if (oe == 0 && next > cur)
            oe = 360;

        int quarter = (os < 90) ? 1 : (os < 180 ? 2 : (os < 270 ? 3 : 4));
        // ipgui_dbg_info("Q%d: [%d, %d]\n", quarter, os, oe);/* 这里得到了象限和角度 */

        draw_quarter(surf, &draw, arc, style, quarter, os, oe);

        cur = next;
        b += 90;
    }

    if (style->sep_type == IPGUI_ARC_ENDPOINT_TYPE_ROUND) {
        /* 在端点处补半圆 */
        draw_round_cap(surf, &draw, arc, style, start_angle, EDGE_HALFPLANE_DIR_RIGHT);
    }
    if (style->eep_type == IPGUI_ARC_ENDPOINT_TYPE_ROUND) {
        /* 在端点处补半圆 */
        draw_round_cap(surf, &draw, arc, style, end_angle,   EDGE_HALFPLANE_DIR_LEFT );
    }
}

static void draw_round_cap(
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_arc_t        * arc,
    ipgui_arc_style_t  * style,
    ipgui_arc_angle_t    angle,
    edge_halfplane_dir_t dir)
{
    ipgui_coord_t r_mid  = (arc->er + arc->ir + 1) >> 1;
    ipgui_coord_t radius = (arc->er - arc->ir + 1) >> 1;
    s32_t sin_val = ipgui_sin(angle);
    s32_t cos_val = ipgui_cos(angle);

    ipgui_coord_t cap_cx = arc->cx + ((s32_t)r_mid * cos_val + 16384 >> 15);
    ipgui_coord_t cap_cy = arc->cy + ((s32_t)r_mid * sin_val + 16384 >> 15);

    /* 确定端点圆的aabb */
    ipgui_aabb_t cap_aabb;
    cap_aabb.start.x = cap_cx - radius;
    cap_aabb.end.x   = cap_cx + radius - 1;
    cap_aabb.start.y = cap_cy - radius;
    cap_aabb.end.y   = cap_cy + radius - 1;

    if (0 != ipgui_aabb_overlap(&cap_aabb, &cap_aabb, clip)) return;

    ipgui_arc_t circle;
    circle.cx = cap_cx;
    circle.cy = cap_cy;
    circle.dir = IPGUI_ARC_DRAW_DIR_CCW;
    circle.er = radius;
    circle.ir = 0;
    circle.start = 0;
    circle.angle = 360;

    ipgui_arc_style_t circle_style;
    circle_style.blend_mode = 0;
    circle_style.sep_type = IPGUI_ARC_ENDPOINT_TYPE_BUTT;
    circle_style.eep_type = IPGUI_ARC_ENDPOINT_TYPE_BUTT;
    circle_style.opacity  = style->opacity;
    circle_style.paint    = style->paint;
    ipgui_draw_arc(surf, &cap_aabb, &circle, &circle_style);
}