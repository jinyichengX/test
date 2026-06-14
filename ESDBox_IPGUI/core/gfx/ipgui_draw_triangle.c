#include "ipgui_draw_triangle.h"
#include "ipgui_edge_halfplane_mask.h"
#include "ipgui_mask_buf.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

/* 根据y值从小到大对三点进行排序 */
__IPGUI_STATIC__ void ipgui_sort3p_by_y(
    ipgui_point_t  * p1,
    ipgui_point_t  * p2,
    ipgui_point_t  * p3,
    ipgui_point_t ** sort_arr)
{
    if (p1->y <= p2->y && p1->y <= p3->y) {
        sort_arr[0] = p1;
        if (p2->y <= p3->y) {
            sort_arr[1] = p2;
            sort_arr[2] = p3;
        } else {
            sort_arr[1] = p3;
            sort_arr[2] = p2;
        }
    } else if (p2->y <= p1->y && p2->y <= p3->y) {
        sort_arr[0] = p2;
        if (p1->y <= p3->y) {
            sort_arr[1] = p1;
            sort_arr[2] = p3;
        } else {
            sort_arr[1] = p3;
            sort_arr[2] = p1;
        }
    } else {
        sort_arr[0] = p3;
        if (p1->y <= p2->y) {
            sort_arr[1] = p1;
            sort_arr[2] = p2;
        } else {
            sort_arr[1] = p2;
            sort_arr[2] = p1;
        }
    }
}

__IPGUI_STATIC__ void fill_mask_two_edges(
    u8_t                  * mask, 
    ipgui_coord_t           sx, 
    ipgui_coord_t           len,
    ipgui_edge_halfplane_mask_dsc_t * right, /* right's halfspan must be right */
    ipgui_edge_halfplane_mask_dsc_t * left)  /* left's  halfspan must be left   */
{
    ipgui_coord_t lx, rx;
    lx = right->x_start + 1; /* the first point which mask is 255 */
    rx = left->x_start  - 1; /* the first point which mask is 255 */

    /* 左侧抗锯齿区 */
    if (sx < lx) {
        ipgui_coord_t mask_end_x = sx + len - 1;
        ipgui_coord_t x = right->x_start;
        ipgui_coord_t left_mask_len = lx - sx;
        x = IPGUI_MIN(x, mask_end_x);
        left_mask_len = IPGUI_MIN(left_mask_len, len);
        ipgui_coord_t mask_idx = x - sx;
        while(x >= sx) {
            mask[mask_idx] = ipgui_edge_halfplane_mask(right, x);
            if(!mask[mask_idx])
                break;
            x --;
            mask_idx --;
        }
        if (x >= sx)
            ipgui_memset(mask, 0, x - sx);

        len  -= left_mask_len;
        if(!len) return;
        mask += left_mask_len;
        sx   += left_mask_len;
    }

    /* 中间 */
    if (rx >= lx) {
        if (sx <= rx) {
            ipgui_coord_t mid_mask_len = rx - sx + 1;
            mid_mask_len = IPGUI_MIN(mid_mask_len, len);
            ipgui_memset(mask, 255, mid_mask_len);

            len  -= mid_mask_len;
            if(!len) return;
            mask += mid_mask_len;
            sx   += mid_mask_len;
        }
    }

    /* 右侧抗锯齿区 */
    ipgui_coord_t mask_idx = 0;
    while (mask_idx < len) {
        u8_t m = ipgui_edge_halfplane_mask(left, sx + mask_idx);
        mask[mask_idx] = m;
        
        if (m == 0) {
            if (mask_idx < len - 1) {
                ipgui_memset(mask + mask_idx + 1, 0, len - mask_idx - 1);
            }
            break;
        }
        mask_idx ++;
    }
}

__IPGUI_API__ void ipgui_draw_triangle(
    ipgui_surf_t           * surf, 
    ipgui_aabb_t           * clip,
    ipgui_point_t          * p1, 
    ipgui_point_t          * p2, 
    ipgui_point_t          * p3,
    ipgui_triangle_style_t * style)
{
    if (!surf || !style || style->opacity < 3)
        return;

    if ((!p1) || (!p2) || (!p3)) return;
    
    ipgui_aabb_t draw;
    if (clip) {
        if (0 != ipgui_aabb_overlap(&draw, clip, &surf->surf))
            return;
    } else draw = surf->surf;

    ipgui_aabb_t self;
    self.start.x = IPGUI_MIN3(p1->x, p2->x, p3->x);
    self.end.x   = IPGUI_MAX3(p1->x, p2->x, p3->x);
    self.start.y = IPGUI_MIN3(p1->y, p2->y, p3->y);
    self.end.y   = IPGUI_MAX3(p1->y, p2->y, p3->y);
    if (0 != ipgui_aabb_overlap(&draw, &self, &draw))
        return;

    /* alloc mask buffer */
    ipgui_coord_t w, h;
    ipgui_coord_t res_h;
    w = ipgui_aabb_width (&draw);
    h = ipgui_aabb_height(&draw);
    u8_t * mask = ipgui_mask_buf_acquire(w, h, &res_h);
    if ((!res_h) || (!mask)) {
        ipgui_dbg_error("error: triangle rasterization failed to acquire mask buffer\r\n");
        return;
    }

    ipgui_point_t * pa[3];
    ipgui_sort3p_by_y(p1, p2, p3, pa); 

    /* 初始化长边 (长边是y跨度最大的边pa[0]->pa[2]，而不是视觉上最长的边) */
    ipgui_edge_param_t e_long;
    e_long = ipgui_edge_param_init(pa[0]->x * 64, pa[0]->y * 64, pa[2]->x * 64, pa[2]->y * 64);

    /* 投影点判定法确定左右关系 */
    ipgui_edge_coord_t x_at_m = edge_x_at_y(&e_long, pa[1]->y);
    edge_halfplane_dir_t e_long_dir;  
    edge_halfplane_dir_t e_short_dir; 

    if ((pa[1]->x * 64) < x_at_m) {
        e_long_dir  = EDGE_HALFPLANE_DIR_LEFT;
        e_short_dir = EDGE_HALFPLANE_DIR_RIGHT;
    } else {
        e_long_dir  = EDGE_HALFPLANE_DIR_RIGHT;
        e_short_dir = EDGE_HALFPLANE_DIR_LEFT;
    }

    /* 光栅化下三角形 (pa[0] -> pa[1]) */
    if (pa[1]->y > pa[0]->y) {
        ipgui_aabb_t bottom_tri = draw;
        ipgui_coord_t end_y = pa[1]->y - 1;
        if (end_y < bottom_tri.start.y)
            goto _ras_top_tri;
        if (end_y< bottom_tri.end.y)
            bottom_tri.end.y = end_y;

        ipgui_coord_t tri_h = ipgui_aabb_height(&bottom_tri);
        ipgui_edge_param_t e1 = ipgui_edge_param_init(pa[0]->x * 64, pa[0]->y * 64, pa[1]->x * 64, pa[1]->y * 64);
        
        ipgui_aabb_t mask_aabb;
        mask_aabb.start.x = bottom_tri.start.x;
        mask_aabb.end.x   = bottom_tri.end.x;

        ipgui_edge_halfplane_mask_dsc_t em1, em2;
        ipgui_coord_t y = bottom_tri.start.y;
        ipgui_edge_halfplane_mask_dsc_t * e_left, * e_right;
        if (e_long_dir == EDGE_HALFPLANE_DIR_LEFT) {
            e_left  = &em2;
            e_right = &em1;
        } else if (e_long_dir == EDGE_HALFPLANE_DIR_RIGHT) {
            e_left  = &em1; 
            e_right = &em2;
        }
        while (tri_h > 0) {
            ipgui_coord_t current_h = IPGUI_MIN(tri_h, res_h);
            mask_aabb.start.y = y;
            mask_aabb.end.y   = y + current_h - 1;

            for (; y <= mask_aabb.end.y; y ++) {
                ipgui_gen_edge_halfplane_mask_dsc(&em1, e_short_dir, &e1,     y);
                ipgui_gen_edge_halfplane_mask_dsc(&em2, e_long_dir,  &e_long, y);

                u8_t * mask_buf = mask + w * (y - mask_aabb.start.y);
#if 0
                /* 逐像素遍历 */
                for (int j = 0; j < w; j ++) {
                    mask_buf[j] = ((u32_t)ipgui_edge_halfplane_mask(&em1, mask_aabb.start.x + j) * ipgui_edge_halfplane_mask(&em2, mask_aabb.start.x + j) + 255) >> 8;
                }
#else
                fill_mask_two_edges(mask_buf, mask_aabb.start.x, w, e_right, e_left);
#endif
            }
            ipgui_blend(surf, (ipgui_aabb_t *)0, &mask_aabb, &style->paint, style->opacity, mask, &mask_aabb, style->blend_mode);
            tri_h -= current_h;
        }
    }

_ras_top_tri:
    /* 光栅化上三角形 (pa[1] -> pa[2]) */
    if (pa[2]->y > pa[1]->y) {
        ipgui_aabb_t top_tri = draw;
        ipgui_coord_t start_y = pa[1]->y + 1;
        if (start_y > top_tri.end.y) goto _ras_mid_line;
        if (start_y > top_tri.start.y) top_tri.start.y = start_y;

        ipgui_coord_t tri_h = ipgui_aabb_height(&top_tri);
        if (tri_h <= 0) goto _ras_mid_line;

        ipgui_edge_param_t e1 = ipgui_edge_param_init(pa[1]->x * 64, pa[1]->y * 64, pa[2]->x * 64, pa[2]->y * 64);
        ipgui_aabb_t mask_aabb;
        mask_aabb.start.x = top_tri.start.x;
        mask_aabb.end.x   = top_tri.end.x;

        ipgui_edge_halfplane_mask_dsc_t em1, em2;
        ipgui_coord_t y = top_tri.start.y;
        ipgui_edge_halfplane_mask_dsc_t * e_left, * e_right;
        if (e_long_dir == EDGE_HALFPLANE_DIR_LEFT) {
            e_left  = &em2;
            e_right = &em1;
        } else if (e_long_dir == EDGE_HALFPLANE_DIR_RIGHT) {
            e_left  = &em1;
            e_right = &em2;
        }
        while (tri_h > 0) {
            ipgui_coord_t current_h = IPGUI_MIN(tri_h, res_h);
            mask_aabb.start.y = y;
            mask_aabb.end.y   = y + current_h - 1;

            for (; y <= mask_aabb.end.y; y ++) {
                ipgui_gen_edge_halfplane_mask_dsc(&em1, e_short_dir, &e1,     y);
                ipgui_gen_edge_halfplane_mask_dsc(&em2, e_long_dir,  &e_long, y);

                u8_t * mask_buf = mask + w * (y - mask_aabb.start.y);
#if 0
                /* 逐像素遍历 */
                for (int j = 0; j < w; j ++) {
                    mask_buf[j] = ((u32_t)ipgui_edge_halfplane_mask(&em1, mask_aabb.start.x + j) * ipgui_edge_halfplane_mask(&em2, mask_aabb.start.x + j) + 255) >> 8;
                }
#else         
                fill_mask_two_edges(mask_buf, mask_aabb.start.x, w, e_right, e_left);
#endif
            }
            ipgui_blend(surf, (ipgui_aabb_t *)0, &mask_aabb, &style->paint, style->opacity, mask, &mask_aabb, style->blend_mode);
            tri_h -= current_h;
        }
    }

_ras_mid_line:
    /* 这里单独渲染pa[1].y所在的行 */
    if (pa[1]->y >= draw.start.y && pa[1]->y <= draw.end.y) {
        ipgui_aabb_t mask_aabb;
        mask_aabb.start.x = draw.start.x;
        mask_aabb.end.x   = draw.end.x;
        mask_aabb.start.y = pa[1]->y;
        mask_aabb.end.y   = pa[1]->y;
        
        ipgui_edge_param_t e_short1 = ipgui_edge_param_init(pa[0]->x * 64, pa[0]->y * 64, pa[1]->x * 64, pa[1]->y * 64);
        ipgui_edge_param_t e_short2 = ipgui_edge_param_init(pa[1]->x * 64, pa[1]->y * 64, pa[2]->x * 64, pa[2]->y * 64);

        ipgui_edge_halfplane_mask_dsc_t em1, em2, em3;
        ipgui_gen_edge_halfplane_mask_dsc(&em1, e_short_dir, &e_short1, mask_aabb.start.y);
        ipgui_gen_edge_halfplane_mask_dsc(&em3, e_short_dir, &e_short2, mask_aabb.start.y);
        ipgui_gen_edge_halfplane_mask_dsc(&em2, e_long_dir,  &e_long,   mask_aabb.start.y);

        u8_t * mask_buf = mask; 
        u8_t m1, m2, m3;
        
        for (s32_t j = 0; j < w; j ++) {
            m1 = ipgui_edge_halfplane_mask(&em1, mask_aabb.start.x + j);
            m2 = ipgui_edge_halfplane_mask(&em2, mask_aabb.start.x + j);
            m3 = ipgui_edge_halfplane_mask(&em3, mask_aabb.start.x + j);
            mask_buf[j] = ((u32_t)m1 * m2 * m3 + 65535) >> 16;
        }
        ipgui_blend(surf, (ipgui_aabb_t *)0, &mask_aabb, &style->paint, style->opacity, mask, &mask_aabb, style->blend_mode);
    }

_ras_done:
    ipgui_mask_buf_free(mask);
}