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
// typedef struct {
//     float s, v, a;
// } inner_gradient_t;

// static void inner_gradient_init(inner_gradient_t *g, float blur_w)
// {
//     g->s = 255.0f;
//     g->v = 512.0f / blur_w;
//     g->a = 2.0f * (g->v * blur_w - g->s) / (blur_w * blur_w);
// }

// static u8_t inner_gradient_compute(inner_gradient_t *g, float t)
// {
//     float val = g->s - (g->v * t - g->a * t * t * 0.5f);
//     if (val < 0)   return 0;
//     if (val > 255) return 255;
//     return (u8_t)val;
// }

// /* 圆角矩形 SDF：负 = 内部，正 = 外部 */
// static float inner_rounded_rect_sdf(
//     float lx, float ly,
//     float w,  float h, float r)
// {
//     float qx = IPGUI_ABS(lx - w * 0.5f) - w * 0.5f + r;
//     float qy = IPGUI_ABS(ly - h * 0.5f) - h * 0.5f + r;
//     float ox = qx > 0.0f ? qx : 0.0f;
//     float oy = qy > 0.0f ? qy : 0.0f;
//     return sqrtf(ox * ox + oy * oy) - r;
// }
// __IPGUI_API__ void ipgui_draw_inner_shadow(
//     ipgui_surf_t             *surf,
//     ipgui_aabb_t             *clip,
//     ipgui_aabb_t             *content_box,
//     ipgui_box_shadow_style_t *style)
// {
//     if (!surf || !content_box || !style) return;
//     if (style->opacity < 3) return;

//     /* ── 基本参数 ── */
//     ipgui_coord_t blur   = style->blur;
//     ipgui_coord_t spread = style->spread;   /* 正值 = 阴影带加宽；负值 = 缩小 */
//     ipgui_coord_t ox     = style->offset_x;
//     ipgui_coord_t oy     = style->offset_y;
//     ipgui_coord_t cr     = style->corner_radius;

//     float blur_w = blur * 1.5f;
//     if (blur_w < 1.0f) blur_w = 1.0f;

//     /* ── content box 尺寸 ── */
//     ipgui_coord_t bx = content_box->start.x;
//     ipgui_coord_t by = content_box->start.y;
//     ipgui_coord_t bw = ipgui_aabb_width(content_box);
//     ipgui_coord_t bh = ipgui_aabb_height(content_box);

//     /* content box 圆角（夹紧） */
//     ipgui_coord_t box_r = IPGUI_MIN(cr, bw / 2);
//     box_r = IPGUI_MIN(box_r, bh / 2);
//     box_r = IPGUI_MAX(box_r, 0);

//     /* ── inner shadow box：向内收缩 spread，并偏移 ox/oy ──
//      *
//      *  布局示意（spread > 0）：
//      *
//      *  ┌─────────────────────────────┐  ← content box 边缘
//      *  │  ░░░░░░░░░░░░░░░░░░░░░░░░  │  ← 实色阴影带（content ~ shadow box）
//      *  │  ░ ┌─────────────────┐ ░  │
//      *  │  ░ │  ░░░░ blur ░░░░ │ ░  │  ← shadow box 内侧：模糊渐变
//      *  │  ░ │                 │ ░  │
//      *  │  ░ │   (透明区域)    │ ░  │
//      *  │  ░ └─────────────────┘ ░  │  ← shadow box
//      *  │  ░░░░░░░░░░░░░░░░░░░░░░░░  │
//      *  └─────────────────────────────┘
//      */
//     ipgui_coord_t sw_x = bx + spread + ox;
//     ipgui_coord_t sw_y = by + spread + oy;
//     ipgui_coord_t sw_w = bw - spread * 2;
//     ipgui_coord_t sw_h = bh - spread * 2;

//     /* shadow box 圆角：随 spread 缩小，不能为负 */
//     ipgui_coord_t sw_r = cr - spread;

//     /* ── 退化情形：spread 过大，整个 content box 均被实色覆盖 ── */
//     int degenerate = (sw_w <= 0 || sw_h <= 0);
//     if (!degenerate) {
//         sw_r = IPGUI_MIN(sw_r, sw_w / 2);
//         sw_r = IPGUI_MIN(sw_r, sw_h / 2);
//         sw_r = IPGUI_MAX(sw_r, 0);
//     }

//     inner_gradient_t g;
//     inner_gradient_init(&g, blur_w);

//     /* ── 遍历范围：仅 content box 内部（+1px 边缘抗锯齿） ── */
//     ipgui_coord_t x0 = bx - 1;
//     ipgui_coord_t y0 = by - 1;
//     ipgui_coord_t x1 = bx + bw + 1;
//     ipgui_coord_t y1 = by + bh + 1;

//     ipgui_coord_t x, y;
//     for (y = y0; y <= y1; y++) {
//         for (x = x0; x <= x1; x++) {

//             /* ── step1：像素必须在 content box 内部（含边缘抗锯齿带）── */
//             float clx   = (float)(x - bx);
//             float cly   = (float)(y - by);
//             float cdist = inner_rounded_rect_sdf(
//                 clx, cly, (float)bw, (float)bh, (float)box_r);

//             /* cdist > 0：在 content box 外部，完全跳过 */
//             if (cdist > 1.0f) continue;

//             /* ── step2：计算该像素相对 inner shadow box 的 shadow_mask ── */
//             u8_t shadow_mask;

//             if (degenerate) {
//                 /* shadow box 已消失，整个 content box 均为实色 */
//                 shadow_mask = 255;
//             } else {
//                 float lx_f  = (float)(x - sw_x);
//                 float ly_f  = (float)(y - sw_y);
//                 float dist  = inner_rounded_rect_sdf(
//                     lx_f, ly_f, (float)sw_w, (float)sw_h, (float)sw_r);

//                 /*
//                  *  dist > 0  →  像素在 shadow box 外侧（实色阴影区）
//                  *  dist <= 0 →  像素在 shadow box 内侧
//                  *    inner = -dist：到 shadow box 边缘的"深入距离"
//                  *    inner < blur_w  → 模糊渐变
//                  *    inner >= blur_w → 完全透明，跳过
//                  */
//                 if (dist >= 0.0f) {
//                     shadow_mask = 255;
//                 } else {
//                     float inner = -dist;
//                     if (inner >= blur_w) continue;
//                     shadow_mask = inner_gradient_compute(&g, inner);
//                     if (shadow_mask < 2) continue;
//                 }
//             }

//             /* ── step3：content box 边缘抗锯齿 ──
//              *
//              *  cdist 从负（内部）趋向 0（恰好在边缘）时，线性淡出，
//              *  使内阴影与外层圆角边框平滑融合。
//              *
//              *  cdist <= -1.0f  → 完全在内部，无衰减
//              *  cdist  ∈ (-1,0] → 过渡带，alpha 线性递减至 0
//              */
//             if (cdist > -1.0f) {
//                 /* edge_factor: 1.0(深处) → 0.0(边缘) */
//                 float edge_factor = -cdist;            /* cdist 在 (-1, 0] 时为正 */
//                 if (edge_factor < 0.0f) edge_factor = 0.0f;
//                 u8_t edge = (u8_t)(edge_factor * 255.0f);
//                 shadow_mask = (u8_t)(((u32_t)shadow_mask * edge) >> 8);
//                 if (shadow_mask < 2) continue;
//             }

//             /* ── step4：写入像素 ── */
//             ipgui_draw_pixel(
//                 surf, clip,
//                 x, y,
//                 style->color,
//                 shadow_mask,
//                 style->opacity,
//                 style->blend_mode);
//         }
//     }
// }