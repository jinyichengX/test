#ifndef IPGUI_PIXEL_LERP_H
#define IPGUI_PIXEL_LERP_H

#include "ipgui_blend_image.h"

/*  像素格式插值（函数指针 + 索引表）
 *
 * 屏幕坐标系说明（左上角为原点，x向右，y向下）：
 *
 *   (0,0)x -→
 *     y a  ---hd-->  b          a=(x  , y  )  左上
 *     ↓ |            |          b=(x+1, y  )  右上
 *       vd          vd          c=(x  , y+1)  左下
 *       |            |          d=(x+1, y+1)  右下
 *       c  ---hd-->  d
 *
 *    hd = 采样点在 a 右侧的横向距离   (0=紧贴a, 255=紧贴b)
 *    vd = 采样点在 a 下侧的纵向距离   (0=紧贴a, 255=紧贴c)
 *
 * ---------- 线性插值（LINEAR）----------
 *
 *    a ---d--> b              a, b 为相邻像素（水平或垂直相邻均可）
 *    |         |              d = 采样点在 a→b 方向的距离 (0=紧贴a, 255=紧贴b)
 *
 * ---------- 用法 ----------
 *
 *   const ipgui_pix_lerp_t *lerp = &g_pix_lerp[img_data->fmt];
 *   lerp->bilinear(pxA, pxB, pxC, pxD, hd, vd, out);
 *   lerp->linear(pxA, pxB, d, out);
 */

typedef void (*pix_lerp_bilinear_fn)(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
                                     u8_t hd, u8_t vd, u8_t *out);
typedef void (*pix_lerp_linear_fn)(const u8_t *a, const u8_t *b, u8_t d, u8_t *out);

typedef struct {
    pix_lerp_bilinear_fn bilinear;
    pix_lerp_linear_fn   linear;
} ipgui_pix_lerp_t;

/* L8 / I8  (1 字节) */
__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_bilinear_l8(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    out[0] = (u8_t)(((u32_t)a[0] * w1 + (u32_t)b[0] * w2 + (u32_t)c[0] * w3 + (u32_t)d[0] * w4 + 32768) >> 16);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_linear_l8(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
}

/* LA88  (2 字节：L A) */
__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_bilinear_la88(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    out[0] = (u8_t)(((u32_t)a[0] * w1 + (u32_t)b[0] * w2 + (u32_t)c[0] * w3 + (u32_t)d[0] * w4 + 32768) >> 16);
    out[1] = (u8_t)(((u32_t)a[1] * w1 + (u32_t)b[1] * w2 + (u32_t)c[1] * w3 + (u32_t)d[1] * w4 + 32768) >> 16);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_linear_la88(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
}

/* RGB565  (LE: byte[0]=G[2:0]B[4:0], byte[1]=R[4:0]G[5:3]) */
__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_bilinear_rgb565(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
#if IPGUI_ENDIAN_LITTLE == 1
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b, pc = *(const u16_t *)c, pd = *(const u16_t *)d;
#else
    u16_t pa = (u16_t)(((u32_t)a[0] << 8) | a[1]), pb = (u16_t)(((u32_t)b[0] << 8) | b[1]),
          pc = (u16_t)(((u32_t)c[0] << 8) | c[1]), pd = (u16_t)(((u32_t)d[0] << 8) | d[1]);
#endif
    u32_t aR = (pa >> 11) << 3, aG = ((pa >> 5) & 0x3F) << 2, aB = (pa & 0x1F) << 3;
    u32_t bR = (pb >> 11) << 3, bG = ((pb >> 5) & 0x3F) << 2, bB = (pb & 0x1F) << 3;
    u32_t cR = (pc >> 11) << 3, cG = ((pc >> 5) & 0x3F) << 2, cB = (pc & 0x1F) << 3;
    u32_t dR = (pd >> 11) << 3, dG = ((pd >> 5) & 0x3F) << 2, dB = (pd & 0x1F) << 3;
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    u8_t Ro = (u8_t)((aR * w1 + bR * w2 + cR * w3 + dR * w4 + 32768) >> 16);
    u8_t Go = (u8_t)((aG * w1 + bG * w2 + cG * w3 + dG * w4 + 32768) >> 16);
    u8_t Bo = (u8_t)((aB * w1 + bB * w2 + cB * w3 + dB * w4 + 32768) >> 16);
    u8_t R5 = Ro >> 3, G6 = Go >> 2, B5 = Bo >> 3;
    out[0] = (u8_t)(((G6 & 0x07) << 5) | B5);
    out[1] = (u8_t)((R5 << 3) | (G6 >> 3));
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_linear_rgb565(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
#if IPGUI_ENDIAN_LITTLE == 1
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b;
#else
    u16_t pa = (u16_t)(((u32_t)a[0] << 8) | a[1]), pb = (u16_t)(((u32_t)b[0] << 8) | b[1]);
#endif
    u32_t aR = (pa >> 11) << 3, aG = ((pa >> 5) & 0x3F) << 2, aB = (pa & 0x1F) << 3;
    u32_t bR = (pb >> 11) << 3, bG = ((pb >> 5) & 0x3F) << 2, bB = (pb & 0x1F) << 3;
    u32_t id = 255 - d;
    u8_t Ro = (u8_t)((aR * id + bR * d + 128) >> 8);
    u8_t Go = (u8_t)((aG * id + bG * d + 128) >> 8);
    u8_t Bo = (u8_t)((aB * id + bB * d + 128) >> 8);
    u8_t R5 = Ro >> 3, G6 = Go >> 2, B5 = Bo >> 3;
    out[0] = (u8_t)(((G6 & 0x07) << 5) | B5);
    out[1] = (u8_t)((R5 << 3) | (G6 >> 3));
}

/* BGR565(LE: byte[0]=G[2:0]R[4:0], byte[1]=B[4:0]G[5:3]) */
__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_bilinear_bgr565(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
#if IPGUI_ENDIAN_LITTLE == 1
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b, pc = *(const u16_t *)c, pd = *(const u16_t *)d;
#else
    u16_t pa = (u16_t)(((u32_t)a[0] << 8) | a[1]), pb = (u16_t)(((u32_t)b[0] << 8) | b[1]),
          pc = (u16_t)(((u32_t)c[0] << 8) | c[1]), pd = (u16_t)(((u32_t)d[0] << 8) | d[1]);
#endif
    u32_t aB = (pa >> 11) << 3, aG = ((pa >> 5) & 0x3F) << 2, aR = (pa & 0x1F) << 3;
    u32_t bB = (pb >> 11) << 3, bG = ((pb >> 5) & 0x3F) << 2, bR = (pb & 0x1F) << 3;
    u32_t cB = (pc >> 11) << 3, cG = ((pc >> 5) & 0x3F) << 2, cR = (pc & 0x1F) << 3;
    u32_t dB = (pd >> 11) << 3, dG = ((pd >> 5) & 0x3F) << 2, dR = (pd & 0x1F) << 3;
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    u8_t Bo = (u8_t)((aB * w1 + bB * w2 + cB * w3 + dB * w4 + 32768) >> 16);
    u8_t Go = (u8_t)((aG * w1 + bG * w2 + cG * w3 + dG * w4 + 32768) >> 16);
    u8_t Ro = (u8_t)((aR * w1 + bR * w2 + cR * w3 + dR * w4 + 32768) >> 16);
    u8_t B5 = Bo >> 3, G6 = Go >> 2, R5 = Ro >> 3;
    out[0] = (u8_t)(((G6 & 0x07) << 5) | R5);
    out[1] = (u8_t)((B5 << 3) | (G6 >> 3));
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_linear_bgr565(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
#if IPGUI_ENDIAN_LITTLE == 1
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b;
#else
    u16_t pa = (u16_t)(((u32_t)a[0] << 8) | a[1]), pb = (u16_t)(((u32_t)b[0] << 8) | b[1]);
#endif
    u32_t aB = (pa >> 11) << 3, aG = ((pa >> 5) & 0x3F) << 2, aR = (pa & 0x1F) << 3;
    u32_t bB = (pb >> 11) << 3, bG = ((pb >> 5) & 0x3F) << 2, bR = (pb & 0x1F) << 3;
    u32_t id = 255 - d;
    u8_t Bo = (u8_t)((aB * id + bB * d + 128) >> 8);
    u8_t Go = (u8_t)((aG * id + bG * d + 128) >> 8);
    u8_t Ro = (u8_t)((aR * id + bR * d + 128) >> 8);
    u8_t B5 = Bo >> 3, G6 = Go >> 2, R5 = Ro >> 3;
    out[0] = (u8_t)(((G6 & 0x07) << 5) | R5);
    out[1] = (u8_t)((B5 << 3) | (G6 >> 3));
}

/* RGB888  (3 字节：R G B) */
__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_bilinear_rgb888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    out[0] = (u8_t)(((u32_t)a[0] * w1 + (u32_t)b[0] * w2 + (u32_t)c[0] * w3 + (u32_t)d[0] * w4 + 32768) >> 16);
    out[1] = (u8_t)(((u32_t)a[1] * w1 + (u32_t)b[1] * w2 + (u32_t)c[1] * w3 + (u32_t)d[1] * w4 + 32768) >> 16);
    out[2] = (u8_t)(((u32_t)a[2] * w1 + (u32_t)b[2] * w2 + (u32_t)c[2] * w3 + (u32_t)d[2] * w4 + 32768) >> 16);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_linear_rgb888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
}

/* BGR888  (3 字节：B G R) */
__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_bilinear_bgr888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    out[0] = (u8_t)(((u32_t)a[0] * w1 + (u32_t)b[0] * w2 + (u32_t)c[0] * w3 + (u32_t)d[0] * w4 + 32768) >> 16);
    out[1] = (u8_t)(((u32_t)a[1] * w1 + (u32_t)b[1] * w2 + (u32_t)c[1] * w3 + (u32_t)d[1] * w4 + 32768) >> 16);
    out[2] = (u8_t)(((u32_t)a[2] * w1 + (u32_t)b[2] * w2 + (u32_t)c[2] * w3 + (u32_t)d[2] * w4 + 32768) >> 16);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_linear_bgr888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
}

/* ARGB8888  (4 字节，LE: B G R A) */
__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_bilinear_argb8888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    out[0] = (u8_t)(((u32_t)a[0] * w1 + (u32_t)b[0] * w2 + (u32_t)c[0] * w3 + (u32_t)d[0] * w4 + 32768) >> 16);
    out[1] = (u8_t)(((u32_t)a[1] * w1 + (u32_t)b[1] * w2 + (u32_t)c[1] * w3 + (u32_t)d[1] * w4 + 32768) >> 16);
    out[2] = (u8_t)(((u32_t)a[2] * w1 + (u32_t)b[2] * w2 + (u32_t)c[2] * w3 + (u32_t)d[2] * w4 + 32768) >> 16);
    out[3] = (u8_t)(((u32_t)a[3] * w1 + (u32_t)b[3] * w2 + (u32_t)c[3] * w3 + (u32_t)d[3] * w4 + 32768) >> 16);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_linear_argb8888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
    out[3] = (u8_t)(((u32_t)a[3] * id + (u32_t)b[3] * d + 128) >> 8);
}

/* ABGR8888  (4 字节，LE: R G B A) */
__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_bilinear_abgr8888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    out[0] = (u8_t)(((u32_t)a[0] * w1 + (u32_t)b[0] * w2 + (u32_t)c[0] * w3 + (u32_t)d[0] * w4 + 32768) >> 16);
    out[1] = (u8_t)(((u32_t)a[1] * w1 + (u32_t)b[1] * w2 + (u32_t)c[1] * w3 + (u32_t)d[1] * w4 + 32768) >> 16);
    out[2] = (u8_t)(((u32_t)a[2] * w1 + (u32_t)b[2] * w2 + (u32_t)c[2] * w3 + (u32_t)d[2] * w4 + 32768) >> 16);
    out[3] = (u8_t)(((u32_t)a[3] * w1 + (u32_t)b[3] * w2 + (u32_t)c[3] * w3 + (u32_t)d[3] * w4 + 32768) >> 16);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_linear_abgr8888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
    out[3] = (u8_t)(((u32_t)a[3] * id + (u32_t)b[3] * d + 128) >> 8);
}

/* RGBA8888  (4 字节，LE: R G B A) */
__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_bilinear_rgba8888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    out[0] = (u8_t)(((u32_t)a[0] * w1 + (u32_t)b[0] * w2 + (u32_t)c[0] * w3 + (u32_t)d[0] * w4 + 32768) >> 16);
    out[1] = (u8_t)(((u32_t)a[1] * w1 + (u32_t)b[1] * w2 + (u32_t)c[1] * w3 + (u32_t)d[1] * w4 + 32768) >> 16);
    out[2] = (u8_t)(((u32_t)a[2] * w1 + (u32_t)b[2] * w2 + (u32_t)c[2] * w3 + (u32_t)d[2] * w4 + 32768) >> 16);
    out[3] = (u8_t)(((u32_t)a[3] * w1 + (u32_t)b[3] * w2 + (u32_t)c[3] * w3 + (u32_t)d[3] * w4 + 32768) >> 16);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_linear_rgba8888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
    out[3] = (u8_t)(((u32_t)a[3] * id + (u32_t)b[3] * d + 128) >> 8);
}

/* BGRA8888  (4 字节，LE: B G R A) */
__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_bilinear_bgra8888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    out[0] = (u8_t)(((u32_t)a[0] * w1 + (u32_t)b[0] * w2 + (u32_t)c[0] * w3 + (u32_t)d[0] * w4 + 32768) >> 16);
    out[1] = (u8_t)(((u32_t)a[1] * w1 + (u32_t)b[1] * w2 + (u32_t)c[1] * w3 + (u32_t)d[1] * w4 + 32768) >> 16);
    out[2] = (u8_t)(((u32_t)a[2] * w1 + (u32_t)b[2] * w2 + (u32_t)c[2] * w3 + (u32_t)d[2] * w4 + 32768) >> 16);
    out[3] = (u8_t)(((u32_t)a[3] * w1 + (u32_t)b[3] * w2 + (u32_t)c[3] * w3 + (u32_t)d[3] * w4 + 32768) >> 16);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void pix_linear_bgra8888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
    out[3] = (u8_t)(((u32_t)a[3] * id + (u32_t)b[3] * d + 128) >> 8);
}

/* 索引表 */
__IPGUI_STATIC__ const ipgui_pix_lerp_t g_pix_lerp[IPGUI_IMG_FMT_MAX] = {
    [IPGUI_IMG_FMT_L8]       = { pix_bilinear_l8,       pix_linear_l8       },
    [IPGUI_IMG_FMT_LA88]     = { pix_bilinear_la88,     pix_linear_la88     },
    [IPGUI_IMG_FMT_RGB565]   = { pix_bilinear_rgb565,   pix_linear_rgb565   },
    [IPGUI_IMG_FMT_BGR565]   = { pix_bilinear_bgr565,   pix_linear_bgr565   },
    [IPGUI_IMG_FMT_RGB888]   = { pix_bilinear_rgb888,   pix_linear_rgb888   },
    [IPGUI_IMG_FMT_BGR888]   = { pix_bilinear_bgr888,   pix_linear_bgr888   },
    [IPGUI_IMG_FMT_ARGB8888] = { pix_bilinear_argb8888, pix_linear_argb8888 },
    [IPGUI_IMG_FMT_ABGR8888] = { pix_bilinear_abgr8888, pix_linear_abgr8888 },
    [IPGUI_IMG_FMT_RGBA8888] = { pix_bilinear_rgba8888, pix_linear_rgba8888 },
    [IPGUI_IMG_FMT_BGRA8888] = { pix_bilinear_bgra8888, pix_linear_bgra8888 }
};

#endif

