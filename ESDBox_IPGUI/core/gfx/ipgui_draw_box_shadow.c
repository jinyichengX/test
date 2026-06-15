/*===========================================================================
 * ipgui_draw_box_shadow.c — 盒阴影渲染（掩码法 / SDF 距离场）
 *
 * 设计思路：
 *   传统逐像素 ipgui_draw_pixel 方案在模糊区域需要逐像素调用颜色转换 +
 *   预乘 + 混合流水线，开销极高（每次调用 ~12 次移位/乘法 + 函数调用开销）。
 *   掩码法将阴影渲染分解为两步：
 *     1) 生成灰度掩码（每个像素的 alpha 值）
 *     2) 一次 ipgui_blend_color 调用将掩码混合到目标表面
 *   掩码的每个像素值由圆角矩形的有符号距离场 (SDF) 映射到 alpha：
 *     d = SDF(像素, padding_box + 偏移)
 *     d ≤ 0            → alpha = 0              (padding_box 内部)
 *     0 < d ≤ spread   → alpha = base           (扩散纯色区)
 *     spread < d ≤ spread+blur → cubic ease-out  (模糊过渡区)
 *     d > spread+blur  → alpha = 0              (超出范围)
 *
 *   ipgui_draw_pixel 不再被内部渲染管线依赖。
 *===========================================================================*/

#include "ipgui_draw_box_shadow.h"
#include "ipgui_blend_color.h"
#include "ipgui_prim.h"
#include <math.h>

/* ---------------------------------------------------------------------------
 * 有符号距离场：点到圆角矩形（4 个独立圆角半径）的距离
 *
 * 返回值：
 *   d > 0  点在形状外侧，d 为最短距离
 *   d = 0  点在形状边界上
 *   d < 0  点在形状内侧（但阴影不需要，始终 return 0 作 alpha）
 *
 * 区域判定：
 *   四角圆弧区域：点到圆心距离 - 半径
 *   四边直线区域：点到对应边的垂直距离
 *-----------------------------------------------------------------------*/
static float sd_rounded_box(
    float px, float py,
    float x1, float y1, float x2, float y2,
    float tl_r, float tr_r, float bl_r, float br_r)
{
    float cx, cy, cr;

    /* ---- 左上角 ---- */
    if (px <= x1 + tl_r && py <= y1 + tl_r) {
        cx = x1 + tl_r; cy = y1 + tl_r; cr = tl_r;
        return sqrtf((px - cx) * (px - cx) + (py - cy) * (py - cy)) - cr;
    }
    /* ---- 右上角 ---- */
    if (px >= x2 - tr_r && py <= y1 + tr_r) {
        cx = x2 - tr_r; cy = y1 + tr_r; cr = tr_r;
        return sqrtf((px - cx) * (px - cx) + (py - cy) * (py - cy)) - cr;
    }
    /* ---- 左下角 ---- */
    if (px <= x1 + bl_r && py >= y2 - bl_r) {
        cx = x1 + bl_r; cy = y2 - bl_r; cr = bl_r;
        return sqrtf((px - cx) * (px - cx) + (py - cy) * (py - cy)) - cr;
    }
    /* ---- 右下角 ---- */
    if (px >= x2 - br_r && py >= y2 - br_r) {
        cx = x2 - br_r; cy = y2 - br_r; cr = br_r;
        return sqrtf((px - cx) * (px - cx) + (py - cy) * (py - cy)) - cr;
    }

    /* ---- 四边直线区域 ---- */
    float dx = 0.0f, dy = 0.0f;
    if      (px < x1) dx = x1 - px;
    else if (px > x2) dx = px - x2;
    if      (py < y1) dy = y1 - py;
    else if (py > y2) dy = py - y2;
    return sqrtf(dx * dx + dy * dy);
}

/* ---------------------------------------------------------------------------
 * 生成阴影掩码
 *
 * 对 shadow_aabb 区域内每个像素计算 SDF 距离，映射为 alpha 值写入 mask。
 * mask 尺寸 = shadow_aabb 尺寸，每像素 1 字节（u8_t）。
 *
 * 参数：
 *   mask         输出掩码缓冲区（调用方分配，w*h 字节）
 *   shadow_aabb  阴影包围盒（已裁剪到 surf 边界后）
 *   pad          padding_box（全局坐标）
 *   tl..br_r     四个圆角半径
 *   spread       扩展量
 *   blur         模糊量
 *   off_x,off_y  偏移量
 *   base_alpha   基准 alpha（shadow_style.color.a * shadow_style.opacity / 256）
 *-----------------------------------------------------------------------*/
static void shadow_mask_generate(
    u8_t               *mask,
    const ipgui_aabb_t *shadow_aabb,
    const ipgui_aabb_t *pad,
    int tl_r, int tr_r, int bl_r, int br_r,
    int spread, int blur,
    int off_x, int off_y,
    int base_alpha)
{
    const int mw = ipgui_aabb_width(shadow_aabb);
    const int mh = ipgui_aabb_height(shadow_aabb);
    const int sx = shadow_aabb->start.x;
    const int sy = shadow_aabb->start.y;
    const float total_dist = (float)(spread + blur);
    const float spread_f   = (float)spread;
    const float blur_f     = (float)blur;
    const float ba_f       = (float)base_alpha;

    float pad_x1 = (float)pad->start.x + 0.5f;
    float pad_y1 = (float)pad->start.y + 0.5f;
    float pad_x2 = (float)pad->end.x   + 0.5f;
    float pad_y2 = (float)pad->end.y   + 0.5f;
    float r_tl  = (float)tl_r;
    float r_tr  = (float)tr_r;
    float r_bl  = (float)bl_r;
    float r_br  = (float)br_r;
    float ox    = (float)off_x;
    float oy    = (float)off_y;

    int x, y;
    for (y = 0; y < mh; y++) {
        float py = (float)(sy + y) + 0.5f - oy;   /* 像素中心 → shape space */

        for (x = 0; x < mw; x++) {
            float px = (float)(sx + x) + 0.5f - ox;
            float d  = sd_rounded_box(px, py, pad_x1, pad_y1, pad_x2, pad_y2,
                                      r_tl, r_tr, r_bl, r_br);

            if (d <= 0.0f)       { mask[y * mw + x] = 0;          continue; }
            if (d <= spread_f)   { mask[y * mw + x] = base_alpha; continue; }
            if (d >= total_dist) { mask[y * mw + x] = 0;          continue; }

            /* cubic ease-out: alpha = base * (1 - t)^3,  t ∈ [0, 1] */
            float t = (d - spread_f) / blur_f;
            float a = ba_f * (1.0f - t) * (1.0f - t) * (1.0f - t);
            mask[y * mw + x] = (u8_t)(a + 0.5f);
        }
    }
}

/* ---------------------------------------------------------------------------
 * 公开 API
 *-----------------------------------------------------------------------*/
__IPGUI_API__ void ipgui_draw_box_shadow(
    ipgui_surf_t             *surf,
    ipgui_aabb_t             *clip,
    ipgui_aabb_t             *box,
    ipgui_box_style_t        *style,
    ipgui_box_shadow_style_t *ss)
{
    if (!surf || !box || !ss || ss->opacity < 3) return;

    /* ---- padding_box ---- */
    ipgui_aabb_t pad;
    if (style) {
        pad.start.x = box->start.x - style->left_padding;
        pad.start.y = box->start.y - style->top_padding;
        pad.end.x   = box->end.x   + style->right_padding;
        pad.end.y   = box->end.y   + style->bottom_padding;
    } else {
        pad = *box;
    }

    const int pad_w = ipgui_aabb_width(&pad);
    const int pad_h = ipgui_aabb_height(&pad);

    int blur   = (int)((float)ss->blur * 1.5f);
    int spread = ss->spread;
    int total  = ss->blur + ss->spread;

    /* 无可见阴影 → 直接返回 */
    if (total <= 0 && blur <= 0 && ss->offset_x == 0 && ss->offset_y == 0)
        return;

    /* ---- 圆角半径（裁剪到盒子尺寸的一半）---- */
    int tl_r = 0, tr_r = 0, bl_r = 0, br_r = 0;
    if (style) {
        int half_max = IPGUI_MIN(pad_w / 2, pad_h / 2);
        tl_r = IPGUI_MIN(style->left_top_radius,     half_max);
        tr_r = IPGUI_MIN(style->right_top_radius,    half_max);
        bl_r = IPGUI_MIN(style->left_bottom_radius,  half_max);
        br_r = IPGUI_MIN(style->right_bottom_radius, half_max);
    }

    /* ---- 阴影包围盒（pad + total + blur 扩展 + offset，然后裁剪到 surf）---- */
    int sx1 = pad.start.x + ss->offset_x - total - blur;
    int sy1 = pad.start.y + ss->offset_y - total - blur;
    int sx2 = pad.end.x   + ss->offset_x + total + blur;
    int sy2 = pad.end.y   + ss->offset_y + total + blur;

    ipgui_aabb_t shadow_aabb = {{sx1, sy1}, {sx2, sy2}};
    {
        ipgui_aabb_t clipped;
        if (0 != ipgui_aabb_overlap(&clipped, &shadow_aabb, clip ? clip : &surf->surf))
            return;   /* 完全在可见区域外 */
        shadow_aabb = clipped;
    }

    /* ---- 分配掩码缓冲区 ---- */
    int mw = ipgui_aabb_width(&shadow_aabb);
    int mh = ipgui_aabb_height(&shadow_aabb);
    if (mw <= 0 || mh <= 0) return;

    size_t mask_bytes = (size_t)mw * (size_t)mh;
    u8_t *mask = (u8_t *)ipgui_mem_alloc_def(mask_bytes);
    if (!mask) return;   /* 分配失败 → 静默跳过阴影 */

    /* ---- 基准 alpha = color.a × opacity / 256 ---- */
    int base_alpha = (int)(((u32_t)ss->color.a * ss->opacity + 127) >> 8);

    /* ---- 生成掩码 ---- */
    shadow_mask_generate(mask, &shadow_aabb, &pad,
                         tl_r, tr_r, bl_r, br_r,
                         spread, blur,
                         ss->offset_x, ss->offset_y,
                         base_alpha);

    /* ---- 一次 blend 完成渲染 ---- */
    ipgui_color_t col = ss->color;
    col.a = 255;   /* 不透明度已编码在掩码中 */
    ipgui_blend_color(surf, clip, &shadow_aabb, col, 255,
                      mask, &shadow_aabb, IPGUI_BLEND_NORMAL);

    /* ---- 释放掩码 ---- */
    ipgui_mem_free_def(mask);
}
