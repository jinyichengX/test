#include "ipgui_draw_box_shadow.h"
#include "ipgui_draw_pixel.h"
#include <math.h>

/* 移植自 pandagl gradient_t */
typedef struct {
    float s;
    float v;
    float a;
} shadow_gradient_t;

static void gradient_init(shadow_gradient_t * g, float blur_w)
{
    g->s = 255.0f;
    g->v = 512.0f / blur_w;
    g->a = 2.0f * (g->v * blur_w - g->s) / (blur_w * blur_w);
}

static u8_t gradient_compute(shadow_gradient_t * g, float t)
{
    float val = g->s - (g->v * t - g->a * t * t * 0.5f);
    if (val < 0)   return 0;
    if (val > 255) return 255;
    return (u8_t)val;
}

/* 圆角矩形 SDF，返回有符号距离：负=内部，正=外部 */
static float rounded_rect_sdf(
    float lx, float ly,
    float w,  float h, float r)
{
    float qx = IPGUI_ABS(lx - w * 0.5f) - w * 0.5f + r;
    float qy = IPGUI_ABS(ly - h * 0.5f) - h * 0.5f + r;
    float ox = qx > 0 ? qx : 0;
    float oy = qy > 0 ? qy : 0;
    return sqrtf(ox * ox + oy * oy) - r;
}

__IPGUI_API__ void ipgui_draw_box_shadow(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_aabb_t         * content_box,
    ipgui_box_shadow_style_t * style)
{
    if (!surf || !content_box || !style) return;
    if (style->opacity < 3) return;

    ipgui_coord_t blur   = style->blur;
    ipgui_coord_t spread = style->spread;
    ipgui_coord_t ox     = style->offset_x;
    ipgui_coord_t oy     = style->offset_y;
    ipgui_coord_t cr     = style->corner_radius;

    /* blur_w：实际模糊带宽度，对应 pandagl 的 BLUR_WIDTH = blur * 1.5 */
    float blur_w = blur * 1.5f;
    if (blur_w < 1.0f) blur_w = 1.0f;

    /* content box 尺寸 */
    ipgui_coord_t bx = content_box->start.x;
    ipgui_coord_t by = content_box->start.y;
    ipgui_coord_t bw = ipgui_aabb_width(content_box);
    ipgui_coord_t bh = ipgui_aabb_height(content_box);

    /* shadow box = content box 向外 spread，再偏移 ox/oy */
    ipgui_coord_t sw_x = bx - spread + ox;
    ipgui_coord_t sw_y = by - spread + oy;
    ipgui_coord_t sw_w = bw + spread * 2;
    ipgui_coord_t sw_h = bh + spread * 2;
    ipgui_coord_t sw_r = cr + spread;
    sw_r = IPGUI_MIN(sw_r, sw_w / 2);
    sw_r = IPGUI_MIN(sw_r, sw_h / 2);
    sw_r = IPGUI_MAX(sw_r, 0);

    ipgui_coord_t box_r = IPGUI_MIN(cr, bw / 2);
    box_r = IPGUI_MIN(box_r, bh / 2);
    box_r = IPGUI_MAX(box_r, 0);

    /* 遍历范围：shadow box + blur_w 四周 */
    ipgui_coord_t pad  = (ipgui_coord_t)blur_w + 2;
    ipgui_coord_t x0   = sw_x - pad;
    ipgui_coord_t y0   = sw_y - pad;
    ipgui_coord_t x1   = sw_x + sw_w + pad;
    ipgui_coord_t y1   = sw_y + sw_h + pad;

    shadow_gradient_t g;
    gradient_init(&g, blur_w);

    ipgui_coord_t x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++) {

            /* ── step1: 计算该像素到 shadow box 的距离 ── */
            float lx   = (float)(x - sw_x);
            float ly   = (float)(y - sw_y);
            float dist = rounded_rect_sdf(lx, ly, (float)sw_w, (float)sw_h, (float)sw_r);

            u8_t shadow_mask = 0;
            if (dist <= 0.0f) {
                shadow_mask = 255;                        /* 内部：实色 */
            } else if (dist < blur_w) {
                shadow_mask = gradient_compute(&g, dist); /* 模糊边缘 */
            } else {
                continue;                                  /* 完全透明，跳过 */
            }

            /* ── step2: 挖掉 content box（对应 pd_clear_boxshadow_content_rect）── */
            float clx  = (float)(x - bx);
            float cly  = (float)(y - by);
            float cdist = rounded_rect_sdf(clx, cly, (float)bw, (float)bh, (float)box_r);
            if (cdist <= 0.0f) {
                continue; /* content box 内部，不画阴影 */
            }
            /* content box 边缘抗锯齿：0~1像素过渡 */
            if (cdist < 1.0f) {
                u8_t edge = (u8_t)(cdist * 255.0f);
                shadow_mask = (u8_t)(((u32_t)shadow_mask * edge) >> 8);
                if (shadow_mask < 2) continue;
            }

            /* ── step3: 调用 ipgui_draw_pixel 写入 surf ── */
            ipgui_draw_pixel(
                surf, clip,
                x, y,
                style->color,
                shadow_mask,
                style->opacity,
                style->blend_mode);
        }
    }
}
// #include "ipgui_draw_box_shadow.h"
// #include "ipgui_mask_buf.h"
// #include "ipgui_memory.h"   // 确保引入了 ipgui_memset 和 ipgui_memcpy
// #include "ipgui_debug.h"

// /* 声明外部共享的圆角规整化函数（位于 ipgui_draw_box_background.c 中） */
// extern void get_max_radius(
//     ipgui_aabb_t * padding_box, ipgui_box_style_t * style,
//     ipgui_coord_t * r_lt, ipgui_coord_t * r_rt,
//     ipgui_coord_t * r_lb, ipgui_coord_t * r_rb);

// /**
//  * @brief 高速纯整数开方算法（避免依赖硬件 FPU）
//  */
// __IPGUI_STATIC__ u32_t ipgui_sqrt32(u32_t n) {
//     u32_t res = 0;
//     u32_t bit = 1 << 30;
//     while (bit > n) bit >>= 2;
//     while (bit != 0) {
//         if (n >= res + bit) {
//             n -= res + bit;
//             res = (res >> 1) + bit;
//         } else {
//             res >>= 1;
//         }
//         bit >>= 2;
//     }
//     return res;
// }

// /**
//  * @brief 1D 模糊剖面函数（采用高品质定点数 Smoothstep 拟合双侧高斯分布曲线）
//  * @param dist 距离阴影实体边界的绝对距离（负数在内部，正数在外部）
//  * @param blur 模糊半径
//  * @return u8_t 遮罩灰度值 (0 ~ 255)
//  */
// __IPGUI_STATIC__ __IPGUI_INLINE__ u8_t ipgui_get_shadow_blur_mask(s32_t dist, s32_t blur) {
//     if (blur <= 0) return (dist < 0) ? 255 : 0;
//     if (dist <= -blur) return 255;
//     if (dist >= blur)  return 0;

//     // 将距离区间 [-blur, blur] 线性映射到 [0, 4096] (Q12 定点数)
//     s32_t f = ((blur - dist) << 12) / (2 * blur);
    
//     // 执行 Smoothstep 三次多项式插值: 3f^2 - 2f^3
//     s32_t f2 = (f * f) >> 12; 
//     s32_t mask = (f2 * (3 * 4096 - 2 * f)) >> 16; // 缩放回 Q8 范围 (0 ~ 255)
    
//     return (mask > 255) ? 255 : ((mask < 0) ? 0 : mask);
// }

// /**
//  * @brief 圆角 2D 径向模糊单角计算与渲染函数
//  */
// __IPGUI_STATIC__ void draw_shadow_one_corner(
//     ipgui_surf_t             * surf,
//     ipgui_aabb_t             * clip,
//     ipgui_aabb_t             * cdraw,
//     ipgui_coord_t              cx,       // 圆角中心点 X 坐标
//     ipgui_coord_t              cy,       // 圆角中心点 Y 坐标
//     ipgui_coord_t              r,        // 最终应用的圆角半径
//     ipgui_coord_t              blur,     // 模糊半径
//     ipgui_box_shadow_style_t * shadow_style)
// {
//     ipgui_coord_t w = ipgui_aabb_width(cdraw);
//     ipgui_coord_t h = ipgui_aabb_height(cdraw);
//     ipgui_coord_t res_h;

//     u8_t * mbuf = ipgui_mask_buf_acquire(w, h, &res_h);
//     if (!mbuf || res_h == 0) {
//         ipgui_dbg_error("error: shadow corner failed to acquire mask buffer\r\n");
//         return;
//     }

//     ipgui_coord_t drawn_h = 0;
//     ipgui_aabb_t mask_aabb;
//     mask_aabb.start.x = cdraw->start.x;
//     mask_aabb.end.x   = cdraw->end.x;

//     while (h > 0) {
//         ipgui_coord_t current_h = IPGUI_MIN(h, res_h);

//         for (int i = 0; i < current_h; i++) {
//             ipgui_coord_t draw_y = cdraw->start.y + drawn_h + i;
//             s32_t dy = IPGUI_ABS(draw_y - cy);
//             u8_t * row_mask = mbuf + i * w;

//             for (int j = 0; j < w; j++) {
//                 ipgui_coord_t draw_x = cdraw->start.x + j;
//                 s32_t dx = IPGUI_ABS(draw_x - cx);
                
//                 // 计算该点到圆心的 2D 欧氏距离
//                 u32_t dist = ipgui_sqrt32(dx * dx + dy * dy);
//                 // 得到相对于边缘的代数距离
//                 s32_t edge_dist = (s32_t)dist - r;
                
//                 row_mask[j] = ipgui_get_shadow_blur_mask(edge_dist, blur);
//             }
//         }

//         mask_aabb.start.y = cdraw->start.y + drawn_h;
//         mask_aabb.end.y   = mask_aabb.start.y + current_h - 1;

//         ipgui_blend(surf, clip, &mask_aabb, &(shadow_style->paint),
//                     shadow_style->opacity, mbuf, &mask_aabb, shadow_style->blend_mode);

//         drawn_h += current_h;
//         h       -= current_h;
//     }
//     ipgui_mask_buf_free(mbuf);
// }

// /**
//  * @brief 外部主接口：矩形阴影渲染九宫格流水线
//  */
// __IPGUI_API__ void ipgui_draw_box_shadow(
//     ipgui_surf_t             * surf,
//     ipgui_aabb_t             * clip,
//     ipgui_aabb_t             * box,
//     ipgui_box_style_t        * style,
//     ipgui_box_shadow_style_t * shadow_style)
// {
//     if (!surf || !box || !shadow_style || shadow_style->opacity < 3)
//         return;

//     ipgui_coord_t blur = shadow_style->blur_radius;
//     ipgui_coord_t spread = shadow_style->spread;

//     // 1. 计算基本的内衬 Padding 盒子
//     ipgui_aabb_t padding_box;
//     padding_box.start.x = box->start.x - (style ? style->left_padding   : 0);
//     padding_box.start.y = box->start.y - (style ? style->top_padding    : 0);
//     padding_box.end.x   = box->end.x   + (style ? style->right_padding  : 0);
//     padding_box.end.y   = box->end.y   + (style ? style->bottom_padding : 0);

//     // 2. 计算未模糊的阴影实体中心骨架 AABB 框（融入 offset 偏移量和 spread 扩张量）
//     ipgui_aabb_t core_box;
//     core_box.start.x = padding_box.start.x - spread + shadow_style->offset_x;
//     core_box.end.x   = padding_box.end.x   + spread + shadow_style->offset_x;
//     core_box.start.y = padding_box.start.y - spread + shadow_style->offset_y;
//     core_box.end.y   = padding_box.end.y   + spread + shadow_style->offset_y;

//     // 3. 叠加模糊半径，得到最终受阴影污染的全局最大渲染框
//     ipgui_aabb_t total_shadow_box;
//     total_shadow_box.start.x = core_box.start.x - blur;
//     total_shadow_box.end.x   = core_box.end.x   + blur;
//     total_shadow_box.start.y = core_box.start.y - blur;
//     total_shadow_box.end.y   = core_box.end.y   + blur;

//     // 4. 重合区域裁剪优化
//     ipgui_aabb_t draw;
//     if (clip) {
//         if (0 != ipgui_aabb_overlap(&draw, clip, &surf->surf)) return;
//     } else draw = surf->surf;

//     if (0 != ipgui_aabb_overlap(&draw, &draw, &total_shadow_box)) return;

//     // 5. 提取并根据 spread 同步缩放四个拐角的圆角半径
//     ipgui_coord_t r_lt, r_rt, r_lb, r_rb;
//     get_max_radius(&padding_box, style, &r_lt, &r_rt, &r_lb, &r_rb);
    
//     r_lt = (r_lt > 0) ? (r_lt + spread) : 0;
//     r_rt = (r_rt > 0) ? (r_rt + spread) : 0;
//     r_lb = (r_lb > 0) ? (r_lb + spread) : 0;
//     r_rb = (r_rb > 0) ? (r_rb + spread) : 0;

//     ipgui_coord_t l_max = IPGUI_MAX(r_lt, r_lb);
//     ipgui_coord_t r_max = IPGUI_MAX(r_rt, r_rb);

//     ipgui_aabb_t fill, cdraw;

// #define draw_solid_shadow(f) \
//     ipgui_blend(surf, clip, &(f), &(shadow_style->paint), \
//         shadow_style->opacity, (u8_t *)0, (ipgui_aabb_t *)0, shadow_style->blend_mode)

//     // ==================== PART 1: 渲染中心纯色全黑实体核心区 ====================
//     fill.start.x = core_box.start.x + l_max;
//     fill.end.x   = core_box.end.x   - r_max;
//     fill.start.y = core_box.start.y + blur;
//     fill.end.y   = core_box.end.y   - blur;
//     if (fill.end.x >= fill.start.x && fill.end.y >= fill.start.y) {
//         draw_solid_shadow(fill);
//     }

//     // 左侧圆角和模糊带之间的纯色块补全
//     if (l_max > blur) {
//         fill.start.x = core_box.start.x + blur;
//         fill.end.x   = core_box.start.x + l_max - 1;
//         fill.start.y = core_box.start.y + r_lt;
//         fill.end.y   = core_box.end.y   - r_lb;
//         if (fill.end.x >= fill.start.x && fill.end.y >= fill.start.y) draw_solid_shadow(fill);
//     }
//     // 右侧圆角和模糊带之间的纯色块补全
//     if (r_max > blur) {
//         fill.start.x = core_box.end.x - r_max + 1;
//         fill.end.x   = core_box.end.x - blur;
//         fill.start.y = core_box.start.y + r_rt;
//         fill.end.y   = core_box.end.y   - r_rb;
//         if (fill.end.x >= fill.start.x && fill.end.y >= fill.start.y) draw_solid_shadow(fill);
//     }

//     // ==================== PART 2: 渲染四个边缘的 1D 线性模糊带 ====================
    
//     // 【顶边 1D 模糊区域】 
//     fill.start.x = core_box.start.x + r_lt;
//     fill.end.x   = core_box.end.x   - r_rt;
//     fill.start.y = core_box.start.y - blur;
//     fill.end.y   = core_box.start.y + blur - 1;
//     if (fill.end.x >= fill.start.x && fill.end.y >= fill.start.y && 0 == ipgui_aabb_overlap(&cdraw, &draw, &fill)) {
//         ipgui_coord_t w = ipgui_aabb_width(&cdraw), h = ipgui_aabb_height(&cdraw), res_h;
//         u8_t * mbuf = ipgui_mask_buf_acquire(w, h, &res_h);
//         if (mbuf) {
//             ipgui_coord_t drawn_h = 0;
//             while (h > 0) {
//                 ipgui_coord_t current_h = IPGUI_MIN(h, res_h);
//                 for (int i = 0; i < current_h; i++) {
//                     s32_t dy = (cdraw.start.y + drawn_h + i) - core_box.start.y;
//                     ipgui_memset(mbuf + i * w, ipgui_get_shadow_blur_mask(dy, blur), w); // 1D行数据完全相同，直接memset
//                 }
//                 ipgui_aabb_t mask_aabb = cdraw;
//                 mask_aabb.start.y = cdraw.start.y + drawn_h;
//                 mask_aabb.end.y = mask_aabb.start.y + current_h - 1;
//                 ipgui_blend(surf, clip, &mask_aabb, &shadow_style->paint, shadow_style->opacity, mbuf, &mask_aabb, shadow_style->blend_mode);
//                 drawn_h += current_h; h -= current_h;
//             }
//             ipgui_mask_buf_free(mbuf);
//         }
//     }

//     // 【底边 1D 模糊区域】
//     fill.start.x = core_box.start.x + r_lb;
//     fill.end.x   = core_box.end.x   - r_rb;
//     fill.start.y = core_box.end.y   - blur + 1;
//     fill.end.y   = core_box.end.y   + blur;
//     if (fill.end.x >= fill.start.x && fill.end.y >= fill.start.y && 0 == ipgui_aabb_overlap(&cdraw, &draw, &fill)) {
//         ipgui_coord_t w = ipgui_aabb_width(&cdraw), h = ipgui_aabb_height(&cdraw), res_h;
//         u8_t * mbuf = ipgui_mask_buf_acquire(w, h, &res_h);
//         if (mbuf) {
//             ipgui_coord_t drawn_h = 0;
//             while (h > 0) {
//                 ipgui_coord_t current_h = IPGUI_MIN(h, res_h);
//                 for (int i = 0; i < current_h; i++) {
//                     s32_t dy = core_box.end.y - (cdraw.start.y + drawn_h + i);
//                     ipgui_memset(mbuf + i * w, ipgui_get_shadow_blur_mask(dy, blur), w);
//                 }
//                 ipgui_aabb_t mask_aabb = cdraw;
//                 mask_aabb.start.y = cdraw.start.y + drawn_h;
//                 mask_aabb.end.y = mask_aabb.start.y + current_h - 1;
//                 ipgui_blend(surf, clip, &mask_aabb, &shadow_style->paint, shadow_style->opacity, mbuf, &mask_aabb, shadow_style->blend_mode);
//                 drawn_h += current_h; h -= current_h;
//             }
//             ipgui_mask_buf_free(mbuf);
//         }
//     }

//     // 【左边 1D 模糊区域】
//     fill.start.x = core_box.start.x - blur;
//     fill.end.x   = core_box.start.x + blur - 1;
//     fill.start.y = core_box.start.y + r_lt;
//     fill.end.y   = core_box.end.y   - r_lb;
//     if (fill.end.x >= fill.start.x && fill.end.y >= fill.start.y && 0 == ipgui_aabb_overlap(&cdraw, &draw, &fill)) {
//         ipgui_coord_t w = ipgui_aabb_width(&cdraw), h = ipgui_aabb_height(&cdraw), res_h;
//         u8_t * mbuf = ipgui_mask_buf_acquire(w, h, &res_h);
//         if (mbuf) {
//             // 高性能优化：只计算第一行的横向阴影剖面数据
//             for (int j = 0; j < w; j++) {
//                 s32_t dx = (cdraw.start.x + j) - core_box.start.x;
//                 mbuf[j] = ipgui_get_shadow_blur_mask(dx, blur);
//             }
//             ipgui_coord_t drawn_h = 0;
//             while (h > 0) {
//                 ipgui_coord_t current_h = IPGUI_MIN(h, res_h);
//                 // 剩余行直接通过高速内存拷贝复制，彻底免除复杂的数学开销
//                 for (int i = (drawn_h == 0 ? 1 : 0); i < current_h; i++) {
//                     ipgui_memcpy(mbuf + i * w, mbuf, w);
//                 }
//                 ipgui_aabb_t mask_aabb = cdraw;
//                 mask_aabb.start.y = cdraw.start.y + drawn_h;
//                 mask_aabb.end.y = mask_aabb.start.y + current_h - 1;
//                 ipgui_blend(surf, clip, &mask_aabb, &shadow_style->paint, shadow_style->opacity, mbuf, &mask_aabb, shadow_style->blend_mode);
//                 drawn_h += current_h; h -= current_h;
//             }
//             ipgui_mask_buf_free(mbuf);
//         }
//     }

//     // 【右边 1D 模糊区域】
//     fill.start.x = core_box.end.x - blur + 1;
//     fill.end.x   = core_box.end.x + blur;
//     fill.start.y = core_box.start.y + r_rt;
//     fill.end.y   = core_box.end.y   - r_rb;
//     if (fill.end.x >= fill.start.x && fill.end.y >= fill.start.y && 0 == ipgui_aabb_overlap(&cdraw, &draw, &fill)) {
//         ipgui_coord_t w = ipgui_aabb_width(&cdraw), h = ipgui_aabb_height(&cdraw), res_h;
//         u8_t * mbuf = ipgui_mask_buf_acquire(w, h, &res_h);
//         if (mbuf) {
//             for (int j = 0; j < w; j++) {
//                 s32_t dx = core_box.end.x - (cdraw.start.x + j);
//                 mbuf[j] = ipgui_get_shadow_blur_mask(dx, blur);
//             }
//             ipgui_coord_t drawn_h = 0;
//             while (h > 0) {
//                 ipgui_coord_t current_h = IPGUI_MIN(h, res_h);
//                 for (int i = (drawn_h == 0 ? 1 : 0); i < current_h; i++) {
//                     ipgui_memcpy(mbuf + i * w, mbuf, w);
//                 }
//                 ipgui_aabb_t mask_aabb = cdraw;
//                 mask_aabb.start.y = cdraw.start.y + drawn_h;
//                 mask_aabb.end.y = mask_aabb.start.y + current_h - 1;
//                 ipgui_blend(surf, clip, &mask_aabb, &shadow_style->paint, shadow_style->opacity, mbuf, &mask_aabb, shadow_style->blend_mode);
//                 drawn_h += current_h; h -= current_h;
//             }
//             ipgui_mask_buf_free(mbuf);
//         }
//     }

// #undef draw_solid_shadow

//     // ==================== PART 3: 渲染四个拐角的 2D 径向模糊区 ====================
//     ipgui_aabb_t corner;
    
//     // 【左上角】 
//     corner.start.x = core_box.start.x - blur;
//     corner.end.x   = core_box.start.x + r_lt - 1;
//     corner.start.y = core_box.start.y - blur;
//     corner.end.y   = core_box.start.y + r_lt - 1;
//     if (corner.end.x >= corner.start.x && corner.end.y >= corner.start.y && 0 == ipgui_aabb_overlap(&cdraw, &draw, &corner)) {
//         draw_shadow_one_corner(surf, clip, &cdraw, core_box.start.x + r_lt, core_box.start.y + r_lt, r_lt, blur, shadow_style);
//     }

//     // 【右上角】 
//     corner.start.x = core_box.end.x - r_rt + 1;
//     corner.end.x   = core_box.end.x + blur;
//     corner.start.y = core_box.start.y - blur;
//     corner.end.y   = core_box.start.y + r_rt - 1;
//     if (corner.end.x >= corner.start.x && corner.end.y >= corner.start.y && 0 == ipgui_aabb_overlap(&cdraw, &draw, &corner)) {
//         draw_shadow_one_corner(surf, clip, &cdraw, core_box.end.x - r_rt, core_box.start.y + r_rt, r_rt, blur, shadow_style);
//     }

//     // 【左下角】 
//     corner.start.x = core_box.start.x - blur;
//     corner.end.x   = core_box.start.x + r_lb - 1;
//     corner.start.y = core_box.end.y - r_lb + 1;
//     corner.end.y   = core_box.end.y + blur;
//     if (corner.end.x >= corner.start.x && corner.end.y >= corner.start.y && 0 == ipgui_aabb_overlap(&cdraw, &draw, &corner)) {
//         draw_shadow_one_corner(surf, clip, &cdraw, core_box.start.x + r_lb, core_box.end.y - r_lb, r_lb, blur, shadow_style);
//     }

//     // 【右下角】 
//     corner.start.x = core_box.end.x - r_rb + 1;
//     corner.end.x   = core_box.end.x + blur;
//     corner.start.y = core_box.end.y - r_rb + 1;
//     corner.end.y   = core_box.end.y + blur;
//     if (corner.end.x >= corner.start.x && corner.end.y >= corner.start.y && 0 == ipgui_aabb_overlap(&cdraw, &draw, &corner)) {
//         draw_shadow_one_corner(surf, clip, &cdraw, core_box.end.x - r_rb, core_box.end.y - r_rb, r_rb, blur, shadow_style);
//     }
// }