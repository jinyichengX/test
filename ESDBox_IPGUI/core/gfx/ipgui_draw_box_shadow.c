
/*
 * box-shadow 完整渲染顺序：
 * 复制原元素轮廓（宽、高、圆角、形状）
 * 先做偏移（x/y）
 * 再执行 spread 缩放（向外膨胀 spread 像素）
 * 得到一个实心纯色轮廓（阴影底色）
 * 最后对这个膨胀后的实心图形做 blur-radius 高斯模糊
 */

/* reference : file:///M:/test/ESDBox_IPGUI/tools/box_shadow_algorithm_visual.html
 */

#include "ipgui_draw_box_shadow.h"
#include "ipgui_utils.h"
#include <math.h>
#include <stdlib.h>

/* ---------- 高斯核生成（u16 定点，65535 = 1.0） ---------- */
static void gen_gauss_kernel(u16_t * kernel, int radius, double sigma)
{
    int   n = 2 * radius + 1;
    double sum = 0.0;
    double *tmp = (double *)malloc((size_t)n * sizeof(double));

    for (int i = 0; i < n; i++) {
        int x = i - radius;
        tmp[i] = exp(-0.5 * x * x / (sigma * sigma));
        sum += tmp[i];
    }
    for (int i = 0; i < n; i++)
        kernel[i] = (u16_t)(tmp[i] / sum * 65535.0 + 0.5);

    free(tmp);
}

/* ---------- 纯矩形阴影测试（无圆角/spread/offset） ---------- */
void ipgui_draw_box_shadow_test(
    ipgui_surf_t  * surf,
    ipgui_coord_t   box_x, ipgui_coord_t box_y,
    ipgui_coord_t   box_w, ipgui_coord_t box_h,
    ipgui_coord_t   blur,
    ipgui_color_t   shadow_color, u8_t opacity)
{
    if (!surf || box_w <= 0 || box_h <= 0) return;

    /* ---- 1. 计算高斯核 ---- */
    double sigma = (double)blur / 3.0;
    int kr;                                /* 核半径 */
    if (blur <= 0) kr = 0;
    else {
        kr = (int)ceil(sigma * 3.0);
        if (kr < 1) kr = 1;
    }
    int kn = 2 * kr + 1;                   /* 核长度 */

    u16_t * kernel = (u16_t *)malloc((size_t)kn * sizeof(u16_t));
    if (!kernel) return;
    gen_gauss_kernel(kernel, kr, sigma);

    /* ---- 2. 阴影包围盒（盒子 + 四周 blur padding） ----
     * 注意：mask/blur 缓冲不能裁剪到 surf 边界。
     * 原因：可分离高斯模糊的垂直 pass 需要相邻行的 kernel 上下文。
     * 若 sbbox 被 surf 裁剪 → 切片边界的 mask 数据缺失 → 模糊不连续。
     * ipgui_blend_color 内部已做 surf 裁剪，所以 dest 超出 surf 也安全。 */
    ipgui_aabb_t sbbox;
    sbbox.start.x = box_x - kr;
    sbbox.start.y = box_y - kr;
    sbbox.end.x   = box_x + box_w - 1 + kr;
    sbbox.end.y   = box_y + box_h - 1 + kr;

    ipgui_coord_t W = sbbox.end.x - sbbox.start.x + 1;
    ipgui_coord_t H = sbbox.end.y - sbbox.start.y + 1;
    if (W <= 0 || H <= 0) { free(kernel); return; }

    /* ---- 3. 分配全缓冲 ---- */
    u8_t  * mask   = (u8_t *)malloc((size_t)(W * H) * sizeof(u8_t));
    u16_t * horiz  = (u16_t *)malloc((size_t)(W * H) * sizeof(u16_t));
    u8_t  * result = (u8_t *)malloc((size_t)(W * H) * sizeof(u8_t));
    if (!mask || !horiz || !result) {
        free(mask); free(horiz); free(result); free(kernel); return;
    }

    /* ---- 4. 光栅化矩形 mask（内部 255，外部 0） ---- */
    for (ipgui_coord_t y = 0; y < H; y++) {
        ipgui_coord_t wy = sbbox.start.y + y;
        for (ipgui_coord_t x = 0; x < W; x++) {
            ipgui_coord_t wx = sbbox.start.x + x;
            mask[y * W + x] = (wx >= box_x && wx < box_x + box_w &&
                               wy >= box_y && wy < box_y + box_h) ? 255 : 0;
        }
    }

    /* ---- 5a. 水平方向一维卷积 ---- */
    for (ipgui_coord_t y = 0; y < H; y++) {
        for (ipgui_coord_t x = 0; x < W; x++) {
            u32_t sum = 0;
            for (int k = 0; k < kn; k++) {
                ipgui_coord_t sx = x + k - kr;
                if (sx < 0 || sx >= W) continue;
                sum += (u32_t)mask[y * W + sx] * kernel[k];
            }
            horiz[y * W + x] = (u16_t)((sum + 32768) >> 16);
        }
    }

    /* ---- 5b. 垂直方向一维卷积 ---- */
    for (ipgui_coord_t y = 0; y < H; y++) {
        for (ipgui_coord_t x = 0; x < W; x++) {
            u32_t sum = 0;
            for (int k = 0; k < kn; k++) {
                ipgui_coord_t sy = y + k - kr;
                if (sy < 0 || sy >= H) continue;
                sum += (u32_t)horiz[sy * W + x] * kernel[k];
            }
            result[y * W + x] = (u8_t)((sum + 32768) >> 16);
        }
    }

    /* ---- 6. 使用框架的 premultiplied-alpha 混合渲染阴影 ---- */
    /*      框架统一用 premultiplied alpha，手动 RGBA 写入会与已有半透明区域冲突 */
    {
        ipgui_aabb_t mask_aabb;
        mask_aabb.start.x = sbbox.start.x;
        mask_aabb.start.y = sbbox.start.y;
        mask_aabb.end.x   = sbbox.end.x;
        mask_aabb.end.y   = sbbox.end.y;

        ipgui_blend_color(
            surf,
            (ipgui_aabb_t *)0,   /* 无额外裁剪 */
            &sbbox,
            shadow_color,
            opacity,
            result,              /* 卷积结果作为逐像素 alpha 掩码 */
            &mask_aabb,
            IPGUI_BLEND_NORMAL);
    }

    /* ---- 7. 释放 ---- */
    free(result);
    free(horiz);
    free(mask);
    free(kernel);
}

