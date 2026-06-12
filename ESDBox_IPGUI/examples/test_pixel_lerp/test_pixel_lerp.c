/* ================================================================
 * test_pixel_lerp.c — 自动校验 ipgui_pixel_lerp.h 所有插值函数
 * 不依赖项目头文件，完全自包含
 *
 * 校验方式：参考数学（uint32 与实现完全相同的公式）算期望值，
 *           与实际输出逐字节比对，无需人工验算。
 * ================================================================ */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t  u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;

/* ================================================================
 * 格式枚举（与 ipgui_blend_image.h 一致）
 * ================================================================ */
enum {
    FMT_L8 = 0, FMT_LA88, FMT_RGB565, FMT_BGR565,
    FMT_RGB888, FMT_BGR888, FMT_ARGB8888, FMT_ABGR8888,
    FMT_RGBA8888, FMT_BGRA8888, FMT_I8, FMT_MAX,
};

/* ================================================================
 * 插值函数（从 ipgui_pixel_lerp.h 完整复制）
 * ================================================================ */

/* --- L8 --- */
static void bilinear_l8(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    out[0] = (u8_t)(((u32_t)a[0] * w1 + (u32_t)b[0] * w2 + (u32_t)c[0] * w3 + (u32_t)d[0] * w4 + 32768) >> 16);
}
static void linear_l8(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
}

/* --- I8 --- */
static void bilinear_i8(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    out[0] = (u8_t)(((u32_t)a[0] * w1 + (u32_t)b[0] * w2 + (u32_t)c[0] * w3 + (u32_t)d[0] * w4 + 32768) >> 16);
}
static void linear_i8(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
}

/* --- LA88 --- */
static void bilinear_la88(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    out[0] = (u8_t)(((u32_t)a[0] * w1 + (u32_t)b[0] * w2 + (u32_t)c[0] * w3 + (u32_t)d[0] * w4 + 32768) >> 16);
    out[1] = (u8_t)(((u32_t)a[1] * w1 + (u32_t)b[1] * w2 + (u32_t)c[1] * w3 + (u32_t)d[1] * w4 + 32768) >> 16);
}
static void linear_la88(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
}

/* --- RGB565 --- */
static void bilinear_rgb565(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b, pc = *(const u16_t *)c, pd = *(const u16_t *)d;
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
static void linear_rgb565(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b;
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

/* --- BGR565 --- */
static void bilinear_bgr565(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *out)
{
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b, pc = *(const u16_t *)c, pd = *(const u16_t *)d;
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
static void linear_bgr565(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b;
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

/* --- RGB888 --- */
static void bilinear_rgb888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
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
static void linear_rgb888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
}

/* --- BGR888 --- */
static void bilinear_bgr888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
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
static void linear_bgr888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
}

/* --- ARGB8888 --- */
static void bilinear_argb8888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
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
static void linear_argb8888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
    out[3] = (u8_t)(((u32_t)a[3] * id + (u32_t)b[3] * d + 128) >> 8);
}

/* --- ABGR8888 --- */
static void bilinear_abgr8888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
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
static void linear_abgr8888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
    out[3] = (u8_t)(((u32_t)a[3] * id + (u32_t)b[3] * d + 128) >> 8);
}

/* --- RGBA8888 --- */
static void bilinear_rgba8888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
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
static void linear_rgba8888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
    out[3] = (u8_t)(((u32_t)a[3] * id + (u32_t)b[3] * d + 128) >> 8);
}

/* --- BGRA8888 --- */
static void bilinear_bgra8888(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
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
static void linear_bgra8888(const u8_t *a, const u8_t *b, u8_t d, u8_t *out)
{
    u32_t id = 255 - d;
    out[0] = (u8_t)(((u32_t)a[0] * id + (u32_t)b[0] * d + 128) >> 8);
    out[1] = (u8_t)(((u32_t)a[1] * id + (u32_t)b[1] * d + 128) >> 8);
    out[2] = (u8_t)(((u32_t)a[2] * id + (u32_t)b[2] * d + 128) >> 8);
    out[3] = (u8_t)(((u32_t)a[3] * id + (u32_t)b[3] * d + 128) >> 8);
}

/* ================================================================
 * 通用类型 / 函数指针表
 * ================================================================ */
typedef void (*lerp_bi_fn)(const u8_t*,const u8_t*,const u8_t*,const u8_t*,u8_t,u8_t,u8_t*);
typedef void (*lerp_li_fn)(const u8_t*,const u8_t*,u8_t,u8_t*);

typedef struct { lerp_bi_fn bilinear; lerp_li_fn linear; } lerp_entry_t;

static const lerp_entry_t g_lerp[FMT_MAX] = {
    [FMT_L8]       = { bilinear_l8,       linear_l8       },
    [FMT_LA88]     = { bilinear_la88,     linear_la88     },
    [FMT_RGB565]   = { bilinear_rgb565,   linear_rgb565   },
    [FMT_BGR565]   = { bilinear_bgr565,   linear_bgr565   },
    [FMT_RGB888]   = { bilinear_rgb888,   linear_rgb888   },
    [FMT_BGR888]   = { bilinear_bgr888,   linear_bgr888   },
    [FMT_ARGB8888] = { bilinear_argb8888, linear_argb8888 },
    [FMT_ABGR8888] = { bilinear_abgr8888, linear_abgr8888 },
    [FMT_RGBA8888] = { bilinear_rgba8888, linear_rgba8888 },
    [FMT_BGRA8888] = { bilinear_bgra8888, linear_bgra8888 },
    [FMT_I8]       = { bilinear_i8,       linear_i8       },
};

/* ================================================================
 * 参考数学（与实现完全相同的公式，用于计算期望值）
 * ================================================================ */

/* 通用 bilinear 参考 —— 对每个字节通道做加权平均 */
static void ref_bilinear_n(int n,
    const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *expect)
{
    u32_t w1 = (u32_t)(255 - hd) * (u32_t)(255 - vd);
    u32_t w2 = (u32_t)hd * (u32_t)(255 - vd);
    u32_t w3 = (u32_t)(255 - hd) * (u32_t)vd;
    u32_t w4 = (u32_t)hd * (u32_t)vd;
    for (int i = 0; i < n; i++)
        expect[i] = (u8_t)(((u32_t)a[i] * w1 + (u32_t)b[i] * w2 +
                            (u32_t)c[i] * w3 + (u32_t)d[i] * w4 + 32768) >> 16);
}

/* 通用 linear 参考 */
static void ref_linear_n(int n, const u8_t *a, const u8_t *b, u8_t d, u8_t *expect)
{
    u32_t id = 255 - d;
    for (int i = 0; i < n; i++)
        expect[i] = (u8_t)(((u32_t)a[i] * id + (u32_t)b[i] * d + 128) >> 8);
}

/* RGB565 参考 —— 与 pix_bilinear_rgb565 逐步骤完全一致 */
static void ref_bilinear_rgb565(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *expect)
{
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b, pc = *(const u16_t *)c, pd = *(const u16_t *)d;
    u32_t aR = (pa >> 11) << 3, aG = ((pa >> 5) & 0x3F) << 2, aB = (pa & 0x1F) << 3;
    u32_t bR = (pb >> 11) << 3, bG = ((pb >> 5) & 0x3F) << 2, bB = (pb & 0x1F) << 3;
    u32_t cR = (pc >> 11) << 3, cG = ((pc >> 5) & 0x3F) << 2, cB = (pc & 0x1F) << 3;
    u32_t dR = (pd >> 11) << 3, dG = ((pd >> 5) & 0x3F) << 2, dB = (pd & 0x1F) << 3;
    u32_t w1 = (u32_t)(255 - hd) * (u32_t)(255 - vd);
    u32_t w2 = (u32_t)hd * (u32_t)(255 - vd);
    u32_t w3 = (u32_t)(255 - hd) * (u32_t)vd;
    u32_t w4 = (u32_t)hd * (u32_t)vd;
    u8_t Ro = (u8_t)((aR * w1 + bR * w2 + cR * w3 + dR * w4 + 32768) >> 16);
    u8_t Go = (u8_t)((aG * w1 + bG * w2 + cG * w3 + dG * w4 + 32768) >> 16);
    u8_t Bo = (u8_t)((aB * w1 + bB * w2 + cB * w3 + dB * w4 + 32768) >> 16);
    u8_t R5 = Ro >> 3, G6 = Go >> 2, B5 = Bo >> 3;
    expect[0] = (u8_t)(((G6 & 0x07) << 5) | B5);
    expect[1] = (u8_t)((R5 << 3) | (G6 >> 3));
}

static void ref_linear_rgb565(const u8_t *a, const u8_t *b, u8_t d, u8_t *expect)
{
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b;
    u32_t aR = (pa >> 11) << 3, aG = ((pa >> 5) & 0x3F) << 2, aB = (pa & 0x1F) << 3;
    u32_t bR = (pb >> 11) << 3, bG = ((pb >> 5) & 0x3F) << 2, bB = (pb & 0x1F) << 3;
    u32_t id = 255 - d;
    u8_t Ro = (u8_t)((aR * id + bR * d + 128) >> 8);
    u8_t Go = (u8_t)((aG * id + bG * d + 128) >> 8);
    u8_t Bo = (u8_t)((aB * id + bB * d + 128) >> 8);
    u8_t R5 = Ro >> 3, G6 = Go >> 2, B5 = Bo >> 3;
    expect[0] = (u8_t)(((G6 & 0x07) << 5) | B5);
    expect[1] = (u8_t)((R5 << 3) | (G6 >> 3));
}

/* BGR565 参考 */
static void ref_bilinear_bgr565(const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    u8_t hd, u8_t vd, u8_t *expect)
{
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b, pc = *(const u16_t *)c, pd = *(const u16_t *)d;
    u32_t aB = (pa >> 11) << 3, aG = ((pa >> 5) & 0x3F) << 2, aR = (pa & 0x1F) << 3;
    u32_t bB = (pb >> 11) << 3, bG = ((pb >> 5) & 0x3F) << 2, bR = (pb & 0x1F) << 3;
    u32_t cB = (pc >> 11) << 3, cG = ((pc >> 5) & 0x3F) << 2, cR = (pc & 0x1F) << 3;
    u32_t dB = (pd >> 11) << 3, dG = ((pd >> 5) & 0x3F) << 2, dR = (pd & 0x1F) << 3;
    u32_t w1 = (u32_t)(255 - hd) * (u32_t)(255 - vd);
    u32_t w2 = (u32_t)hd * (u32_t)(255 - vd);
    u32_t w3 = (u32_t)(255 - hd) * (u32_t)vd;
    u32_t w4 = (u32_t)hd * (u32_t)vd;
    u8_t Bo = (u8_t)((aB * w1 + bB * w2 + cB * w3 + dB * w4 + 32768) >> 16);
    u8_t Go = (u8_t)((aG * w1 + bG * w2 + cG * w3 + dG * w4 + 32768) >> 16);
    u8_t Ro = (u8_t)((aR * w1 + bR * w2 + cR * w3 + dR * w4 + 32768) >> 16);
    u8_t B5 = Bo >> 3, G6 = Go >> 2, R5 = Ro >> 3;
    expect[0] = (u8_t)(((G6 & 0x07) << 5) | R5);
    expect[1] = (u8_t)((B5 << 3) | (G6 >> 3));
}

static void ref_linear_bgr565(const u8_t *a, const u8_t *b, u8_t d, u8_t *expect)
{
    u16_t pa = *(const u16_t *)a, pb = *(const u16_t *)b;
    u32_t aB = (pa >> 11) << 3, aG = ((pa >> 5) & 0x3F) << 2, aR = (pa & 0x1F) << 3;
    u32_t bB = (pb >> 11) << 3, bG = ((pb >> 5) & 0x3F) << 2, bR = (pb & 0x1F) << 3;
    u32_t id = 255 - d;
    u8_t Bo = (u8_t)((aB * id + bB * d + 128) >> 8);
    u8_t Go = (u8_t)((aG * id + bG * d + 128) >> 8);
    u8_t Ro = (u8_t)((aR * id + bR * d + 128) >> 8);
    u8_t B5 = Bo >> 3, G6 = Go >> 2, R5 = Ro >> 3;
    expect[0] = (u8_t)(((G6 & 0x07) << 5) | R5);
    expect[1] = (u8_t)((B5 << 3) | (G6 >> 3));
}

/* ================================================================
 * 测试框架
 * ================================================================ */
static int total_pass, total_fail;

static int bytes_eq(const u8_t *a, const u8_t *b, int n) {
    for (int i = 0; i < n; i++) if (a[i] != b[i]) return 0;
    return 1;
}

static void print_hex(const u8_t *p, int n) {
    printf("[");
    for (int i = 0; i < n; i++) printf("%s%02X", i ? " " : "", p[i]);
    printf("]");
}

/* 校验：对比 got 和 expect，打印一条结果 */
static int check(int n, const char *label,
    const u8_t *got, const u8_t *expect)
{
    if (bytes_eq(got, expect, n)) {
        printf("  %-22s  ", label);
        print_hex(got, n);
        printf("  [OK]\n");
        return 1;
    } else {
        printf("  %-22s  got=", label);
        print_hex(got, n);
        printf("  expect=");
        print_hex(expect, n);
        printf("  [FAIL]\n");
        return 0;
    }
}

/* ---- 测试 bilinear（通用字节格式）---- */
static int pass_local, fail_local;

static void test_bilinear_byte(int fmt_idx, const char *name, int nch,
    lerp_bi_fn func, const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d)
{
    u8_t got[4], expect[4];
    /* 4 个角点 + 4 个边中点 + 中心 + 额外距离 */
    static const u8_t hd_vals[] = {0, 0, 255, 255, 128, 0, 128, 255, 128, 64, 192, 51, 204};
    static const u8_t vd_vals[] = {0, 255, 0, 255, 255, 128, 0, 128, 128, 192, 64, 204, 51};
    static const char *labels[] = {
        "corner-a (hd=0,vd=0)", "corner-c (hd=0,vd=255)", "corner-b (hd=255,vd=0)",
        "corner-d (hd=255,vd=255)", "edge-cd (hd=128,vd=255)", "edge-ac (hd=0,vd=128)",
        "edge-ab (hd=128,vd=0)", "edge-bd (hd=255,vd=128)", "center (hd=128,vd=128)",
        "hd=64 vd=192", "hd=192 vd=64", "hd=51 vd=204", "hd=204 vd=51",
    };
    for (int i = 0; i < 13; i++) {
        func(a, b, c, d, hd_vals[i], vd_vals[i], got);
        ref_bilinear_n(nch, a, b, c, d, hd_vals[i], vd_vals[i], expect);
        if (!check(nch, labels[i], got, expect)) fail_local++; else pass_local++;
    }
}

/* ---- 测试 linear（通用字节格式）---- */
static void test_linear_byte(int fmt_idx, const char *name, int nch,
    lerp_li_fn func, const u8_t *a, const u8_t *b)
{
    u8_t got[4], expect[4];
    static const u8_t dvals[] = {0, 51, 102, 128, 153, 204, 255};
    static const char *labels_li[] = {
        "d=0 (==a)", "d=51 (20%)", "d=102 (40%)", "d=128 (50%)",
        "d=153 (60%)", "d=204 (80%)", "d=255 (==b)",
    };
    for (int i = 0; i < 7; i++) {
        func(a, b, dvals[i], got);
        ref_linear_n(nch, a, b, dvals[i], expect);
        if (!check(nch, labels_li[i], got, expect)) fail_local++; else pass_local++;
    }
}

/* ---- 测试 bilinear RGB565/BGR565 ---- */
static void test_bilinear_565(const char *name,
    lerp_bi_fn func, void (*ref)(const u8_t*,const u8_t*,const u8_t*,const u8_t*,u8_t,u8_t,u8_t*),
    const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d)
{
    u8_t got[2], expect[2];
    static const u8_t hd_vals[] = {0, 0, 255, 255, 128, 0, 128, 255, 128, 64, 192};
    static const u8_t vd_vals[] = {0, 255, 0, 255, 255, 128, 0, 128, 128, 192, 64};
    static const char *labels[] = {
        "corner-a", "corner-c", "corner-b", "corner-d",
        "edge-cd", "edge-ac", "edge-ab", "edge-bd", "center",
        "hd=64 vd=192", "hd=192 vd=64",
    };
    for (int i = 0; i < 11; i++) {
        func(a, b, c, d, hd_vals[i], vd_vals[i], got);
        ref(a, b, c, d, hd_vals[i], vd_vals[i], expect);
        if (!check(2, labels[i], got, expect)) fail_local++; else pass_local++;
    }
}

static void test_linear_565(const char *name,
    lerp_li_fn func, void (*ref)(const u8_t*,const u8_t*,u8_t,u8_t*),
    const u8_t *a, const u8_t *b)
{
    u8_t got[2], expect[2];
    static const u8_t dvals[] = {0, 51, 102, 128, 153, 204, 255};
    static const char *labels[] = {"d=0","d=51","d=102","d=128","d=153","d=204","d=255"};
    for (int i = 0; i < 7; i++) {
        func(a, b, dvals[i], got);
        ref(a, b, dvals[i], expect);
        if (!check(2, labels[i], got, expect)) fail_local++; else pass_local++;
    }
}

/* ================================================================
 * 测试数据 —— 所有像素值均非零，每个格式有语义明确的测试输入
 * ================================================================ */

static void test_format(int fmt_idx, const char *name, int nch,
    lerp_bi_fn bifunc, lerp_li_fn lifunc,
    const u8_t *a, const u8_t *b, const u8_t *c, const u8_t *d,
    const u8_t *la, const u8_t *lb,
    int is_565, int is_bgr565)
{
    pass_local = fail_local = 0;
    printf("\n-----  %-8s  (%d-ch)  -----\n", name, nch);
    printf("  bilinear a="); print_hex(a, nch);
    printf(" b="); print_hex(b, nch);
    printf(" c="); print_hex(c, nch);
    printf(" d="); print_hex(d, nch); printf("\n");

    if (is_565) {
        if (is_bgr565)
            test_bilinear_565(name, bifunc, ref_bilinear_bgr565, a, b, c, d);
        else
            test_bilinear_565(name, bifunc, ref_bilinear_rgb565, a, b, c, d);
    } else {
        test_bilinear_byte(fmt_idx, name, nch, bifunc, a, b, c, d);
    }

    printf("  linear   a="); print_hex(la, nch);
    printf("  b="); print_hex(lb, nch); printf("\n");

    if (is_565) {
        if (is_bgr565)
            test_linear_565(name, lifunc, ref_linear_bgr565, la, lb);
        else
            test_linear_565(name, lifunc, ref_linear_rgb565, la, lb);
    } else {
        test_linear_byte(fmt_idx, name, nch, lifunc, la, lb);
    }

    printf("  >> %d/%d PASS\n", pass_local, pass_local + fail_local);
    total_pass += pass_local;
    total_fail += fail_local;
}

int main(void) {
    printf("=====  ipgui_pixel_lerp.h  全格式插值自动校验 =====\n");
    printf("校验方式: 参考数学(与实现逐步骤相同)算期望值，比对实际输出\n");

    /* ---- L8 ---- */
    {
        u8_t a[1]={0x40}, b[1]={0x80}, c[1]={0xC0}, d[1]={0xE0};
        u8_t la[1]={0x40}, lb[1]={0xBF};
        test_format(FMT_L8, "L8", 1, bilinear_l8, linear_l8, a,b,c,d, la,lb, 0,0);
    }

    /* ---- LA88 ---- */
    {
        u8_t a[2]={0x33,0x88}, b[2]={0xCC,0x55}, c[2]={0x66,0x11}, d[2]={0x99,0xF0};
        u8_t la[2]={0x33,0x88}, lb[2]={0xCC,0x55};
        test_format(FMT_LA88, "LA88", 2, bilinear_la88, linear_la88, a,b,c,d, la,lb, 0,0);
    }

    /* ---- RGB565 ---- */
    {
        u8_t a[2]={0x00,0xF8}, b[2]={0xE0,0x07}, c[2]={0x1F,0x00}, d[2]={0xFF,0xFF};
        test_format(FMT_RGB565, "RGB565", 2, bilinear_rgb565, linear_rgb565, a,b,c,d, a,b, 1,0);
    }

    /* ---- BGR565 ---- */
    {
        u8_t a[2]={0x00,0xF8}, b[2]={0xE0,0x07}, c[2]={0x1F,0x00}, d[2]={0xFF,0xFF};
        test_format(FMT_BGR565, "BGR565", 2, bilinear_bgr565, linear_bgr565, a,b,c,d, a,b, 1,1);
    }

    /* ---- RGB888 ---- */
    {
        u8_t a[3]={0xFE,0x10,0x10}, b[3]={0x10,0xFE,0x10}, c[3]={0x10,0x10,0xFE}, d[3]={0xFE,0xFE,0xFE};
        test_format(FMT_RGB888, "RGB888", 3, bilinear_rgb888, linear_rgb888, a,b,c,d, a,b, 0,0);
    }

    /* ---- BGR888 ---- */
    {
        u8_t a[3]={0xFE,0x10,0x10}, b[3]={0x10,0xFE,0x10}, c[3]={0x10,0x10,0xFE}, d[3]={0xFE,0xFE,0xFE};
        test_format(FMT_BGR888, "BGR888", 3, bilinear_bgr888, linear_bgr888, a,b,c,d, a,b, 0,0);
    }

    /* ---- ARGB8888 ---- */
    {
        u8_t a[4]={0x10,0x10,0xFE,0xFE}, b[4]={0x10,0xFE,0x10,0xFE};
        u8_t c[4]={0xFE,0x10,0x10,0xFE}, d[4]={0xFE,0xFE,0xFE,0xFE};
        test_format(FMT_ARGB8888, "ARGB8888", 4, bilinear_argb8888, linear_argb8888, a,b,c,d, a,b, 0,0);
    }

    /* ---- ABGR8888 ---- */
    {
        u8_t a[4]={0xFE,0x10,0x10,0xFE}, b[4]={0x10,0xFE,0x10,0xFE};
        u8_t c[4]={0x10,0x10,0xFE,0xFE}, d[4]={0xFE,0xFE,0xFE,0xFE};
        test_format(FMT_ABGR8888, "ABGR8888", 4, bilinear_abgr8888, linear_abgr8888, a,b,c,d, a,b, 0,0);
    }

    /* ---- RGBA8888 ---- */
    {
        u8_t a[4]={0xFE,0x10,0x10,0xFE}, b[4]={0x10,0xFE,0x10,0xFE};
        u8_t c[4]={0x10,0x10,0xFE,0xFE}, d[4]={0xFE,0xFE,0xFE,0xFE};
        test_format(FMT_RGBA8888, "RGBA8888", 4, bilinear_rgba8888, linear_rgba8888, a,b,c,d, a,b, 0,0);
    }

    /* ---- BGRA8888 ---- */
    {
        u8_t a[4]={0x10,0x10,0xFE,0xFE}, b[4]={0x10,0xFE,0x10,0xFE};
        u8_t c[4]={0xFE,0x10,0x10,0xFE}, d[4]={0xFE,0xFE,0xFE,0xFE};
        test_format(FMT_BGRA8888, "BGRA8888", 4, bilinear_bgra8888, linear_bgra8888, a,b,c,d, a,b, 0,0);
    }

    /* ---- I8 ---- */
    {
        u8_t a[1]={0x20}, b[1]={0x50}, c[1]={0xA0}, d[1]={0xD0};
        u8_t la[1]={0x20}, lb[1]={0x9F};
        test_format(FMT_I8, "I8", 1, bilinear_i8, linear_i8, a,b,c,d, la,lb, 0,0);
    }

    /* ---- 函数指针 LUT 遍历 ---- */
    printf("\n-----  LUT TABLE  -----\n");
    {
        int lpass = 0, lfail = 0;
        static const char *names[] = {"L8","LA88","RGB565","BGR565","RGB888","BGR888",
            "ARGB8888","ABGR8888","RGBA8888","BGRA8888","I8"};
        static const int nch[] = {1,2,2,2,3,3,4,4,4,4,1};
        u8_t px[4]={0x40,0x80,0xC0,0xFE}, got[4], expect[4];
        for (int i = 0; i < 11; i++) {
            const lerp_entry_t *e = &g_lerp[i];
            int nc = nch[i];
            if (!e->bilinear || !e->linear) {
                printf("  %-8s NULL func  [FAIL]\n", names[i]);
                lfail++; continue;
            }
            /* bilinear: 4相同像素 + hd=64,vd=192 */
            e->bilinear(px, px, px, px, 64, 192, got);
            if (i == FMT_RGB565)
                ref_bilinear_rgb565(px, px, px, px, 64, 192, expect);
            else if (i == FMT_BGR565)
                ref_bilinear_bgr565(px, px, px, px, 64, 192, expect);
            else
                ref_bilinear_n(nc, px, px, px, px, 64, 192, expect);
            if (bytes_eq(got, expect, nc)) lpass++; else lfail++;

            /* linear: d=128 */
            e->linear(px, px, 128, got);
            if (i == FMT_RGB565)
                ref_linear_rgb565(px, px, 128, expect);
            else if (i == FMT_BGR565)
                ref_linear_bgr565(px, px, 128, expect);
            else
                ref_linear_n(nc, px, px, 128, expect);
            if (bytes_eq(got, expect, nc)) lpass++; else lfail++;
        }
        printf("  LUT dispatch: %d/%d PASS\n", lpass, lpass + lfail);
        total_pass += lpass;
        total_fail += lfail;
    }

    /* ---- 总结果 ---- */
    printf("\n================================================\n");
    printf("  TOTAL  PASS: %d   FAIL: %d\n", total_pass, total_fail);
    printf("================================================\n");
    return total_fail ? 1 : 0;
}
