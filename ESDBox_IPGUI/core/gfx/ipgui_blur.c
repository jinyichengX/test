/*
 * ipgui_blur.c - Alpha box blur for GUI rendering
 *
 * Based on Mozilla's AlphaBoxBlur algorithm (Blur.cpp).
 * 3-pass box blur approximates Gaussian blur (central limit theorem).
 * Sliding window approach, no integral image.
 */

#include "ipgui_blur.h"
#include "ipgui_mask_buf.h"
#include "ipgui_memory.h"

/*
 * 定点倒数除法：避免逐像素整数除法（Cortex-M 除法慢/需软件除法）。
 *
 * out = sum / kn（向下取整），用 (sum * mul) >> shift 等价实现。
 * 精确条件：2^shift > 255 * kn^2（sum <= 255*kn，误差 < 1/kn，floor 不变）。
 * kn > 256 时 shift 封顶 24，结果可能差 1（模糊场景视觉不可见，实际用不到）。
 */
static inline void ipgui_blur_recip(u32_t kn, u32_t * mul, u32_t * shift)
{
    u32_t k     = (kn > 256u) ? 256u : kn;  /* 防 255*k*k 溢出，且 s<=24 */
    u32_t limit = 255u * k * k;
    u32_t s     = 24;

    while (s > 0 && (((u32_t)1u << (s - 1u)) > limit)) {
        s --;
    }
    * shift = s;
    * mul   = (((u32_t)1u << s) + kn - 1u) / kn;  /* ceil(2^s / kn) */
}

/* sum / kn 的定点实现：返回 (sum * mul) >> shift */
static inline u8_t ipgui_blur_mean(s32_t sum, u32_t mul, u32_t shift)
{
    return (u8_t)(((u32_t)sum * mul) >> shift);
}

/*
 * average blur (box blur / mean blur)
 *
 * 三阶段滑动窗口，zero-padding（越界取 0）：
 *   1. 左边缘：leaving 越界取 0
 *   2. 中心：leaving 和 entering 都是真实像素，4x 展开
 *   3. 右边缘：entering 越界取 0
 *
 * out[x] = in[max(0,x-kn_size+1) ~ min(in_w-1,x)] 的均值（越界取 0）
 * 有效输出宽度 = min(out->w, in->w + kn_size - 1)
 * 有效输出高度 = min(in->h, out->h)
 *
 * 支持 in-place（in->mask == out->mask 且 in->w == out->w）：
 * 使用 kn_size 字节环形缓冲区，在覆盖前保存原始值。
 */
__IPGUI_API__ void ipgui_average_blur_hor(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          kn_size, /* average blur kernel size */
    ipgui_mask_surface_t * out)
{
    if (!in || !in->mask || !out || !out->mask) return;
    if (kn_size <= 0) return;

    /* 定点倒数参数，替代逐像素除法 */
    u32_t mul;
    u32_t shift;
    ipgui_blur_recip((u32_t)kn_size, &mul, &shift);

    ipgui_coord_t in_w  = in->w;
    ipgui_coord_t in_h  = in->h;
    ipgui_coord_t out_w = out->w;
    ipgui_coord_t out_h = out->h;

    ipgui_coord_t eff_h  = (in_h < out_h) ? in_h : out_h;
    ipgui_coord_t full_w = in_w + kn_size - 1;
    ipgui_coord_t eff_w  = (out_w < full_w) ? out_w : full_w;

    u8_t in_place = (in->mask == out->mask && in_w == out_w);

    if (in_place) {
        /* ---- in-place 路径：环形缓冲区 ---- */
        ipgui_coord_t ring_h;
        u8_t * ring = ipgui_mask_buf_acquire(kn_size, 1, &ring_h);
        if (!ring || ring_h != 1) {
            if (ring) ipgui_mask_buf_free(ring);
            return;
        }

        for (ipgui_coord_t y = 0; y < eff_h; y ++) {
            u8_t * row = out->mask + (uintptr_t)y * out_w;
            ipgui_memset(ring, 0, (u32_t)kn_size);

            s32_t sum = (s32_t)row[0];
            ring[0] = row[0];
            row[0]  = ipgui_blur_mean(sum, mul, shift);

            ipgui_coord_t c_start = kn_size;
            ipgui_coord_t c_end   = in_w;
            ipgui_coord_t x = 1;
            ipgui_coord_t ri = 1 % kn_size;

            /* Phase 1: 左边缘（leaving 越界取 0） */
            for (; x < c_start && x < eff_w; x ++) {
                s32_t enter = (s32_t)row[x];
                ring[ri] = row[x];
                ri ++; if (ri >= kn_size) ri = 0;
                sum += enter - 0;
                row[x] = ipgui_blur_mean(sum, mul, shift);
            }

            /* Phase 2: 中心 */
            while (x < c_end && x < eff_w) {
                s32_t enter = (s32_t)row[x];
                s32_t leave = ring[ri];
                ring[ri] = row[x];
                ri ++; if (ri >= kn_size) ri = 0;
                sum += enter - leave;
                row[x] = ipgui_blur_mean(sum, mul, shift);
                x ++;
            }

            /* Phase 3: 右边缘（entering 越界取 0） */
            for (; x < eff_w; x ++) {
                s32_t leave = ring[ri];
                ri ++; if (ri >= kn_size) ri = 0;
                sum += 0 - leave;
                row[x] = ipgui_blur_mean(sum, mul, shift);
            }
        }

        ipgui_mask_buf_free(ring);

    } else {
        /* ---- 非原地路径：滑动窗口 ---- */
        for (ipgui_coord_t y = 0; y < eff_h; y ++) {
            const u8_t * src = in->mask + (uintptr_t)y * in_w;
            u8_t       * dst = out->mask + (uintptr_t)y * out_w;

            /* 初始 sum：窗口 [-kn_size+1, 0]，越界部分取 0 */
            s32_t sum = (s32_t)src[0];
            dst[0] = ipgui_blur_mean(sum, mul, shift);

            /* 阶段边界 */
            ipgui_coord_t c_start = kn_size;  /* x >= kn_size: leave_idx >= 0 */
            ipgui_coord_t c_end   = in_w;     /* x < in_w: enter_idx < in_w   */

            ipgui_coord_t x = 1;

            /* Phase 1: 左边缘（leaving 越界取 0） */
            for (; x < c_start && x < eff_w; x ++) {
                s32_t enter = (x < in_w) ? (s32_t)src[x] : 0;
                sum += enter - 0;
                dst[x] = ipgui_blur_mean(sum, mul, shift);
            }

            /* Phase 2: 中心（4x 展开） */
            while (x + 4 <= c_end && x + 4 <= eff_w) {
                sum += (s32_t)src[x] - (s32_t)src[x - kn_size];
                dst[x] = ipgui_blur_mean(sum, mul, shift); x ++;
                sum += (s32_t)src[x] - (s32_t)src[x - kn_size];
                dst[x] = ipgui_blur_mean(sum, mul, shift); x ++;
                sum += (s32_t)src[x] - (s32_t)src[x - kn_size];
                dst[x] = ipgui_blur_mean(sum, mul, shift); x ++;
                sum += (s32_t)src[x] - (s32_t)src[x - kn_size];
                dst[x] = ipgui_blur_mean(sum, mul, shift); x ++;
            }
            while (x < c_end && x < eff_w) {
                sum += (s32_t)src[x] - (s32_t)src[x - kn_size];
                dst[x] = ipgui_blur_mean(sum, mul, shift); x ++;
            }

            /* Phase 3: 右边缘（entering 越界取 0） */
            for (; x < eff_w; x ++) {
                sum += 0 - (s32_t)src[x - kn_size];
                dst[x] = ipgui_blur_mean(sum, mul, shift);
            }
        }
    }

    /* 越界区域置 0：每行 eff_w 之后的像素 */
    for (ipgui_coord_t y = 0; y < eff_h; y ++) {
        u8_t * dst = out->mask + (uintptr_t)y * out_w;
        ipgui_memset(dst + eff_w, 0, (u32_t)(out_w - eff_w));
    }

    /* 越界行置 0：eff_h 之后的所有行 */
    if (out_h > eff_h) {
        u8_t * dst = out->mask + (uintptr_t)eff_h * out_w;
        ipgui_memset(dst, 0, (u32_t)((u32_t)out_w * (u32_t)(out_h - eff_h)));
    }
}

__IPGUI_API__ void ipgui_average_blur_ver(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          kn_size, /* average blur kernel size */
    ipgui_mask_surface_t * out)
{
    if (!in || !in->mask || !out || !out->mask) return;
    if (kn_size <= 0) return;

    /* 定点倒数参数，替代逐像素除法 */
    u32_t mul;
    u32_t shift;
    ipgui_blur_recip((u32_t)kn_size, &mul, &shift);

    ipgui_coord_t in_w  = in->w;
    ipgui_coord_t in_h  = in->h;
    ipgui_coord_t out_w = out->w;
    ipgui_coord_t out_h = out->h;

    ipgui_coord_t eff_w  = (in_w < out_w) ? in_w : out_w;
    ipgui_coord_t full_h = in_h + kn_size - 1;
    ipgui_coord_t eff_h  = (out_h < full_h) ? out_h : full_h;

    u8_t in_place = (in->mask == out->mask && in_w == out_w);

    if (in_place) {
        /* ---- in-place 路径：环形缓冲区 ---- */
        ipgui_coord_t ring_h;
        u8_t * ring = ipgui_mask_buf_acquire(kn_size, 1, &ring_h);
        if (!ring || ring_h != 1) {
            if (ring) ipgui_mask_buf_free(ring);
            return;
        }

        for (ipgui_coord_t x = 0; x < eff_w; x ++) {
            u8_t * col = out->mask + (uintptr_t)x;
            ipgui_memset(ring, 0, (u32_t)kn_size);

            s32_t sum = (s32_t)col[0];
            ring[0] = col[0];
            col[0] = ipgui_blur_mean(sum, mul, shift);

            ipgui_coord_t c_start = kn_size;
            ipgui_coord_t c_end   = in_h;
            ipgui_coord_t y = 1;
            ipgui_coord_t ri = 1 % kn_size;

            /* Phase 1: 上边缘（leaving 越界取 0） */
            for (; y < c_start && y < eff_h; y ++) {
                s32_t enter = (y < in_h) ? (s32_t)col[(uintptr_t)y * out_w] : 0;
                ring[ri] = (u8_t)enter;
                ri ++; if (ri >= kn_size) ri = 0;
                sum += enter - 0;
                col[(uintptr_t)y * out_w] = ipgui_blur_mean(sum, mul, shift);
            }

            /* Phase 2: 中心 */
            while (y < c_end && y < eff_h) {
                s32_t enter = (s32_t)col[(uintptr_t)y * out_w];
                s32_t leave = ring[ri];
                ring[ri] = col[(uintptr_t)y * out_w];
                ri ++; if (ri >= kn_size) ri = 0;
                sum += enter - leave;
                col[(uintptr_t)y * out_w] = ipgui_blur_mean(sum, mul, shift);
                y ++;
            }

            /* Phase 3: 下边缘（entering 越界取 0） */
            for (; y < eff_h; y ++) {
                s32_t leave = ring[ri];
                ri ++; if (ri >= kn_size) ri = 0;
                sum += 0 - leave;
                col[(uintptr_t)y * out_w] = ipgui_blur_mean(sum, mul, shift);
            }
        }

        ipgui_mask_buf_free(ring);

    } else {
        /* ---- 非原地路径：滑动窗口 ---- */
        for (ipgui_coord_t x = 0; x < eff_w; x ++) {
            const u8_t * src = in->mask + (uintptr_t)x;
            u8_t       * dst = out->mask + (uintptr_t)x;

            /* 初始 sum：窗口 [-kn_size+1, 0]，越界部分取 0 */
            s32_t sum = (s32_t)src[0];
            dst[0] = ipgui_blur_mean(sum, mul, shift);

            /* 阶段边界 */
            ipgui_coord_t c_start = kn_size;  /* y >= kn_size: leave_idx >= 0 */
            ipgui_coord_t c_end   = in_h;     /* y < in_h: enter_idx < in_h   */

            ipgui_coord_t y = 1;

            /* Phase 1: 上边缘（leaving 越界取 0） */
            for (; y < c_start && y < eff_h; y ++) {
                s32_t enter = (y < in_h) ? (s32_t)src[(uintptr_t)y * in_w] : 0;
                sum += enter - 0;
                dst[(uintptr_t)y * out_w] = ipgui_blur_mean(sum, mul, shift);
            }

            /* Phase 2: 中心（4x 展开） */
            while (y + 4 <= c_end && y + 4 <= eff_h) {
                sum += (s32_t)src[(uintptr_t)y * in_w] - (s32_t)src[(uintptr_t)(y - kn_size) * in_w];
                dst[(uintptr_t)y * out_w] = ipgui_blur_mean(sum, mul, shift); y ++;
                sum += (s32_t)src[(uintptr_t)y * in_w] - (s32_t)src[(uintptr_t)(y - kn_size) * in_w];
                dst[(uintptr_t)y * out_w] = ipgui_blur_mean(sum, mul, shift); y ++;
                sum += (s32_t)src[(uintptr_t)y * in_w] - (s32_t)src[(uintptr_t)(y - kn_size) * in_w];
                dst[(uintptr_t)y * out_w] = ipgui_blur_mean(sum, mul, shift); y ++;
                sum += (s32_t)src[(uintptr_t)y * in_w] - (s32_t)src[(uintptr_t)(y - kn_size) * in_w];
                dst[(uintptr_t)y * out_w] = ipgui_blur_mean(sum, mul, shift); y ++;
            }
            while (y < c_end && y < eff_h) {
                sum += (s32_t)src[(uintptr_t)y * in_w] - (s32_t)src[(uintptr_t)(y - kn_size) * in_w];
                dst[(uintptr_t)y * out_w] = ipgui_blur_mean(sum, mul, shift); y++;
            }

            /* Phase 3: 下边缘（entering 越界取 0） */
            for (; y < eff_h; y ++) {
                sum += 0 - (s32_t)src[(uintptr_t)(y - kn_size) * in_w];
                dst[(uintptr_t)y * out_w] = ipgui_blur_mean(sum, mul, shift);
            }
        }
    }

    /* 越界区域置 0：每列 eff_h 之后的像素（strided，不能用 memset） */
    for (ipgui_coord_t x = 0; x < eff_w; x ++) {
        for (ipgui_coord_t y = eff_h; y < out_h; y ++) {
            out->mask[(uintptr_t)y * out_w + x] = 0;
        }
    }

    /* 越界列置 0：eff_w 之后的所有列（每行尾部连续，用 memset） */
    for (ipgui_coord_t y = 0; y < out_h; y ++) {
        u8_t * row = out->mask + (uintptr_t)y * out_w;
        ipgui_memset(row + eff_w, 0, (u32_t)(out_w - eff_w));
    }
}

__IPGUI_API__ void ipgui_blur(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          h_kn_size, /* horizontal blur kernel size */
    ipgui_coord_t          v_kn_size, /* vertical blur kernel size */
    ipgui_mask_surface_t * out)
{
    
}