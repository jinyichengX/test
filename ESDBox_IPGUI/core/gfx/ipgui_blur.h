/*
 * ipgui_blur.h - Alpha box blur for GUI rendering
 *
 * 3-pass box blur approximates Gaussian blur (central limit theorem).
 * Sliding window approach, no integral image.
 */

#ifndef IPGUI_BLUR_H
#define IPGUI_BLUR_H

#include "ipgui_types.h"
#include "ipgui_coord.h"
#include "ipgui_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A8 surface: 8-bit alpha-only, row-major, stride == width */
typedef struct {
    u8_t           * mask;
    ipgui_coord_t    w;
    ipgui_coord_t    h;
} ipgui_mask_surface_t;

extern __IPGUI_API__ void ipgui_average_blur_hor(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          kn_size, /* average blur kernel size */
    ipgui_mask_surface_t * out);

extern __IPGUI_API__ void ipgui_average_blur_ver(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          kn_size, /* average blur kernel size */
    ipgui_mask_surface_t * out);

/*
 * 居中对称 box blur（zero-padding）
 * out 尺寸与 in 相同；适合阴影/发光等需要“向四周扩散”的场景。
 * kn_size 建议为奇数；偶数会向下取整为 kn_size-1。
 * 支持 in-place（in->mask == out->mask）。
 * 返回 0 成功，非 0 失败（通常是临时行/列缓冲申请失败）。
 */
extern __IPGUI_API__ int ipgui_average_blur_hor_centered(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          kn_size,
    ipgui_mask_surface_t * out);

extern __IPGUI_API__ int ipgui_average_blur_ver_centered(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          kn_size,
    ipgui_mask_surface_t * out);

/*
 * 分离式模糊：先水平后垂直。
 * passes:
 *   1 = 单次分离 box blur
 *   2 = 两次
 *   3 = 三次（更接近高斯）
 * 使用居中对称核，输出尺寸与 in 相同。
 * 返回 0 成功，非 0 失败。
 */
extern __IPGUI_API__ int ipgui_blur(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          h_kn_size, /* horizontal blur kernel size */
    ipgui_coord_t          v_kn_size, /* vertical blur kernel size */
    u8_t                   passes,
    ipgui_mask_surface_t * out);

#ifdef __cplusplus
}
#endif

#endif /* IPGUI_BLUR_H */
