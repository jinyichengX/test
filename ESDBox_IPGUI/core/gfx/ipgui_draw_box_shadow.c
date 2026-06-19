/*===========================================================================
 * ipgui_draw_box_shadow.c — 盒阴影渲染（SDF 距离场 + 高斯 CDF 对称衰减）
 *
 * 对标 CSS box-shadow 的物理过程：
 *   1. 生成形状的硬边缘 mask（spread 膨胀后的 padding_box）
 *   2. 对 mask 做二维高斯模糊 → 等价于用 SDF 距离 + CDF(Φ) 查表
 *   3. 关键：边界两侧对称模糊 —— 内部也衰减，边界处 alpha≈50%
 *
 * 流程：
 *   1) dilated_pad = pad + spread（膨胀形状，圆角半径同步增大）
 *   2) 计算 SDF 到 dilated_pad 边界的距离 d（>0 外部，<0 内部）
 *   3) d 映射到 [-blur_range, +blur_range] → CDF 查表
 *      - d ≤ -blur_range → alpha = base_alpha（深处满不透明度）
 *      - d = 0          → alpha ≈ base_alpha * 0.5（边界50%）
 *      - d ≥ +blur_range → alpha = 0（外部完全透明）
 *   4) 生成 mask → 一次 ipgui_blend_color 渲染
 *===========================================================================*/

#include "ipgui_draw_box_shadow.h"
#include "ipgui_blend_color.h"
#include "ipgui_memory.h"

/* -------- Φ(-d/σ) CDF 查找表 --------
 * idx=0   → d = -3σ → Φ(3)  ≈ 0.9986 → 255
 * idx=128 → d = 0   → Φ(0)  = 0.5    → 127
 * idx=255 → d = +3σ → Φ(-3) ≈ 0.0014 →   0
 *
 * 用 Abramowitz & Stegun erf 近似生成，精度 > 1e-6
 * ------------------------------------ */
static const u8_t cdf_lut[256] = {
    255,255,255,255,255,254,254,254,254,254,254,254,254,254,254,254,
    254,254,254,254,254,253,253,253,253,253,253,253,253,252,252,252,
    252,252,251,251,251,251,251,250,250,250,249,249,249,248,248,248,
    247,247,246,246,245,245,244,244,243,243,242,242,241,240,239,239,
    238,237,236,236,235,234,233,232,231,230,229,228,227,225,224,223,
    222,220,219,218,216,215,214,212,211,209,207,206,204,202,201,199,
    197,195,194,192,190,188,186,184,182,180,178,176,173,171,169,167,
    165,163,160,158,156,153,151,149,146,144,142,139,137,135,132,130,
    127,125,123,120,118,116,113,111,109,106,104,102, 99, 97, 95, 92,
     90, 88, 86, 84, 82, 79, 77, 75, 73, 71, 69, 67, 65, 63, 61, 60,
     58, 56, 54, 53, 51, 49, 48, 46, 44, 43, 41, 40, 39, 37, 36, 35,
     33, 32, 31, 30, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 19, 18,
     17, 16, 16, 15, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,
      8,  7,  7,  7,  6,  6,  6,  5,  5,  5,  4,  4,  4,  4,  4,  3,
      3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,  1,  1,  1,  1,
      1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
};

/* ========================= 整数开平方 ========================= */
static int isqrt_scaled256(int val)
{
    int x;
    if (val <= 1) return val * 16;
    {
        int shift = 0, t = val;
        while (t > 0) { t >>= 1; shift++; }
        x = 1 << (shift / 2);
    }
    x = (x + val / x) >> 1;
    x = (x + val / x) >> 1;
    x = (x + val / x) >> 1;
    x = (x + val / x) >> 1;
    return x << 4;
}

/* ========================= 圆角矩形 SDF =========================
 * 所有坐标/半径均为定点 scale=256
 * 返回：距离 × 256（>0 在外部，≤0 在内部）
 *=================================================================*/
static int sd_rounded_box_fixed(
    int px, int py,
    int x1, int y1, int x2, int y2,
    int tl_r, int tr_r, int bl_r, int br_r)
{
    int dx, dy;
    long long d2;

    /* 左上角 */
    if (px <= x1 + tl_r && py <= y1 + tl_r) {
        dx = px - (x1 + tl_r); dy = py - (y1 + tl_r);
        d2 = ((long long)dx * dx + (long long)dy * dy) >> 8;
        return isqrt_scaled256((int)d2) - tl_r;
    }
    /* 右上角 */
    if (px >= x2 - tr_r && py <= y1 + tr_r) {
        dx = px - (x2 - tr_r); dy = py - (y1 + tr_r);
        d2 = ((long long)dx * dx + (long long)dy * dy) >> 8;
        return isqrt_scaled256((int)d2) - tr_r;
    }
    /* 左下角 */
    if (px <= x1 + bl_r && py >= y2 - bl_r) {
        dx = px - (x1 + bl_r); dy = py - (y2 - bl_r);
        d2 = ((long long)dx * dx + (long long)dy * dy) >> 8;
        return isqrt_scaled256((int)d2) - bl_r;
    }
    /* 右下角 */
    if (px >= x2 - br_r && py >= y2 - br_r) {
        dx = px - (x2 - br_r); dy = py - (y2 - br_r);
        d2 = ((long long)dx * dx + (long long)dy * dy) >> 8;
        return isqrt_scaled256((int)d2) - br_r;
    }

    /* 四边直线区域 */
    dx = 0; dy = 0;
    if      (px < x1) dx = x1 - px;
    else if (px > x2) dx = px - x2;
    if      (py < y1) dy = y1 - py;
    else if (py > y2) dy = py - y2;

    if (dx == 0) return dy;
    if (dy == 0) return dx;
    d2 = ((long long)dx * dx + (long long)dy * dy) >> 8;
    return isqrt_scaled256((int)d2);
}

/* ========================= CDF 衰减 =========================
 * d          : SDF 距离 × 256 (>0外部, <0内部)
 * blur_range : 模糊半宽（像素），对应 3σ
 * base       : 最大 alpha
 *
 * 映射：idx = (d + blur_range*256) * 255 / (2 * blur_range * 256)
 *        d=-br*256 → idx=0   → 255 (满)
 *        d=0       → idx=128 → 127 (50%)
 *        d=+br*256 → idx=255 →   0 (零)
 *===========================================================*/
static u8_t cdf_falloff(int d, int blur_range, u8_t base)
{
    int idx;
    long long num, den;

    if (d <= -(blur_range << 8)) return base;
    if (d >=  (blur_range << 8)) return 0;

    /* idx = (d + br*256) * 255 / (br * 512) */
    num = (long long)d + ((long long)blur_range << 8);
    num = num * 255LL;
    den = (long long)blur_range << 9;  /* br * 512 */

    idx = (int)(num / den);
    if (idx < 0) idx = 0;
    if (idx > 255) idx = 255;

    return (u8_t)(((u32_t)cdf_lut[idx] * base + 127) >> 8);
}

/* ========================= 生成阴影掩码 ========================= */
static void shadow_mask_generate(
    u8_t               *mask,
    const ipgui_aabb_t *shadow_aabb,
    const ipgui_aabb_t *dilated_pad,
    int tl_r, int tr_r, int bl_r, int br_r,
    int blur_range,
    int off_x, int off_y,
    int base_alpha)
{
    const int mw  = ipgui_aabb_width(shadow_aabb);
    const int mh  = ipgui_aabb_height(shadow_aabb);
    const int sx  = shadow_aabb->start.x;
    const int sy  = shadow_aabb->start.y;

    const int pad_x1 = (dilated_pad->start.x << 8) + 128 + (off_x << 8);
    const int pad_y1 = (dilated_pad->start.y << 8) + 128 + (off_y << 8);
    const int pad_x2 = (dilated_pad->end.x   << 8) + 128 + (off_x << 8);
    const int pad_y2 = (dilated_pad->end.y   << 8) + 128 + (off_y << 8);

    const int r_tl = tl_r << 8;
    const int r_tr = tr_r << 8;
    const int r_bl = bl_r << 8;
    const int r_br = br_r << 8;

    int x, y;
    for (y = 0; y < mh; y++) {
        int py = ((sy + y) << 8) + 128;
        for (x = 0; x < mw; x++) {
            int px = ((sx + x) << 8) + 128;
            int d  = sd_rounded_box_fixed(px, py,
                                          pad_x1, pad_y1, pad_x2, pad_y2,
                                          r_tl, r_tr, r_bl, r_br);
            mask[y * mw + x] = cdf_falloff(d, blur_range, (u8_t)base_alpha);
        }
    }
}

/* ========================= 公开 API ========================= */
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

    int spread = ss->spread;
    int blur   = ss->blur;

    if (blur <= 0 && spread <= 0 && ss->offset_x == 0 && ss->offset_y == 0)
        return;

    /* blur_range = 3σ，取值 blur × 2 */
    int blur_range = blur * 2;

    /* ---- 膨胀后的 pad ---- */
    ipgui_aabb_t dilated_pad = pad;
    dilated_pad.start.x -= spread;
    dilated_pad.start.y -= spread;
    dilated_pad.end.x   += spread;
    dilated_pad.end.y   += spread;

    /* ---- 圆角半径同步膨胀 ---- */
    int tl_r = 0, tr_r = 0, bl_r = 0, br_r = 0;
    if (style) {
        int dw = ipgui_aabb_width(&dilated_pad);
        int dh = ipgui_aabb_height(&dilated_pad);
        int half_max = IPGUI_MIN(dw / 2, dh / 2);
        tl_r = IPGUI_MIN(style->left_top_radius     + spread, half_max);
        tr_r = IPGUI_MIN(style->right_top_radius    + spread, half_max);
        bl_r = IPGUI_MIN(style->left_bottom_radius  + spread, half_max);
        br_r = IPGUI_MIN(style->right_bottom_radius + spread, half_max);
    }

    /* ---- 阴影包围盒 = dilated_pad ± blur_range + offset ---- */
    int sx1 = dilated_pad.start.x + ss->offset_x - blur_range;
    int sy1 = dilated_pad.start.y + ss->offset_y - blur_range;
    int sx2 = dilated_pad.end.x   + ss->offset_x + blur_range;
    int sy2 = dilated_pad.end.y   + ss->offset_y + blur_range;

    ipgui_aabb_t shadow_aabb = {{sx1, sy1}, {sx2, sy2}};
    {
        ipgui_aabb_t clipped;
        if (0 != ipgui_aabb_overlap(&clipped, &shadow_aabb, clip ? clip : &surf->surf))
            return;
        shadow_aabb = clipped;
    }

    int mw = ipgui_aabb_width(&shadow_aabb);
    int mh = ipgui_aabb_height(&shadow_aabb);
    if (mw <= 0 || mh <= 0) return;

    size_t mask_bytes = (size_t)mw * (size_t)mh;
    u8_t *mask = (u8_t *)ipgui_mem_alloc_def(mask_bytes);
    if (!mask) return;

    int base_alpha = (int)(((u32_t)IPGUI_COLOR_A(ss->color) * ss->opacity + 127) >> 8);

    shadow_mask_generate(mask, &shadow_aabb, &dilated_pad,
                         tl_r, tr_r, bl_r, br_r,
                         blur_range,
                         ss->offset_x, ss->offset_y,
                         base_alpha);

    ipgui_color_t col = ss->color;
    IPGUI_COLOR_SET_A(col, 255);
    ipgui_blend_color(surf, clip, &shadow_aabb, col, 255,
                      mask, &shadow_aabb, IPGUI_BLEND_NORMAL);

    ipgui_mem_free_def(mask);
}
