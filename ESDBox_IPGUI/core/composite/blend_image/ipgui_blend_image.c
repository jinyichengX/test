#include "ipgui_conf.h"
#include "ipgui_blend_image.h"

typedef void (* img_px_blend_func_t)(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode);

/* 引用自 ipgui_blend_color.c 的常量定义 */
#define MASK_RB     0xf81fU   
#define MASK_G      0x07e0U   
#define MASK_MUL_RB 0x3e07c0U 
#define MASK_MUL_G  0x1f800U  

/* 无除法精确 floor(x/255): (x + 1 + x>>8) >> 8, 仅移位+加法, 比原 >>8 (÷256) 精确 */
#define DIV_255(x) (((x) + 1 + ((x) >> 8)) >> 8)

#if USE_INV_TABLE == 1
extern const u16_t g_inv_tbl[256];
#endif

/* ========================================================================== */
/* IMAGE L8 (8-bit Gray) BLENDERS                                            */
/* ========================================================================== */

static void blend_l8_2_rgb565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t a6 = (alpha + 2) >> 2;
    u32_t ia6 = 64 - a6;
    u8_t l = *src;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t bg_rb = ((ia6 * (bg & MASK_RB)) & MASK_MUL_RB) >> 6;
    u32_t bg_g  = ((ia6 * (bg & MASK_G )) & MASK_MUL_G ) >> 6;
    u32_t fg565 = ((u32_t)(l >> 3) << 11) | ((u32_t)(l >> 2) << 5) | (u32_t)(l >> 3);
    u32_t out = (((fg565 & MASK_RB) * a6) >> 6) + bg_rb;
    out |= (((fg565 & MASK_G) * a6) >> 6) + bg_g;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_l8_2_bgr565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t a6 = (alpha + 2) >> 2;
    u32_t ia6 = 64 - a6;
    u8_t l = *src;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t bg_rb = ((ia6 * (bg & MASK_RB)) & MASK_MUL_RB) >> 6;
    u32_t bg_g  = ((ia6 * (bg & MASK_G )) & MASK_MUL_G ) >> 6;
    /* BGR565: B在高位 */
    u32_t fg565 = ((u32_t)(l >> 3) << 11) | ((u32_t)(l >> 2) << 5) | (u32_t)(l >> 3);
    u32_t out = (((fg565 & MASK_RB) * a6) >> 6) + bg_rb;
    out |= (((fg565 & MASK_G) * a6) >> 6) + bg_g;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_l8_2_rgb888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t ia = 255 - alpha;
    u8_t l = *src;
    dst[0] = (u8_t)(DIV_255((u32_t)l * alpha + (u32_t)dst[0] * ia));
    dst[1] = (u8_t)(DIV_255((u32_t)l * alpha + (u32_t)dst[1] * ia));
    dst[2] = (u8_t)(DIV_255((u32_t)l * alpha + (u32_t)dst[2] * ia));
}

static void blend_l8_2_bgr888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t ia = 255 - alpha;
    u8_t l = *src;
    dst[0] = (u8_t)(DIV_255((u32_t)l * alpha + (u32_t)dst[0] * ia));
    dst[1] = (u8_t)(DIV_255((u32_t)l * alpha + (u32_t)dst[1] * ia));
    dst[2] = (u8_t)(DIV_255((u32_t)l * alpha + (u32_t)dst[2] * ia));
}

static void blend_l8_2_argb8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t cr = *(u32_t *)dst;
    u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; dr = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; dr = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; db = cr & 0xff;
#endif
    u8_t l = *src;
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)l * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)l * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)l * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (r12 << 8) | (g12 << 16) | (b12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (r12 << 16) | (g12 << 8) | b12;
#endif
}

static void blend_l8_2_abgr8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t cr = *(u32_t *)dst;
    u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; db = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; db = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; dr = cr & 0xff;
#endif
    u8_t l = *src;
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)l * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)l * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)l * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (b12 << 8) | (g12 << 16) | (r12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (b12 << 16) | (g12 << 8) | r12;
#endif
}

static void blend_l8_2_rgba8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t cr = *(u32_t *)dst;
    u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    dr = cr & 0xff; dg = (cr >> 8) & 0xff; db = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    dr = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u8_t l = *src;
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)l * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)l * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)l * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = r12 | (g12 << 8) | (b12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (r12 << 24) | (g12 << 16) | (b12 << 8) | a12;
#endif
}

static void blend_l8_2_bgra8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t cr = *(u32_t *)dst;
    u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    db = cr & 0xff; dg = (cr >> 8) & 0xff; dr = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    db = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u8_t l = *src;
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)l * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)l * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)l * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = b12 | (g12 << 8) | (r12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (b12 << 24) | (g12 << 16) | (r12 << 8) | a12;
#endif
}

/* ========================================================================== */
/* IMAGE LA88 (Luminance+Alpha) BLENDERS                                     */
/* ========================================================================== */

/* 以下 LA88 系列逻辑类似 L8，但提取源 Alpha */
static void blend_la88_2_rgb565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u8_t combined_a = (u8_t)(DIV_255((u32_t)src[1] * alpha));
    blend_l8_2_rgb565(src, dst, combined_a, blend_mode);
}

static void blend_la88_2_bgr565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u8_t combined_a = (u8_t)(DIV_255((u32_t)src[1] * alpha));
    blend_l8_2_bgr565(src, dst, combined_a, blend_mode);
}

static void blend_la88_2_rgb888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u8_t combined_a = (u8_t)(DIV_255((u32_t)src[1] * alpha));
    blend_l8_2_rgb888(src, dst, combined_a, blend_mode);
}

static void blend_la88_2_bgr888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u8_t combined_a = (u8_t)(DIV_255((u32_t)src[1] * alpha));
    blend_l8_2_bgr888(src, dst, combined_a, blend_mode);
}

static void blend_la88_2_argb8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u8_t combined_a = (u8_t)(DIV_255((u32_t)src[1] * alpha));
    blend_l8_2_argb8888(src, dst, combined_a, blend_mode);
}

static void blend_la88_2_abgr8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u8_t combined_a = (u8_t)(DIV_255((u32_t)src[1] * alpha));
    blend_l8_2_abgr8888(src, dst, combined_a, blend_mode);
}

static void blend_la88_2_rgba8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u8_t combined_a = (u8_t)(DIV_255((u32_t)src[1] * alpha));
    blend_l8_2_rgba8888(src, dst, combined_a, blend_mode);
}

static void blend_la88_2_bgra8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u8_t combined_a = (u8_t)(DIV_255((u32_t)src[1] * alpha));
    blend_l8_2_bgra8888(src, dst, combined_a, blend_mode);
}

/* ========================================================================== */
/* IMAGE RGB565 BLENDERS                                                     */
/* ========================================================================== */

static void blend_rgb565_2_rgb565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t a6 = (alpha + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src; u32_t bg = *(u16_t *)dst;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1]; u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t out_rb = (((fg & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_rgb565_2_bgr565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t a6 = (alpha + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src; u32_t bg = *(u16_t *)dst;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1]; u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    /* RGB提取 */
    u8_t r = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t b = (fg & 0x1F) << 3;
    /* 注入BGR565 */
    u32_t fg_bgr = ((u32_t)(b >> 3) << 11) | ((u32_t)(g >> 2) << 5) | (u32_t)(r >> 3);
    u32_t out_rb = (((fg_bgr & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg_bgr & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_rgb565_2_rgb888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u32_t ia = 255 - alpha;
    u8_t r = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t b = (fg & 0x1F) << 3;
    dst[0] = (u8_t)(DIV_255(r * alpha + dst[0] * ia));
    dst[1] = (u8_t)(DIV_255(g * alpha + dst[1] * ia));
    dst[2] = (u8_t)(DIV_255(b * alpha + dst[2] * ia));
}

static void blend_rgb565_2_bgr888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u32_t ia = 255 - alpha;
    u8_t r = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t b = (fg & 0x1F) << 3;
    dst[0] = (u8_t)(DIV_255(b * alpha + dst[0] * ia));
    dst[1] = (u8_t)(DIV_255(g * alpha + dst[1] * ia));
    dst[2] = (u8_t)(DIV_255(r * alpha + dst[2] * ia));
}

static void blend_rgb565_2_argb8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u8_t r = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t b = (fg & 0x1F) << 3;
    u32_t cr = *(u32_t *)dst;
    u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; dr = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; dr = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; db = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)r * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)g * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)b * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (r12 << 8) | (g12 << 16) | (b12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (r12 << 16) | (g12 << 8) | b12;
#endif
}

static void blend_rgb565_2_abgr8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u8_t r = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t b = (fg & 0x1F) << 3;
    u32_t cr = *(u32_t *)dst;
    u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; db = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; db = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; dr = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)r * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)g * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)b * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (b12 << 8) | (g12 << 16) | (r12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (b12 << 16) | (g12 << 8) | r12;
#endif
}

static void blend_rgb565_2_rgba8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u8_t r = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t b = (fg & 0x1F) << 3;
    u32_t cr = *(u32_t *)dst;
    u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    dr = cr & 0xff; dg = (cr >> 8) & 0xff; db = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    dr = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)r * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)g * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)b * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = r12 | (g12 << 8) | (b12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (r12 << 24) | (g12 << 16) | (b12 << 8) | a12;
#endif
}

static void blend_rgb565_2_bgra8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u8_t r = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t b = (fg & 0x1F) << 3;
    u32_t cr = *(u32_t *)dst;
    u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    db = cr & 0xff; dg = (cr >> 8) & 0xff; dr = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    db = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)r * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)g * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)b * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = b12 | (g12 << 8) | (r12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (b12 << 24) | (g12 << 16) | (r12 << 8) | a12;
#endif
}

/* ========================================================================== */
/* IMAGE BGR565 BLENDERS                                                     */
/* ========================================================================== */

static void blend_bgr565_2_rgb565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t a6 = (alpha + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src; u32_t bg = *(u16_t *)dst;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1]; u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    /* BGR提取: B在[15:11], G在[10:5], R在[4:0] */
    u8_t b = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t r = (fg & 0x1F) << 3;
    u32_t fg_rgb = ((u32_t)(r >> 3) << 11) | ((u32_t)(g >> 2) << 5) | (u32_t)(b >> 3);
    u32_t out_rb = (((fg_rgb & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg_rgb & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_bgr565_2_bgr565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t a6 = (alpha + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src; u32_t bg = *(u16_t *)dst;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1]; u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t out_rb = (((fg & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

/* 其他 BGR565 2 目标格式转换逻辑与 RGB565 2 目标逻辑一致，仅源像素提取时的 R/B 翻转，此处手动展开 */

static void blend_bgr565_2_rgb888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u32_t ia = 255 - alpha;
    u8_t b = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t r = (fg & 0x1F) << 3;
    dst[0] = (u8_t)(DIV_255(r * alpha + dst[0] * ia));
    dst[1] = (u8_t)(DIV_255(g * alpha + dst[1] * ia));
    dst[2] = (u8_t)(DIV_255(b * alpha + dst[2] * ia));
}

static void blend_bgr565_2_bgr888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u32_t ia = 255 - alpha;
    u8_t b = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t r = (fg & 0x1F) << 3;
    dst[0] = (u8_t)(DIV_255(b * alpha + dst[0] * ia));
    dst[1] = (u8_t)(DIV_255(g * alpha + dst[1] * ia));
    dst[2] = (u8_t)(DIV_255(r * alpha + dst[2] * ia));
}

static void blend_bgr565_2_argb8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u8_t b = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t r = (fg & 0x1F) << 3;
    u32_t cr = *(u32_t *)dst;
    u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; dr = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; dr = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; db = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)r * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)g * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)b * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (r12 << 8) | (g12 << 16) | (b12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (r12 << 16) | (g12 << 8) | b12;
#endif
}

static void blend_bgr565_2_abgr8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u8_t b = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t r = (fg & 0x1F) << 3;
    u32_t cr = *(u32_t *)dst;
    u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; db = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; db = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; dr = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)r * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)g * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)b * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (b12 << 8) | (g12 << 16) | (r12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (b12 << 16) | (g12 << 8) | r12;
#endif
}

static void blend_bgr565_2_rgba8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u8_t b = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t r = (fg & 0x1F) << 3;
    u32_t cr = *(u32_t *)dst;
    u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    dr = cr & 0xff; dg = (cr >> 8) & 0xff; db = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    dr = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)r * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)g * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)b * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = r12 | (g12 << 8) | (b12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (r12 << 24) | (g12 << 16) | (b12 << 8) | a12;
#endif
}

static void blend_bgr565_2_bgra8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t fg = *(u16_t *)src;
#else
    u32_t fg = ((u32_t)src[0] << 8) | src[1];
#endif
    u8_t b = (fg >> 11) << 3; u8_t g = ((fg >> 5) & 0x3F) << 2; u8_t r = (fg & 0x1F) << 3;
    u32_t cr = *(u32_t *)dst;
    u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    db = cr & 0xff; dg = (cr >> 8) & 0xff; dr = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    db = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)r * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)g * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)b * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = b12 | (g12 << 8) | (r12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (b12 << 24) | (g12 << 16) | (r12 << 8) | a12;
#endif
}

/* ========================================================================== */
/* IMAGE RGB888 BLENDERS                                                     */
/* ========================================================================== */

static void blend_rgb888_2_rgb565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t a6 = (alpha + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u8_t sr = src[0], sg = src[1], sb = src[2];
    u32_t fg565 = ((u32_t)(sr >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sb >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_rgb888_2_bgr565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t a6 = (alpha + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u8_t sr = src[0], sg = src[1], sb = src[2];
    u32_t fg565 = ((u32_t)(sb >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sr >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_rgb888_2_rgb888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t ia = 255 - alpha;
    dst[0] = (u8_t)(DIV_255(src[0] * alpha + dst[0] * ia));
    dst[1] = (u8_t)(DIV_255(src[1] * alpha + dst[1] * ia));
    dst[2] = (u8_t)(DIV_255(src[2] * alpha + dst[2] * ia));
}

static void blend_rgb888_2_bgr888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t ia = 255 - alpha;
    dst[0] = (u8_t)(DIV_255(src[2] * alpha + dst[0] * ia));
    dst[1] = (u8_t)(DIV_255(src[1] * alpha + dst[1] * ia));
    dst[2] = (u8_t)(DIV_255(src[0] * alpha + dst[2] * ia));
}

static void blend_rgb888_2_argb8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t cr = *(u32_t *)dst;
    u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; dr = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; dr = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; db = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)src[0] * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)src[1] * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)src[2] * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (r12 << 8) | (g12 << 16) | (b12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (r12 << 16) | (g12 << 8) | b12;
#endif
}

static void blend_rgb888_2_abgr8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t cr = *(u32_t *)dst;
    u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; db = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; db = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; dr = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)src[0] * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)src[1] * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)src[2] * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (b12 << 8) | (g12 << 16) | (r12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (b12 << 16) | (g12 << 8) | r12;
#endif
}

static void blend_rgb888_2_rgba8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t cr = *(u32_t *)dst;
    u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    dr = cr & 0xff; dg = (cr >> 8) & 0xff; db = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    dr = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)src[0] * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)src[1] * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)src[2] * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = r12 | (g12 << 8) | (b12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (r12 << 24) | (g12 << 16) | (b12 << 8) | a12;
#endif
}

static void blend_rgb888_2_bgra8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t cr = *(u32_t *)dst;
    u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    db = cr & 0xff; dg = (cr >> 8) & 0xff; dr = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    db = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)src[0] * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)src[1] * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)src[2] * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = b12 | (g12 << 8) | (r12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (b12 << 24) | (g12 << 16) | (r12 << 8) | a12;
#endif
}

/* ========================================================================== */
/* IMAGE BGR888 BLENDERS                                                     */
/* ========================================================================== */

static void blend_bgr888_2_rgb565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t a6 = (alpha + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u8_t sb = src[0], sg = src[1], sr = src[2];
    u32_t fg565 = ((u32_t)(sr >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sb >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_bgr888_2_bgr565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t a6 = (alpha + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u8_t sb = src[0], sg = src[1], sr = src[2];
    u32_t fg565 = ((u32_t)(sb >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sr >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_bgr888_2_rgb888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t ia = 255 - alpha;
    dst[0] = (u8_t)(DIV_255(src[2] * alpha + dst[0] * ia));
    dst[1] = (u8_t)(DIV_255(src[1] * alpha + dst[1] * ia));
    dst[2] = (u8_t)(DIV_255(src[0] * alpha + dst[2] * ia));
}

static void blend_bgr888_2_bgr888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    (void)blend_mode;
    u32_t ia = 255 - alpha;
    dst[0] = (u8_t)(DIV_255(src[0] * alpha + dst[0] * ia));
    dst[1] = (u8_t)(DIV_255(src[1] * alpha + dst[1] * ia));
    dst[2] = (u8_t)(DIV_255(src[2] * alpha + dst[2] * ia));
}

static void blend_bgr888_2_argb8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    /* 提取源为BGR, 混合入ARGB */
    u8_t sb = src[0], sg = src[1], sr = src[2];
    u32_t cr = *(u32_t *)dst; u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; dr = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; dr = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; db = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (r12 << 8) | (g12 << 16) | (b12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (r12 << 16) | (g12 << 8) | b12;
#endif
}

static void blend_bgr888_2_abgr8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u8_t sb = src[0], sg = src[1], sr = src[2];
    u32_t cr = *(u32_t *)dst; u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; db = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; db = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; dr = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (b12 << 8) | (g12 << 16) | (r12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (b12 << 16) | (g12 << 8) | r12;
#endif
}

static void blend_bgr888_2_rgba8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u8_t sb = src[0], sg = src[1], sr = src[2];
    u32_t cr = *(u32_t *)dst; u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    dr = cr & 0xff; dg = (cr >> 8) & 0xff; db = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    dr = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = r12 | (g12 << 8) | (b12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (r12 << 24) | (g12 << 16) | (b12 << 8) | a12;
#endif
}

static void blend_bgr888_2_bgra8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u8_t sb = src[0], sg = src[1], sr = src[2];
    u32_t cr = *(u32_t *)dst; u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    db = cr & 0xff; dg = (cr >> 8) & 0xff; dr = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    db = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - alpha;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * alpha);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * alpha);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * alpha);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = b12 | (g12 << 8) | (r12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (b12 << 24) | (g12 << 16) | (r12 << 8) | a12;
#endif
}

/* ========================================================================== */
/* IMAGE ARGB8888 BLENDERS                                                   */
/* ========================================================================== */

static void blend_argb8888_2_rgb565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u32_t fv = *(u32_t *)src; u8_t sa, sr, sg, sb;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sr = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sr = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sb = fv & 0xff;
#endif
    u8_t combined_a = (u8_t)(DIV_255((u32_t)sa * alpha));
    u32_t a6 = (combined_a + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t fg565 = ((u32_t)(sr >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sb >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_argb8888_2_bgr565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u32_t fv = *(u32_t *)src; u8_t sa, sr, sg, sb;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sr = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sr = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sb = fv & 0xff;
#endif
    u8_t combined_a = (u8_t)(DIV_255((u32_t)sa * alpha));
    u32_t a6 = (combined_a + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t fg565 = ((u32_t)(sb >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sr >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_argb8888_2_rgb888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u32_t fv = *(u32_t *)src; u8_t sa, sr, sg, sb;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sr = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sr = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sb = fv & 0xff;
#endif
    u32_t combined_a = DIV_255((u32_t)sa * alpha);
    u32_t ia = 255 - combined_a;
    dst[0] = (u8_t)(DIV_255(sr * combined_a + dst[0] * ia));
    dst[1] = (u8_t)(DIV_255(sg * combined_a + dst[1] * ia));
    dst[2] = (u8_t)(DIV_255(sb * combined_a + dst[2] * ia));
}

static void blend_argb8888_2_bgr888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u32_t fv = *(u32_t *)src; u8_t sa, sr, sg, sb;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sr = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sr = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sb = fv & 0xff;
#endif
    u32_t combined_a = DIV_255((u32_t)sa * alpha);
    u32_t ia = 255 - combined_a;
    dst[0] = (u8_t)(DIV_255(sb * combined_a + dst[0] * ia));
    dst[1] = (u8_t)(DIV_255(sg * combined_a + dst[1] * ia));
    dst[2] = (u8_t)(DIV_255(sr * combined_a + dst[2] * ia));
}

static void blend_argb8888_2_argb8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u32_t fv = *(u32_t *)src; u8_t sa, sr, sg, sb;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sr = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sr = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sb = fv & 0xff;
#endif
    u32_t combined_a = DIV_255((u32_t)sa * alpha);
    u32_t cr = *(u32_t *)dst; u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; dr = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; dr = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; db = cr & 0xff;
#endif
    u32_t ia2 = 255 - combined_a;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * combined_a);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * combined_a);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * combined_a);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (r12 << 8) | (g12 << 16) | (b12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (r12 << 16) | (g12 << 8) | b12;
#endif
}

static void blend_argb8888_2_abgr8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u32_t fv = *(u32_t *)src; u8_t sa, sr, sg, sb;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sr = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sr = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sb = fv & 0xff;
#endif
    u32_t combined_a = DIV_255((u32_t)sa * alpha);
    u32_t cr = *(u32_t *)dst; u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; db = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; db = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; dr = cr & 0xff;
#endif
    u32_t ia2 = 255 - combined_a;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * combined_a);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * combined_a);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * combined_a);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (b12 << 8) | (g12 << 16) | (r12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (b12 << 16) | (g12 << 8) | r12;
#endif
}

static void blend_argb8888_2_rgba8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u32_t fv = *(u32_t *)src; u8_t sa, sr, sg, sb;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sr = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sr = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sb = fv & 0xff;
#endif
    u32_t combined_a = DIV_255((u32_t)sa * alpha);
    u32_t cr = *(u32_t *)dst; u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    dr = cr & 0xff; dg = (cr >> 8) & 0xff; db = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    dr = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - combined_a;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * combined_a);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * combined_a);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * combined_a);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = r12 | (g12 << 8) | (b12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (r12 << 24) | (g12 << 16) | (b12 << 8) | a12;
#endif
}

static void blend_argb8888_2_bgra8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u32_t fv = *(u32_t *)src; u8_t sa, sr, sg, sb;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sr = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sr = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sb = fv & 0xff;
#endif
    u32_t combined_a = DIV_255((u32_t)sa * alpha);
    u32_t cr = *(u32_t *)dst; u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    db = cr & 0xff; dg = (cr >> 8) & 0xff; dr = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    db = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - combined_a;
    u32_t bg_w = DIV_255(da * ia2);
    u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * combined_a);
    u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * combined_a);
    u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * combined_a);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = b12 | (g12 << 8) | (r12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (b12 << 24) | (g12 << 16) | (r12 << 8) | a12;
#endif
}

/* ========================================================================== */
/* IMAGE ABGR8888 BLENDERS                                                   */
/* ========================================================================== */

/* 以下 ABGR8888 系列提取逻辑: sa=[0], sb=[1], sg=[2], sr=[3] (Little Endian) */
static void blend_abgr8888_2_rgb565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode)
{
    u32_t fv = *(u32_t *)src; u8_t sa, sb, sg, sr;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sb = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sb = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sr = fv & 0xff;
#endif
    u8_t combined_a = (u8_t)(DIV_255((u32_t)sa * alpha));
    u32_t a6 = (combined_a + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t fg565 = ((u32_t)(sr >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sb >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

/* 后续 ABGR, RGBA, BGRA 源格式以此类推手工补全，逻辑保持寄存器级运算，处理端序 */

static void blend_abgr8888_2_bgr565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sa, sb, sg, sr;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sb = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sb = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sr = fv & 0xff;
#endif
    u8_t ca = (u8_t)(DIV_255((u32_t)sa * alpha)); u32_t a6 = (ca + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t fg565 = ((u32_t)(sb >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sr >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_abgr8888_2_rgb888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sa, sb, sg, sr;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sb = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sb = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sr = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t ia = 255 - ca;
    dst[0] = (u8_t)(DIV_255(sr * ca + dst[0] * ia)); dst[1] = (u8_t)(DIV_255(sg * ca + dst[1] * ia)); dst[2] = (u8_t)(DIV_255(sb * ca + dst[2] * ia));
}

static void blend_abgr8888_2_bgr888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sa, sb, sg, sr;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sb = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sb = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sr = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t ia = 255 - ca;
    dst[0] = (u8_t)(DIV_255(sb * ca + dst[0] * ia)); dst[1] = (u8_t)(DIV_255(sg * ca + dst[1] * ia)); dst[2] = (u8_t)(DIV_255(sr * ca + dst[2] * ia));
}

static void blend_abgr8888_2_argb8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sa, sb, sg, sr;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sb = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sb = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sr = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha);
    u32_t cr = *(u32_t *)dst; u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; dr = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; dr = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; db = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t bg_w = DIV_255(da * ia2); u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (r12 << 8) | (g12 << 16) | (b12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (r12 << 16) | (g12 << 8) | b12;
#endif
}

static void blend_abgr8888_2_abgr8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sa, sb, sg, sr;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sb = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sb = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sr = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t cr = *(u32_t *)dst; u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; db = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; db = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; dr = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t bg_w = DIV_255(da * ia2); u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (b12 << 8) | (g12 << 16) | (r12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (b12 << 16) | (g12 << 8) | r12;
#endif
}

static void blend_abgr8888_2_rgba8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sa, sb, sg, sr;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sb = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sb = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sr = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t cr = *(u32_t *)dst; u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    dr = cr & 0xff; dg = (cr >> 8) & 0xff; db = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    dr = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t bg_w = DIV_255(da * ia2); u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = r12 | (g12 << 8) | (b12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (r12 << 24) | (g12 << 16) | (b12 << 8) | a12;
#endif
}

static void blend_abgr8888_2_bgra8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sa, sb, sg, sr;
#if IPGUI_ENDIAN_LITTLE == 1
    sa = fv & 0xff; sb = (fv >> 8) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 24) & 0xff;
#else
    sa = (fv >> 24) & 0xff; sb = (fv >> 16) & 0xff; sg = (fv >> 8) & 0xff; sr = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t cr = *(u32_t *)dst; u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    db = cr & 0xff; dg = (cr >> 8) & 0xff; dr = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    db = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t bg_w = DIV_255(da * ia2); u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = b12 | (g12 << 8) | (r12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (b12 << 24) | (g12 << 16) | (r12 << 8) | a12;
#endif
}

/* ========================================================================== */
/* IMAGE RGBA8888 BLENDERS                                                   */
/* ========================================================================== */

/* 以下 RGBA8888 系列提取逻辑: sr=[0], sg=[1], sb=[2], sa=[3] (Little Endian) */
static void blend_rgba8888_2_rgb565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sr, sg, sb, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sr = fv & 0xff; sg = (fv >> 8) & 0xff; sb = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sr = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u8_t ca = (u8_t)(DIV_255((u32_t)sa * alpha)); u32_t a6 = (ca + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t fg565 = ((u32_t)(sr >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sb >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

/* 后续函数同理展开 */
static void blend_rgba8888_2_bgr565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sr, sg, sb, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sr = fv & 0xff; sg = (fv >> 8) & 0xff; sb = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sr = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u8_t ca = (u8_t)(DIV_255((u32_t)sa * alpha)); u32_t a6 = (ca + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t fg565 = ((u32_t)(sb >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sr >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_rgba8888_2_rgb888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sr, sg, sb, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sr = fv & 0xff; sg = (fv >> 8) & 0xff; sb = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sr = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t ia = 255 - ca;
    dst[0] = (u8_t)(DIV_255(sr * ca + dst[0] * ia)); dst[1] = (u8_t)(DIV_255(sg * ca + dst[1] * ia)); dst[2] = (u8_t)(DIV_255(sb * ca + dst[2] * ia));
}

static void blend_rgba8888_2_bgr888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sr, sg, sb, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sr = fv & 0xff; sg = (fv >> 8) & 0xff; sb = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sr = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t ia = 255 - ca;
    dst[0] = (u8_t)(DIV_255(sb * ca + dst[0] * ia)); dst[1] = (u8_t)(DIV_255(sg * ca + dst[1] * ia)); dst[2] = (u8_t)(DIV_255(sr * ca + dst[2] * ia));
}

static void blend_rgba8888_2_argb8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sr, sg, sb, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sr = fv & 0xff; sg = (fv >> 8) & 0xff; sb = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sr = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t cr = *(u32_t *)dst; u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; dr = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; dr = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; db = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t bg_w = DIV_255(da * ia2); u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (r12 << 8) | (g12 << 16) | (b12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (r12 << 16) | (g12 << 8) | b12;
#endif
}

static void blend_rgba8888_2_abgr8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sr, sg, sb, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sr = fv & 0xff; sg = (fv >> 8) & 0xff; sb = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sr = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t cr = *(u32_t *)dst; u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; db = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; db = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; dr = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t bg_w = DIV_255(da * ia2); u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (b12 << 8) | (g12 << 16) | (r12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (b12 << 16) | (g12 << 8) | r12;
#endif
}

static void blend_rgba8888_2_rgba8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sr, sg, sb, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sr = fv & 0xff; sg = (fv >> 8) & 0xff; sb = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sr = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t cr = *(u32_t *)dst; u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    dr = cr & 0xff; dg = (cr >> 8) & 0xff; db = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    dr = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t bg_w = DIV_255(da * ia2); u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = r12 | (g12 << 8) | (b12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (r12 << 24) | (g12 << 16) | (b12 << 8) | a12;
#endif
}

static void blend_rgba8888_2_bgra8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sr, sg, sb, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sr = fv & 0xff; sg = (fv >> 8) & 0xff; sb = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sr = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sb = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t cr = *(u32_t *)dst; u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    db = cr & 0xff; dg = (cr >> 8) & 0xff; dr = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    db = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t bg_w = DIV_255(da * ia2); u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = b12 | (g12 << 8) | (r12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (b12 << 24) | (g12 << 16) | (r12 << 8) | a12;
#endif
}

/* ========================================================================== */
/* IMAGE BGRA8888 BLENDERS                                                   */
/* ========================================================================== */

static void blend_bgra8888_2_rgb565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sb, sg, sr, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sb = fv & 0xff; sg = (fv >> 8) & 0xff; sr = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sb = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u8_t ca = (u8_t)(DIV_255((u32_t)sa * alpha)); u32_t a6 = (ca + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t fg565 = ((u32_t)(sr >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sb >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_bgra8888_2_bgr565(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sb, sg, sr, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sb = fv & 0xff; sg = (fv >> 8) & 0xff; sr = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sb = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u8_t ca = (u8_t)(DIV_255((u32_t)sa * alpha)); u32_t a6 = (ca + 2) >> 2; u32_t ia6 = 64 - a6;
#if IPGUI_ENDIAN_LITTLE == 1
    u32_t bg = *(u16_t *)dst;
#else
    u32_t bg = ((u32_t)dst[0] << 8) | dst[1];
#endif
    u32_t fg565 = ((u32_t)(sb >> 3) << 11) | ((u32_t)(sg >> 2) << 5) | (u32_t)(sr >> 3);
    u32_t out_rb = (((fg565 & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
    u32_t out_g  = (((fg565 & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
    u32_t out = (out_rb & MASK_RB) | (out_g & MASK_G);
#if IPGUI_ENDIAN_LITTLE == 1
    *(u16_t *)dst = (u16_t)out;
#else
    dst[0] = (u8_t)(out >> 8); dst[1] = (u8_t)(out & 0xFF);
#endif
}

static void blend_bgra8888_2_rgb888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sb, sg, sr, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sb = fv & 0xff; sg = (fv >> 8) & 0xff; sr = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sb = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t ia = 255 - ca;
    dst[0] = (u8_t)(DIV_255(sr * ca + dst[0] * ia)); dst[1] = (u8_t)(DIV_255(sg * ca + dst[1] * ia)); dst[2] = (u8_t)(DIV_255(sb * ca + dst[2] * ia));
}

static void blend_bgra8888_2_bgr888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sb, sg, sr, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sb = fv & 0xff; sg = (fv >> 8) & 0xff; sr = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sb = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t ia = 255 - ca;
    dst[0] = (u8_t)(DIV_255(sb * ca + dst[0] * ia)); dst[1] = (u8_t)(DIV_255(sg * ca + dst[1] * ia)); dst[2] = (u8_t)(DIV_255(sr * ca + dst[2] * ia));
}

static void blend_bgra8888_2_argb8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sb, sg, sr, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sb = fv & 0xff; sg = (fv >> 8) & 0xff; sr = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sb = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t cr = *(u32_t *)dst; u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; dr = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; dr = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; db = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t bg_w = DIV_255(da * ia2); u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (r12 << 8) | (g12 << 16) | (b12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (r12 << 16) | (g12 << 8) | b12;
#endif
}

static void blend_bgra8888_2_abgr8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sb, sg, sr, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sb = fv & 0xff; sg = (fv >> 8) & 0xff; sr = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sb = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t cr = *(u32_t *)dst; u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    da = cr & 0xff; db = (cr >> 8) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 24) & 0xff;
#else
    da = (cr >> 24) & 0xff; db = (cr >> 16) & 0xff; dg = (cr >> 8) & 0xff; dr = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t bg_w = DIV_255(da * ia2); u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = a12 | (b12 << 8) | (g12 << 16) | (r12 << 24);
#else
    *(u32_t *)dst = (a12 << 24) | (b12 << 16) | (g12 << 8) | r12;
#endif
}

static void blend_bgra8888_2_rgba8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sb, sg, sr, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sb = fv & 0xff; sg = (fv >> 8) & 0xff; sr = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sb = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t cr = *(u32_t *)dst; u32_t da, dr, dg, db;
#if IPGUI_ENDIAN_LITTLE == 1
    dr = cr & 0xff; dg = (cr >> 8) & 0xff; db = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    dr = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; db = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t bg_w = DIV_255(da * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = r12 | (g12 << 8) | (b12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (r12 << 24) | (g12 << 16) | (b12 << 8) | a12;
#endif
}

static void blend_bgra8888_2_bgra8888(const u8_t * src, u8_t * dst, u8_t alpha, ipgui_blend_mode_t blend_mode) {
    u32_t fv = *(u32_t *)src; u8_t sb, sg, sr, sa;
#if IPGUI_ENDIAN_LITTLE == 1
    sb = fv & 0xff; sg = (fv >> 8) & 0xff; sr = (fv >> 16) & 0xff; sa = (fv >> 24) & 0xff;
#else
    sb = (fv >> 24) & 0xff; sg = (fv >> 16) & 0xff; sr = (fv >> 8) & 0xff; sa = fv & 0xff;
#endif
    u32_t ca = DIV_255((u32_t)sa * alpha); u32_t cr = *(u32_t *)dst; u32_t da, db, dg, dr;
#if IPGUI_ENDIAN_LITTLE == 1
    db = cr & 0xff; dg = (cr >> 8) & 0xff; dr = (cr >> 16) & 0xff; da = (cr >> 24) & 0xff;
#else
    db = (cr >> 24) & 0xff; dg = (cr >> 16) & 0xff; dr = (cr >> 8) & 0xff; da = cr & 0xff;
#endif
    u32_t ia2 = 255 - ca; u32_t bg_w = DIV_255(da * ia2); u32_t a12 = 255 - DIV_255((255 - da) * ia2);
    u32_t r12 = DIV_255(dr * bg_w) + DIV_255((u32_t)sr * ca); u32_t g12 = DIV_255(dg * bg_w) + DIV_255((u32_t)sg * ca); u32_t b12 = DIV_255(db * bg_w) + DIV_255((u32_t)sb * ca);
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[a12]) >> 8; g12 = (g12 * g_inv_tbl[a12]) >> 8; b12 = (b12 * g_inv_tbl[a12]) >> 8;
#else
    r12 = a12 ? (r12 << 8) / a12 : 0; g12 = a12 ? (g12 << 8) / a12 : 0; b12 = a12 ? (b12 << 8) / a12 : 0;
#endif
    if (r12 > 255) r12 = 255; if (g12 > 255) g12 = 255; if (b12 > 255) b12 = 255;
#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)dst = b12 | (g12 << 8) | (r12 << 16) | (a12 << 24);
#else
    *(u32_t *)dst = (b12 << 24) | (g12 << 16) | (r12 << 8) | a12;
#endif
}
static const img_px_blend_func_t img_blend_table[IPGUI_IMG_FMT_MAX][PIX_FMT_MAX] = {
    [IPGUI_IMG_FMT_L8] = {
        [PIX_FMT_RGB565]   = blend_l8_2_rgb565,   [PIX_FMT_BGR565]   = blend_l8_2_bgr565,
        [PIX_FMT_RGB888]   = blend_l8_2_rgb888,   [PIX_FMT_BGR888]   = blend_l8_2_bgr888,
        [PIX_FMT_ARGB8888] = blend_l8_2_argb8888, [PIX_FMT_ABGR8888] = blend_l8_2_abgr8888,
        [PIX_FMT_RGBA8888] = blend_l8_2_rgba8888, [PIX_FMT_BGRA8888] = blend_l8_2_bgra8888,
    },
    [IPGUI_IMG_FMT_LA88] = {
        [PIX_FMT_RGB565]   = blend_la88_2_rgb565,   [PIX_FMT_BGR565]   = blend_la88_2_bgr565,
        [PIX_FMT_RGB888]   = blend_la88_2_rgb888,   [PIX_FMT_BGR888]   = blend_la88_2_bgr888,
        [PIX_FMT_ARGB8888] = blend_la88_2_argb8888, [PIX_FMT_ABGR8888] = blend_la88_2_abgr8888,
        [PIX_FMT_RGBA8888] = blend_la88_2_rgba8888, [PIX_FMT_BGRA8888] = blend_la88_2_bgra8888,
    },
    [IPGUI_IMG_FMT_RGB565] = {
        [PIX_FMT_RGB565]   = blend_rgb565_2_rgb565,   [PIX_FMT_BGR565]   = blend_rgb565_2_bgr565,
        [PIX_FMT_RGB888]   = blend_rgb565_2_rgb888,   [PIX_FMT_BGR888]   = blend_rgb565_2_bgr888,
        [PIX_FMT_ARGB8888] = blend_rgb565_2_argb8888, [PIX_FMT_ABGR8888] = blend_rgb565_2_abgr8888,
        [PIX_FMT_RGBA8888] = blend_rgb565_2_rgba8888, [PIX_FMT_BGRA8888] = blend_rgb565_2_bgra8888,
    },
    [IPGUI_IMG_FMT_BGR565] = {
        [PIX_FMT_RGB565]   = blend_bgr565_2_rgb565,   [PIX_FMT_BGR565]   = blend_bgr565_2_bgr565,
        [PIX_FMT_RGB888]   = blend_bgr565_2_rgb888,   [PIX_FMT_BGR888]   = blend_bgr565_2_bgr888,
        [PIX_FMT_ARGB8888] = blend_bgr565_2_argb8888, [PIX_FMT_ABGR8888] = blend_bgr565_2_abgr8888,
        [PIX_FMT_RGBA8888] = blend_bgr565_2_rgba8888, [PIX_FMT_BGRA8888] = blend_bgr565_2_bgra8888,
    },
    [IPGUI_IMG_FMT_RGB888] = {
        [PIX_FMT_RGB565]   = blend_rgb888_2_rgb565,   [PIX_FMT_BGR565]   = blend_rgb888_2_bgr565,
        [PIX_FMT_RGB888]   = blend_rgb888_2_rgb888,   [PIX_FMT_BGR888]   = blend_rgb888_2_bgr888,
        [PIX_FMT_ARGB8888] = blend_rgb888_2_argb8888, [PIX_FMT_ABGR8888] = blend_rgb888_2_abgr8888,
        [PIX_FMT_RGBA8888] = blend_rgb888_2_rgba8888, [PIX_FMT_BGRA8888] = blend_rgb888_2_bgra8888,
    },
    [IPGUI_IMG_FMT_BGR888] = {
        [PIX_FMT_RGB565]   = blend_bgr888_2_rgb565,   [PIX_FMT_BGR565]   = blend_bgr888_2_bgr565,
        [PIX_FMT_RGB888]   = blend_bgr888_2_rgb888,   [PIX_FMT_BGR888]   = blend_bgr888_2_bgr888,
        [PIX_FMT_ARGB8888] = blend_bgr888_2_argb8888, [PIX_FMT_ABGR8888] = blend_bgr888_2_abgr8888,
        [PIX_FMT_RGBA8888] = blend_bgr888_2_rgba8888, [PIX_FMT_BGRA8888] = blend_bgr888_2_bgra8888,
    },
    [IPGUI_IMG_FMT_ARGB8888] = {
        [PIX_FMT_RGB565]   = blend_argb8888_2_rgb565,   [PIX_FMT_BGR565]   = blend_argb8888_2_bgr565,
        [PIX_FMT_RGB888]   = blend_argb8888_2_rgb888,   [PIX_FMT_BGR888]   = blend_argb8888_2_bgr888,
        [PIX_FMT_ARGB8888] = blend_argb8888_2_argb8888, [PIX_FMT_ABGR8888] = blend_argb8888_2_abgr8888,
        [PIX_FMT_RGBA8888] = blend_argb8888_2_rgba8888, [PIX_FMT_BGRA8888] = blend_argb8888_2_bgra8888,
    },
    [IPGUI_IMG_FMT_ABGR8888] = {
        [PIX_FMT_RGB565]   = blend_abgr8888_2_rgb565,   [PIX_FMT_BGR565]   = blend_abgr8888_2_bgr565,
        [PIX_FMT_RGB888]   = blend_abgr8888_2_rgb888,   [PIX_FMT_BGR888]   = blend_abgr8888_2_bgr888,
        [PIX_FMT_ARGB8888] = blend_abgr8888_2_argb8888, [PIX_FMT_ABGR8888] = blend_abgr8888_2_abgr8888,
        [PIX_FMT_RGBA8888] = blend_abgr8888_2_rgba8888, [PIX_FMT_BGRA8888] = blend_abgr8888_2_bgra8888,
    },
    [IPGUI_IMG_FMT_RGBA8888] = {
        [PIX_FMT_RGB565]   = blend_rgba8888_2_rgb565,   [PIX_FMT_BGR565]   = blend_rgba8888_2_bgr565,
        [PIX_FMT_RGB888]   = blend_rgba8888_2_rgb888,   [PIX_FMT_BGR888]   = blend_rgba8888_2_bgr888,
        [PIX_FMT_ARGB8888] = blend_rgba8888_2_argb8888, [PIX_FMT_ABGR8888] = blend_rgba8888_2_abgr8888,
        [PIX_FMT_RGBA8888] = blend_rgba8888_2_rgba8888, [PIX_FMT_BGRA8888] = blend_rgba8888_2_bgra8888,
    },
    [IPGUI_IMG_FMT_BGRA8888] = {
        [PIX_FMT_RGB565]   = blend_bgra8888_2_rgb565,   [PIX_FMT_BGR565]   = blend_bgra8888_2_bgr565,
        [PIX_FMT_RGB888]   = blend_bgra8888_2_rgb888,   [PIX_FMT_BGR888]   = blend_bgra8888_2_bgr888,
        [PIX_FMT_ARGB8888] = blend_bgra8888_2_argb8888, [PIX_FMT_ABGR8888] = blend_bgra8888_2_abgr8888,
        [PIX_FMT_RGBA8888] = blend_bgra8888_2_rgba8888, [PIX_FMT_BGRA8888] = blend_bgra8888_2_bgra8888,
    },
};

/* 这个接口已经接近完美了，但是像素格式为A8或者I8时怎么处理？ 
 * A8：不会作为图片格式，而是形状（掩码）格式，所以这个接口根本不需要支持A8
 *     用ipgui_blend_color可以胜任，但是不灵活只能是连续的A8数据
 * I8：？？？？？
 */

/* v1: mask aabb belong to image aaabb */
__IPGUI_API__ void ipgui_blend_image_v1(
    ipgui_surf_t      * surf,
    ipgui_aabb_t      * clip,

    ipgui_image_src_t * img_src,

    u8_t              * mask,       /* 蒙版，作用于图像 */
    ipgui_aabb_t      * mask_aabb,  /* 蒙版区域，必须大于等于图像包围盒*/

    u8_t                opacity,
    ipgui_blend_mode_t  blend_mode)
{
    if ((!surf) ||\
        (!img_src) ||\
        (!img_src->buf) || \
        (!img_src->img_aabb) ||\
        (opacity < 3))
        return;

    ipgui_aabb_t blend_aabb;
    if (0 != ipgui_aabb_overlap(&blend_aabb, &surf->surf, img_src->img_aabb))
        return;
    if (clip) {
        if (0 != ipgui_aabb_overlap(&blend_aabb, &blend_aabb, clip))
            return; 
    }

    /* get dest buf, pix size and stride of surf */
    ipgui_coord_t x, y;
    s8_t dest_pix_size;
    s32_t dest_stride;
    u8_t * dest_cr_buf;
    x = blend_aabb.start.x - surf->surf.start.x;
    y = blend_aabb.start.y - surf->surf.start.y;
    dest_cr_buf   = ipgui_surf_color_get(surf, x, y);
    dest_pix_size = surf->pix_size;
    dest_stride   = surf->stride;

    /* get src buf of image */
    u8_t * img_buf;
    x = blend_aabb.start.x - img_src->img_aabb->start.x;
    y = blend_aabb.start.y - img_src->img_aabb->start.y;
    img_buf = img_src->buf + y * img_src->stride + x * img_src->px_size;

    ipgui_coord_t x_span, y_span;
    x_span = ipgui_aabb_width(&blend_aabb);
    y_span = ipgui_aabb_height(&blend_aabb);

    img_px_blend_func_t blend_px = img_blend_table[img_src->img_pxfmt][surf->pix_fmt];

    s32_t dest_row_pix_off;
    u32_t img_row_pix_off;
    if (mask && mask_aabb) {
        u8_t mask_val, mask_opacity_combined;
        ipgui_coord_t mask_stride = ipgui_aabb_width(mask_aabb);
        ipgui_coord_t mask_x0     = blend_aabb.start.x - mask_aabb->start.x;
        ipgui_coord_t mask_y0     = blend_aabb.start.y - mask_aabb->start.y;
        u8_t        * mask_row    = mask + mask_y0 * mask_stride + mask_x0;

        for (y = 0; y < y_span; y ++) {
            dest_row_pix_off = 0;
            img_row_pix_off  = 0;
            for (x = 0; x < x_span; x ++) {
                /* blend pixel by pixel */
                mask_val = mask_row[x];
                if (mask_val > 2) {
                    /* mix mask and opacity */
                    mask_opacity_combined =
                        (u8_t)(((u32_t)opacity * mask_val + 127) >> 8);
                    
                    blend_px(&img_buf[img_row_pix_off], &dest_cr_buf[dest_row_pix_off], mask_opacity_combined, blend_mode);
                }
                dest_row_pix_off += dest_pix_size;
                img_row_pix_off  += img_src->px_size;
            }
            dest_cr_buf += dest_stride;
            img_buf     += img_src->stride;
            mask_row    += mask_stride;
        }
    } else {
        for (y = 0; y < y_span; y ++) {
            dest_row_pix_off = 0;
            img_row_pix_off  = 0;
            for (x = 0; x < x_span; x ++) {
                /* blend pixel by pixel */
                blend_px(&img_buf[img_row_pix_off], &dest_cr_buf[dest_row_pix_off], opacity, blend_mode);
                dest_row_pix_off += dest_pix_size;
                img_row_pix_off  += img_src->px_size;
            }
            dest_cr_buf += dest_stride;
            img_buf     += img_src->stride;
        }
    }
}

/* v2: mask aabb belong to dest aaabb */
__IPGUI_API__ void ipgui_blend_image_v2(
    ipgui_surf_t      * surf,
    ipgui_aabb_t      * clip,
    ipgui_aabb_t      * dest,
    ipgui_image_src_t * img_src,
    u8_t                opacity,
    u8_t              * mask,       /* 蒙版，作用于dest */
    ipgui_aabb_t      * mask_aabb,  /* 蒙版区域，必须大于等于dest围盒*/
    ipgui_blend_mode_t  blend_mode)
{
    if ((!surf) ||\
    (!dest)||\
    (!img_src) ||\
    (!img_src->buf) || \
    (!img_src->img_aabb) ||\
    (opacity < 3))
    return;

    ipgui_aabb_t blend_aabb;
    if (0 != ipgui_aabb_overlap(&blend_aabb, dest, &(surf->surf)))
        return;

    if (clip) {
        if (0 != ipgui_aabb_overlap(&blend_aabb, &blend_aabb, clip))
            return;
    }

    /* 再和图片的aabb求交 */
    if (0 != ipgui_aabb_overlap(&blend_aabb, &blend_aabb, img_src->img_aabb))
        return;

    /* get dest buf, pix size and stride of surf */
    ipgui_coord_t x, y;
    s8_t dest_pix_size;
    s32_t dest_stride;
    u8_t * dest_cr_buf;
    x = blend_aabb.start.x - surf->surf.start.x;
    y = blend_aabb.start.y - surf->surf.start.y;
    dest_cr_buf   = ipgui_surf_color_get(surf, x, y);
    dest_pix_size = surf->pix_size;
    dest_stride   = surf->stride;

    /* get src buf of image */
    u8_t * img_buf;
    x = blend_aabb.start.x - img_src->img_aabb->start.x;
    y = blend_aabb.start.y - img_src->img_aabb->start.y;
    img_buf = img_src->buf + y * img_src->stride + x * img_src->px_size;
    ipgui_coord_t x_span, y_span;
    x_span = ipgui_aabb_width(&blend_aabb);
    y_span = ipgui_aabb_height(&blend_aabb);

    img_px_blend_func_t blend_px = img_blend_table[img_src->img_pxfmt][surf->pix_fmt];

    s32_t dest_row_pix_off;
    u32_t img_row_pix_off;
    if (mask && mask_aabb) {
        u8_t mask_val, mask_opacity_combined;
        ipgui_coord_t mask_stride = ipgui_aabb_width(mask_aabb);
        ipgui_coord_t mask_x0     = blend_aabb.start.x - mask_aabb->start.x;
        ipgui_coord_t mask_y0     = blend_aabb.start.y - mask_aabb->start.y;
        u8_t        * mask_row    = mask + mask_y0 * mask_stride + mask_x0;

        for (y = 0; y < y_span; y ++) {
            dest_row_pix_off = 0;
            img_row_pix_off  = 0;
            for (x = 0; x < x_span; x ++) {
                /* blend pixel by pixel */
                mask_val = mask_row[x];
                if (mask_val > 2) {
                    /* mix mask and opacity */
                    mask_opacity_combined =
                        (u8_t)(((u32_t)opacity * mask_val + 127) >> 8);
                    
                    blend_px(&img_buf[img_row_pix_off], &dest_cr_buf[dest_row_pix_off], mask_opacity_combined, blend_mode);
                }
                dest_row_pix_off += dest_pix_size;
                img_row_pix_off  += img_src->px_size;
            }
            dest_cr_buf += dest_stride;
            img_buf     += img_src->stride;
            mask_row    += mask_stride;
        }
    } else {
        for (y = 0; y < y_span; y ++) {
            dest_row_pix_off = 0;
            img_row_pix_off  = 0;
            for (x = 0; x < x_span; x ++) {
                /* blend pixel by pixel */
                blend_px(&img_buf[img_row_pix_off], &dest_cr_buf[dest_row_pix_off], opacity, blend_mode);
                dest_row_pix_off += dest_pix_size;
                img_row_pix_off  += img_src->px_size;
            }
            dest_cr_buf += dest_stride;
            img_buf     += img_src->stride;
        }
    }
}