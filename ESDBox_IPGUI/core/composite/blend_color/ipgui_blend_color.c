#include "ipgui_blend_color.h"
#include "ipgui_conf.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"
#include "ipgui_mask_gradient.h"

/* RGB565/BGR565 packed blend 公共掩码 */
#define MASK_RB     0xf81fU   /* 位15:11 + 位4:0，即高5位和低5位 */
#define MASK_G      0x07e0U   /* 位10:5，中间6位 */
#define MASK_MUL_RB 0x3e07c0U /* MASK_RB << 6，乘以最大alpha(64)后的范围 */
#define MASK_MUL_G  0x1f800U  /* MASK_G  << 6 */

#if USE_INV_TABLE == 1
/* the table's result is 255 * 255 / (0-255) */ 
const u16_t g_inv_tbl[256] = {
    0,65025,32512,21675,16256,13005,10837, 9289, 8128, 7225, 6502, 5911, 5418, 5001, 4644, 4335,
 4064, 3825, 3612, 3422, 3251, 3096, 2955, 2827, 2709, 2601, 2500, 2408, 2322, 2242, 2167, 2097,
 2032, 1970, 1912, 1857, 1806, 1757, 1711, 1667, 1625, 1585, 1548, 1512, 1477, 1445, 1413, 1383,
 1354, 1327, 1300, 1275, 1250, 1226, 1204, 1182, 1161, 1140, 1121, 1102, 1083, 1065, 1048, 1032,
 1016, 1000,  985,  970,  956,  942,  928,  915,  903,  890,  878,  867,  855,  844,  833,  823,
  812,  802,  792,  783,  774,  765,  756,  747,  738,  730,  722,  714,  706,  699,  691,  684,
  677,  670,  663,  656,  650,  643,  637,  631,  625,  619,  613,  607,  602,  596,  591,  585,
  580,  575,  570,  565,  560,  555,  551,  546,  541,  537,  532,  528,  524,  520,  516,  512,
  508,  504,  500,  496,  492,  488,  485,  481,  478,  474,  471,  467,  464,  461,  457,  454,
  451,  448,  445,  442,  439,  436,  433,  430,  427,  425,  422,  419,  416,  414,  411,  408,
  406,  403,  401,  398,  396,  394,  391,  389,  387,  384,  382,  380,  378,  375,  373,  371,
  369,  367,  365,  363,  361,  359,  357,  355,  353,  351,  349,  347,  345,  344,  342,  340,
  338,  336,  335,  333,  331,  330,  328,  326,  325,  323,  321,  320,  318,  317,  315,  314,
  312,  311,  309,  308,  306,  305,  303,  302,  301,  299,  298,  296,  295,  294,  292,  291,
  290,  289,  287,  286,  285,  283,  282,  281,  280,  279,  277,  276,  275,  274,  273,  272,
  270,  269,  268,  267,  266,  265,  264,  263,  262,  261,  260,  259,  258,  257,  256,  255,
};
#endif

/*
格式     端序  [0]低字节 [1]高字节
RGB565  小端  GGGBBBBB  RRRRRGGG 
RGB565  大端  RRRRRGGG  GGGBBBBB 
BGR565  小端  GGGRRRRR  BBBBBGGG
BGR565  大端  BBBBBGGG  GGGRRRRR
*/

/* builtin color(premultiplied) blend with rgb565/bgr565 \
 * rgb565/bgr565 is background color
 * blend formula: result = fg color(premultiplied) + (1-alpha) * background
 */
#if IPGUI_ENDIAN_LITTLE == 1
void ipgui_builtin_premultiplied_color_blend_to_rgb565(
    ipgui_color_t color, u8_t * rgb565, ipgui_blend_mode_t blend_mode)
{
    /* unused parameter */
    (void) blend_mode;

    /* ialpha: 8bit → 6bit+1，范围[0..64]，+2是为了四舍五入 */
    u32_t ialpha6 = (255 - IPGUI_COLOR_A(color) + 2) >> 2;
    u32_t bg = *(u16_t *)rgb565;

    /* 直接在packed空间对bg整体做缩放，掩码防止通道间溢出串扰 */
    u32_t bg_rb = ((ialpha6 * (bg & MASK_RB)) & MASK_MUL_RB) >> 6;
    u32_t bg_g  = ((ialpha6 * (bg & MASK_G )) & MASK_MUL_G ) >> 6;

    /* fg premultiplied 8bit → rgb565 packed格式 */
    u32_t fg565 = ((u32_t)(IPGUI_COLOR_R(color) >> 3) << 11)
                       | ((u32_t)(IPGUI_COLOR_G(color) >> 2) <<  5)
                       | ((u32_t)(IPGUI_COLOR_B(color) >> 3)      );

    /* packed直接相加：premultiplied保证各通道结果不会溢出本字段 */
    *(u16_t *)rgb565 = (u16_t)(
        ((fg565 & MASK_RB) + bg_rb) | ((fg565 & MASK_G) + bg_g)
    );
}

void ipgui_builtin_premultiplied_color_blend_to_bgr565(
    ipgui_color_t color, u8_t * rgb565, ipgui_blend_mode_t blend_mode)
{
    /* unused parameter */
    (void) blend_mode;

    u32_t ialpha6 = (255 - IPGUI_COLOR_A(color) + 2) >> 2;
    u32_t bg = *(u16_t *)rgb565;

    u32_t bg_rb = ((ialpha6 * (bg & MASK_RB)) & MASK_MUL_RB) >> 6;
    u32_t bg_g  = ((ialpha6 * (bg & MASK_G )) & MASK_MUL_G ) >> 6;

    /* BGR565：B在高位(<<11)，R在低位 */
    u32_t fg565 = ((u32_t)(IPGUI_COLOR_B(color) >> 3) << 11)
                       | ((u32_t)(IPGUI_COLOR_G(color) >> 2) <<  5)
                       | ((u32_t)(IPGUI_COLOR_R(color) >> 3)      );

    *(u16_t *)rgb565 = (u16_t)(
        ((fg565 & MASK_RB) + bg_rb) | ((fg565 & MASK_G) + bg_g)
    );
}
#else
void ipgui_builtin_premultiplied_color_blend_to_rgb565(
    ipgui_color_t color, u8_t * rgb565, ipgui_blend_mode_t blend_mode)
{
    /* unused parameter */
    (void) blend_mode;

    u32_t ialpha6 = (255 - IPGUI_COLOR_A(color) + 2) >> 2;
    /* 大端：[0]是高字节(RRRRRGGG)，[1]是低字节(GGGBBBBB) */
    u32_t bg = ((u32_t)rgb565[0] << 8) | rgb565[1];

    u32_t bg_rb = ((ialpha6 * (bg & MASK_RB)) & MASK_MUL_RB) >> 6;
    u32_t bg_g  = ((ialpha6 * (bg & MASK_G )) & MASK_MUL_G ) >> 6;

    u32_t fg565 = ((u32_t)(IPGUI_COLOR_R(color) >> 3) << 11)
                       | ((u32_t)(IPGUI_COLOR_G(color) >> 2) <<  5)
                       | ((u32_t)(IPGUI_COLOR_B(color) >> 3)      );

    u32_t out = ((fg565 & MASK_RB) + bg_rb) | ((fg565 & MASK_G) + bg_g);
    rgb565[0] = (u8_t)(out >> 8);
    rgb565[1] = (u8_t)(out & 0xFF);
}

void ipgui_builtin_premultiplied_color_blend_to_bgr565(
    ipgui_color_t color, u8_t * rgb565, ipgui_blend_mode_t blend_mode)
{
    /* unused parameter */
    (void) blend_mode;
    
    u32_t ialpha6 = (255 - IPGUI_COLOR_A(color) + 2) >> 2;
    u32_t bg = ((u32_t)rgb565[0] << 8) | rgb565[1];

    u32_t bg_rb = ((ialpha6 * (bg & MASK_RB)) & MASK_MUL_RB) >> 6;
    u32_t bg_g  = ((ialpha6 * (bg & MASK_G )) & MASK_MUL_G ) >> 6;

    u32_t fg565 = ((u32_t)(IPGUI_COLOR_B(color) >> 3) << 11)
                       | ((u32_t)(IPGUI_COLOR_G(color) >> 2) <<  5)
                       | ((u32_t)(IPGUI_COLOR_R(color) >> 3)      );

    u32_t out = ((fg565 & MASK_RB) + bg_rb) | ((fg565 & MASK_G) + bg_g);
    rgb565[0] = (u8_t)(out >> 8);
    rgb565[1] = (u8_t)(out & 0xFF);
}
#endif

/* builtin color(premultiplied) blend with rgb888 \
 * rgb888 is background color
 * blend formula: result = fg color(premultiplied) + (1-alpha) * background
 */
void ipgui_builtin_premultiplied_color_blend_to_rgb888(
    ipgui_color_t color, u8_t * rgb888, ipgui_blend_mode_t blend_mode)
{
    /* unused parameter */
    (void) blend_mode;
    
    u8_t ialpha = 255 - IPGUI_COLOR_A(color);

    /* r */
    rgb888[0] = IPGUI_COLOR_R(color) + (u8_t)((ialpha * rgb888[0]) >> 8);
    /* g */
    rgb888[1] = IPGUI_COLOR_G(color) + (u8_t)((ialpha * rgb888[1]) >> 8);
    /* b */
    rgb888[2] = IPGUI_COLOR_B(color) + (u8_t)((ialpha * rgb888[2]) >> 8);
}

/* builtin color(premultiplied) blend with bgr888 \
 * bgr888 is background color
 * blend formula: result = fg color(premultiplied) + (1-alpha) * background
 */
void ipgui_builtin_premultiplied_color_blend_to_bgr888(
    ipgui_color_t color, u8_t * bgr888, ipgui_blend_mode_t blend_mode)
{
    /* unused parameter */
    (void) blend_mode;
    
    u8_t ialpha = 255 - IPGUI_COLOR_A(color);

    /* b */
    bgr888[0] = IPGUI_COLOR_B(color) + (u8_t)((ialpha * bgr888[0]) >> 8);
    /* g */
    bgr888[1] = IPGUI_COLOR_G(color) + (u8_t)((ialpha * bgr888[1]) >> 8);
    /* r */
    bgr888[2] = IPGUI_COLOR_R(color) + (u8_t)((ialpha * bgr888[2]) >> 8);
}

/* 两个像素格式为RGBA8888（及其他类似格式）中间层（假设C1为底层，C2为C1的上层）混合后
 * 那么C12 = (C1 * alpha1 * (1 - alpha2) + C2 * alpha2) / (alpha1 + alpha2 - alpha1 * alpha2)
 * alpha12 = alpha1 + alpha2 - alpha1 * alpha2
 */

/* builtin color(premultiplied) blend with argb8888 \
 * argb8888 is background color
 */
void ipgui_builtin_premultiplied_color_blend_to_argb8888(
    ipgui_color_t color, u8_t * argb8888, ipgui_blend_mode_t blend_mode)
{
    /* unused parameter */
    (void) blend_mode;
    
    u32_t cr = *(u32_t *)argb8888;
    u32_t alpha, r, g, b;

#if IPGUI_ENDIAN_LITTLE == 1
    /* little endian memory: [A][R][G][B] → cr = 0xBBGGRRAA */
    /*                       [0][1][2][3]                   */
    alpha =  cr        & 0xff;
    r     = (cr >>  8) & 0xff;
    g     = (cr >> 16) & 0xff;
    b     = (cr >> 24) & 0xff;
#else
    /* big endian memory: [A][R][G][B] → cr = 0xAARRGGBB */
    /*                    [0][1][2][3]                   */
    alpha = (cr >> 24) & 0xff;
    r     = (cr >> 16) & 0xff;
    g     = (cr >>  8) & 0xff;
    b     =  cr        & 0xff;
#endif
    u32_t ialpha2 = 255 - IPGUI_COLOR_A(color);

    /* alpha12 = 1 - (1 - alpha1) * (1 - alpha2)，等价于 alpha1 + alpha2 - alpha1 * alpha2，且天然不超过255 */
    u32_t alpha12 = 255 - (((255 - alpha) * ialpha2) >> 8);

    u32_t r12 = ((r * alpha * ialpha2) >> 16) + IPGUI_COLOR_R(color);
    u32_t g12 = ((g * alpha * ialpha2) >> 16) + IPGUI_COLOR_G(color);
    u32_t b12 = ((b * alpha * ialpha2) >> 16) + IPGUI_COLOR_B(color);

#if USE_INV_TABLE == 0
    r12 = alpha12 ? (r12 << 8) / alpha12 : 0;
    g12 = alpha12 ? (g12 << 8) / alpha12 : 0;
    b12 = alpha12 ? (b12 << 8) / alpha12 : 0;
#else
    r12 = (r12 * g_inv_tbl[alpha12]) >> 8;
    g12 = (g12 * g_inv_tbl[alpha12]) >> 8;
    b12 = (b12 * g_inv_tbl[alpha12]) >> 8;
#endif
    /* clamp */
    if (r12 > 255) r12 = 255;
    if (g12 > 255) g12 = 255;
    if (b12 > 255) b12 = 255;

#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)argb8888 = alpha12 | (r12 << 8) | (g12 << 16) | (b12 << 24);
#else
    *(u32_t *)argb8888 = (alpha12 << 24) | (r12 << 16) | (g12 << 8) | b12;
#endif
}

/* builtin color(premultiplied) blend with abgr8888 \
 * abgr8888 is background color
 */
void ipgui_builtin_premultiplied_color_blend_to_abgr8888(
    ipgui_color_t color, u8_t * abgr8888, ipgui_blend_mode_t blend_mode)
{
    /* unused parameter */
    (void) blend_mode;
    
    u32_t cr = *(u32_t *)abgr8888;
    u32_t alpha, r, g, b;

#if IPGUI_ENDIAN_LITTLE == 1
    /* little endian memory: [A][B][G][R] → cr = 0xRRGGBBAA */
    /*                       [0][1][2][3]                   */
    alpha =  cr        & 0xff;
    b     = (cr >>  8) & 0xff;
    g     = (cr >> 16) & 0xff;
    r     = (cr >> 24) & 0xff;
#else
    /* big endian memory: [A][B][G][R] → cr = 0xAABBGGRR */
    /*                    [0][1][2][3]                   */
    alpha = (cr >> 24) & 0xff;
    b     = (cr >> 16) & 0xff;
    g     = (cr >>  8) & 0xff;
    r     =  cr        & 0xff;
#endif

    u32_t ialpha2 = 255 - IPGUI_COLOR_A(color);
    u32_t alpha12 = 255 - (((255 - alpha) * ialpha2) >> 8);

    u32_t r12 = ((r * alpha * ialpha2) >> 16) + IPGUI_COLOR_R(color);
    u32_t g12 = ((g * alpha * ialpha2) >> 16) + IPGUI_COLOR_G(color);
    u32_t b12 = ((b * alpha * ialpha2) >> 16) + IPGUI_COLOR_B(color);

#if USE_INV_TABLE == 0
    r12 = alpha12 ? (r12 << 8) / alpha12 : 0;
    g12 = alpha12 ? (g12 << 8) / alpha12 : 0;
    b12 = alpha12 ? (b12 << 8) / alpha12 : 0;
#else
    r12 = (r12 * g_inv_tbl[alpha12]) >> 8;
    g12 = (g12 * g_inv_tbl[alpha12]) >> 8;
    b12 = (b12 * g_inv_tbl[alpha12]) >> 8;
#endif

    if (r12 > 255) r12 = 255;
    if (g12 > 255) g12 = 255;
    if (b12 > 255) b12 = 255;

#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)abgr8888 = alpha12 | (b12 << 8) | (g12 << 16) | (r12 << 24);
#else
    *(u32_t *)abgr8888 = (alpha12 << 24) | (b12 << 16) | (g12 << 8) | r12;
#endif
}

/* builtin color(premultiplied) blend with rgba8888 \
 * rgba8888 is background color
 */
void ipgui_builtin_premultiplied_color_blend_to_rgba8888(
    ipgui_color_t color, u8_t * rgba8888, ipgui_blend_mode_t blend_mode)
{
    /* unused parameter */
    (void) blend_mode;
    
    u32_t cr = *(u32_t *)rgba8888;
    u32_t alpha, r, g, b;

#if IPGUI_ENDIAN_LITTLE == 1
    /* little endian memory: [R][G][B][A] → cr = 0xAABBGGRR */
    /*                       [0][1][2][3]                   */
    r     =  cr        & 0xff;
    g     = (cr >>  8) & 0xff;
    b     = (cr >> 16) & 0xff;
    alpha = (cr >> 24) & 0xff;
#else
    /* big endian memory: [R][G][B][A] → cr = 0xRRGGBBAA */
    /*                    [0][1][2][3]                   */
    r     = (cr >> 24) & 0xff;
    g     = (cr >> 16) & 0xff;
    b     = (cr >>  8) & 0xff;
    alpha =  cr        & 0xff;
#endif

    u32_t ialpha2 = 255 - IPGUI_COLOR_A(color);

    u32_t alpha12 = 255 - (((255 - alpha) * ialpha2) >> 8);
    u32_t r12 = ((r * alpha * ialpha2) >> 16) + IPGUI_COLOR_R(color);
    u32_t g12 = ((g * alpha * ialpha2) >> 16) + IPGUI_COLOR_G(color);
    u32_t b12 = ((b * alpha * ialpha2) >> 16) + IPGUI_COLOR_B(color);

#if USE_INV_TABLE == 0
    r12 = alpha12 ? (r12 << 8) / alpha12 : 0;
    g12 = alpha12 ? (g12 << 8) / alpha12 : 0;
    b12 = alpha12 ? (b12 << 8) / alpha12 : 0;
#else
    r12 = (r12 * g_inv_tbl[alpha12]) >> 8;
    g12 = (g12 * g_inv_tbl[alpha12]) >> 8;
    b12 = (b12 * g_inv_tbl[alpha12]) >> 8;
#endif

    if (r12 > 255) r12 = 255;
    if (g12 > 255) g12 = 255;
    if (b12 > 255) b12 = 255;

#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)rgba8888 = r12 | (g12 << 8) | (b12 << 16) | (alpha12 << 24);
#else
    *(u32_t *)rgba8888 = (r12 << 24) | (g12 << 16) | (b12 << 8) | alpha12;
#endif
}

/* builtin color(premultiplied) blend with bgra8888 \
 * bgra8888 is background color
 */
void ipgui_builtin_premultiplied_color_blend_to_bgra8888(
    ipgui_color_t color, u8_t * bgra8888, ipgui_blend_mode_t blend_mode)
{
    /* unused parameter */
    (void) blend_mode;
    
    u32_t cr = *(u32_t *)bgra8888;
    u32_t alpha, r, g, b;

#if IPGUI_ENDIAN_LITTLE == 1
    /* little endian memory: [B][G][R][A] → cr = 0xAARRGGBB */
    /*                       [0][1][2][3]                   */
    b     =  cr        & 0xff;
    g     = (cr >>  8) & 0xff;
    r     = (cr >> 16) & 0xff;
    alpha = (cr >> 24) & 0xff;
#else
    /* big endian memory: [B][G][R][A] → cr = 0xBBGGRRAA */
    /*                    [0][1][2][3]                   */
    b     = (cr >> 24) & 0xff;
    g     = (cr >> 16) & 0xff;
    r     = (cr >>  8) & 0xff;
    alpha =  cr        & 0xff;
#endif

    u32_t ialpha2 = 255 - IPGUI_COLOR_A(color);
    u32_t alpha12 = 255 - (((255 - alpha) * ialpha2) >> 8);

    u32_t r12 = ((r * alpha * ialpha2) >> 16) + IPGUI_COLOR_R(color);
    u32_t g12 = ((g * alpha * ialpha2) >> 16) + IPGUI_COLOR_G(color);
    u32_t b12 = ((b * alpha * ialpha2) >> 16) + IPGUI_COLOR_B(color);

#if USE_INV_TABLE == 0
    r12 = alpha12 ? (r12 << 8) / alpha12 : 0;
    g12 = alpha12 ? (g12 << 8) / alpha12 : 0;
    b12 = alpha12 ? (b12 << 8) / alpha12 : 0;
#else
    r12 = (r12 * g_inv_tbl[alpha12]) >> 8;
    g12 = (g12 * g_inv_tbl[alpha12]) >> 8;
    b12 = (b12 * g_inv_tbl[alpha12]) >> 8;
#endif

    if (r12 > 255) r12 = 255;
    if (g12 > 255) g12 = 255;
    if (b12 > 255) b12 = 255;

#if IPGUI_ENDIAN_LITTLE == 1
    *(u32_t *)bgra8888 = b12 | (g12 << 8) | (r12 << 16) | (alpha12 << 24);
#else
    *(u32_t *)bgra8888 = (b12 << 24) | (g12 << 16) | (r12 << 8) | alpha12;
#endif
}

premult_blend_func_t premult_blend_table[PIX_FMT_MAX] = {
    [PIX_FMT_RGB565] = ipgui_builtin_premultiplied_color_blend_to_rgb565,
    [PIX_FMT_BGR565] = ipgui_builtin_premultiplied_color_blend_to_bgr565,

    [PIX_FMT_RGB888] = ipgui_builtin_premultiplied_color_blend_to_rgb888,
    [PIX_FMT_BGR888] = ipgui_builtin_premultiplied_color_blend_to_bgr888,

    [PIX_FMT_ARGB8888] = ipgui_builtin_premultiplied_color_blend_to_argb8888,
    [PIX_FMT_ABGR8888] = ipgui_builtin_premultiplied_color_blend_to_abgr8888,
    [PIX_FMT_RGBA8888] = ipgui_builtin_premultiplied_color_blend_to_rgba8888,
    [PIX_FMT_BGRA8888] = ipgui_builtin_premultiplied_color_blend_to_bgra8888,
};

#if IPGUI_ENDIAN_LITTLE == 1
/* solid color convert to rgb565, must ignore color's alpha */
u32_t ipgui_solid_color_2_rgb565(ipgui_color_t color, u8_t * rgb565)
{
    *(u16_t *)rgb565 = ((u16_t)(IPGUI_COLOR_R(color) >> 3) << 11)
                              | ((u16_t)(IPGUI_COLOR_G(color) >> 2) <<  5)
                              | ((u16_t)(IPGUI_COLOR_B(color) >> 3)      );
    return 2;
}

/* solid color convert to bgr565, must ignore color's alpha */
u32_t ipgui_solid_color_2_bgr565(ipgui_color_t color, u8_t * bgr565)
{
    *(u16_t *)bgr565 = ((u16_t)(IPGUI_COLOR_B(color) >> 3) << 11)
                              | ((u16_t)(IPGUI_COLOR_G(color) >> 2) <<  5)
                              | ((u16_t)(IPGUI_COLOR_R(color) >> 3)      );
    return 2;
}
#else
/* solid color convert to rgb565, must ignore color's alpha */
u32_t ipgui_solid_color_2_rgb565(ipgui_color_t color, u8_t * rgb565)
{
    u16_t v = ((u16_t)(IPGUI_COLOR_R(color) >> 3) << 11)
                     | ((u16_t)(IPGUI_COLOR_G(color) >> 2) <<  5)
                     | ((u16_t)(IPGUI_COLOR_B(color) >> 3)      );
    rgb565[0] = (u8_t)(v >> 8);
    rgb565[1] = (u8_t)(v & 0xff);
    return 2;
}

/* solid color convert to bgr565, must ignore color's alpha */
u32_t ipgui_solid_color_2_bgr565(ipgui_color_t color, u8_t * bgr565)
{
    u16_t v = ((u16_t)(IPGUI_COLOR_B(color) >> 3) << 11)
                     | ((u16_t)(IPGUI_COLOR_G(color) >> 2) <<  5)
                     | ((u16_t)(IPGUI_COLOR_R(color) >> 3)      );
    bgr565[0] = (u8_t)(v >> 8);
    bgr565[1] = (u8_t)(v & 0xff);
    return 2;
}
#endif

/* solid color convert to rgb888, must ignore color's alpha */
u32_t ipgui_solid_color_2_rgb888(ipgui_color_t color, u8_t * rgb888)
{
    rgb888[0] = IPGUI_COLOR_R(color);
    rgb888[1] = IPGUI_COLOR_G(color);
    rgb888[2] = IPGUI_COLOR_B(color);
    return 3;
}

/* solid color convert to bgr888, must ignore color's alpha */
u32_t ipgui_solid_color_2_bgr888(ipgui_color_t color, u8_t * bgr888)
{
    bgr888[0] = IPGUI_COLOR_B(color);
    bgr888[1] = IPGUI_COLOR_G(color);
    bgr888[2] = IPGUI_COLOR_R(color);
    return 3;
}

/* solid color convert to argb8888, must ignore color's alpha */
u32_t ipgui_solid_color_2_argb8888(ipgui_color_t color, u8_t * argb8888)
{
    *(u32_t *)argb8888 = 
#if IPGUI_ENDIAN_LITTLE == 1
    0x000000ffU \
    | ((u32_t)IPGUI_COLOR_R(color) << 8) \
    | ((u32_t)IPGUI_COLOR_G(color) << 16) \
    | ((u32_t)IPGUI_COLOR_B(color) << 24);
#else
    0xff000000U \
    | ((u32_t)IPGUI_COLOR_R(color) << 16) \
    | ((u32_t)IPGUI_COLOR_G(color) << 8) \
    | IPGUI_COLOR_B(color);
#endif
    return 4;
}

/* solid color convert to abgr8888, must ignore color's alpha */
u32_t ipgui_solid_color_2_abgr8888(ipgui_color_t color, u8_t * abgr8888)
{
    /* memory: [A][B][G][R] */
    *(u32_t *)abgr8888 =
#if IPGUI_ENDIAN_LITTLE == 1
    0x000000ffU \
    | ((u32_t)IPGUI_COLOR_B(color) << 8) \
    | ((u32_t)IPGUI_COLOR_G(color) << 16) \
    | ((u32_t)IPGUI_COLOR_R(color) << 24);
#else
    0xff000000U \
    | ((u32_t)IPGUI_COLOR_B(color) << 16) \
    | ((u32_t)IPGUI_COLOR_G(color) << 8) \
    | IPGUI_COLOR_R(color);
#endif
    return 4;
}

/* solid color convert to rgba8888, must ignore color's alpha */
u32_t ipgui_solid_color_2_rgba8888(ipgui_color_t color, u8_t * rgba8888)
{
    /* memory: [R][G][B][A] */
    *(u32_t *)rgba8888 =
#if IPGUI_ENDIAN_LITTLE == 1
    ((u32_t)IPGUI_COLOR_R(color)) \
    | ((u32_t)IPGUI_COLOR_G(color) << 8) \
    | ((u32_t)IPGUI_COLOR_B(color) << 16) \
    | 0xff000000U;
#else
    ((u32_t)IPGUI_COLOR_R(color) << 24) \
    | ((u32_t)IPGUI_COLOR_G(color) << 16) \
    | ((u32_t)IPGUI_COLOR_B(color) << 8) \
    | 0x000000ffU;
#endif
    return 4;
}

/* solid color convert to bgra8888, must ignore color's alpha */
u32_t ipgui_solid_color_2_bgra8888(ipgui_color_t color, u8_t * bgra8888)
{
    /* memory: [B][G][R][A] */
    *(u32_t *)bgra8888 =
#if IPGUI_ENDIAN_LITTLE == 1
    ((u32_t)IPGUI_COLOR_B(color)) \
    | ((u32_t)IPGUI_COLOR_G(color) << 8) \
    | ((u32_t)IPGUI_COLOR_R(color) << 16) \
    | 0xff000000U;
#else
    ((u32_t)IPGUI_COLOR_B(color) << 24) \
    | ((u32_t)IPGUI_COLOR_G(color) << 16) \
    | ((u32_t)IPGUI_COLOR_R(color) << 8) \
    | 0x000000ffU;
#endif
    return 4;
}

solid_convert_func_t solid_conv_table[PIX_FMT_MAX] = {
    [PIX_FMT_RGB565] = ipgui_solid_color_2_rgb565,
    [PIX_FMT_BGR565] = ipgui_solid_color_2_bgr565,

    [PIX_FMT_RGB888] = ipgui_solid_color_2_rgb888,
    [PIX_FMT_BGR888] = ipgui_solid_color_2_bgr888,

    [PIX_FMT_ARGB8888] = ipgui_solid_color_2_argb8888,
    [PIX_FMT_ABGR8888] = ipgui_solid_color_2_abgr8888,
    [PIX_FMT_RGBA8888] = ipgui_solid_color_2_rgba8888,
    [PIX_FMT_BGRA8888] = ipgui_solid_color_2_bgra8888,
};

/* return premultiplied color */
ipgui_color_t ipgui_color_premultiply(ipgui_color_t * color)
{
    ipgui_color_t res;

	u8_t ca = color->a;
    u8_t cr = (u8_t)(((u32_t)color->r * ca + 128) >> 8);
    u8_t cg = (u8_t)(((u32_t)color->g * ca + 128) >> 8);
    u8_t cb = (u8_t)(((u32_t)color->b * ca + 128) >> 8);
    
	IPGUI_COLOR_SET_R(res, cr);
	IPGUI_COLOR_SET_G(res, cg);
	IPGUI_COLOR_SET_B(res, cb);
	IPGUI_COLOR_SET_A(res, ca);

    return res;
}

ipgui_color_t ipgui_color_combine_opacity(ipgui_color_t * color, u8_t opacity)
{
    ipgui_color_t res;

    IPGUI_COLOR_SET_R(res, color->r);
	IPGUI_COLOR_SET_G(res, color->g);
	IPGUI_COLOR_SET_B(res, color->b);
	IPGUI_COLOR_SET_A(res, (u8_t)(((u32_t)color->a * opacity + 128) >> 8));

	return res;
}

ipgui_color_t ipgui_color_combine_opacity_and_premultiply(
    ipgui_color_t * color, u8_t opacity)
{
    ipgui_color_t res;

	u8_t ca = (u8_t)(((u32_t)color->a * opacity + 128) >> 8);
    u8_t cr = (u8_t)(((u32_t)color->r * ca + 128) >> 8);
    u8_t cg = (u8_t)(((u32_t)color->g * ca + 128) >> 8);
    u8_t cb = (u8_t)(((u32_t)color->b * ca + 128) >> 8);

	IPGUI_COLOR_SET_R(res, cr);
	IPGUI_COLOR_SET_G(res, cg);
	IPGUI_COLOR_SET_B(res, cb);
	IPGUI_COLOR_SET_A(res, ca);

    return res;
}

__IPGUI_API__ void ipgui_fill_color(
    ipgui_surf_t * surf, 
    ipgui_aabb_t * clip,
    ipgui_aabb_t * dest, 
    ipgui_color_t color, 
    u8_t opacity,
    ipgui_blend_mode_t blend_mode)
{
    if ((!dest) || (!surf) || (opacity < 3))
        return;
    
    ipgui_aabb_t fill_aabb;
    if(0 != ipgui_aabb_overlap(&fill_aabb, dest, &(surf->surf)))
        return;
    
    if (clip) {
        if(0 != ipgui_aabb_overlap(&fill_aabb, &fill_aabb, clip))
            return;
    }

    ipgui_coord_t x, y, x_span, y_span;
    s8_t pix_size;
    s32_t stride;
    u8_t * dest_cr_buf;

    x = fill_aabb.start.x - surf->surf.start.x;
    y = fill_aabb.start.y - surf->surf.start.y;
    dest_cr_buf = ipgui_surf_color_get(surf, x, y);
    x_span = ipgui_aabb_width(&fill_aabb);
    y_span = ipgui_aabb_height(&fill_aabb);
    pix_size = surf->pix_size;
    stride = surf->stride;

    /* combine color and opacity */
    ipgui_color_t combined = ipgui_color_combine_opacity(&color, opacity);

    if (IPGUI_COLOR_A(combined) > 252) {
        s32_t row_pix_off;
        u8_t solid_pix[8];

        solid_convert_func_t solid_conv_fn = solid_conv_table[surf->pix_fmt];
        if (!solid_conv_fn) return;

        u32_t fmt_size = solid_conv_fn(combined, solid_pix);
        if ((fmt_size > sizeof(solid_pix)) || (fmt_size > pix_size)) {
            ipgui_dbg_error("solid_pix buffer is not enough or pix_size is too small");
            return;  /* 缓冲区不足或者格式太大，大于支持的像素大小 */
        }

        if (fmt_size == pix_size) {
            /* fill first row first */
            u8_t * first_row = dest_cr_buf;
            row_pix_off = 0;
            for (x = 0; x < x_span; x ++) {
                ipgui_memcpy(&dest_cr_buf[row_pix_off], solid_pix, pix_size);
                row_pix_off += pix_size;
            }

            /* copy first row to left rows */
            for (y = 1; y < y_span; y ++) {
                dest_cr_buf += stride;
                ipgui_memcpy(dest_cr_buf, first_row, x_span * pix_size);
            }
        } else {
            for (y = 0; y < y_span; y ++) {
                /* solid color fill the current row */
                row_pix_off = 0;
                for (x = 0; x < x_span; x ++) {
                    ipgui_memcpy(&dest_cr_buf[row_pix_off], solid_pix, fmt_size);
                    row_pix_off += pix_size;
                }

                /* go to the next row */
                dest_cr_buf += stride;
            }
        }
    } else {
        s32_t row_pix_off;

        premult_blend_func_t blend_fn = premult_blend_table[surf->pix_fmt];
        if (!blend_fn) return;

        /* premultiply color */
        combined = ipgui_color_premultiply(&combined);

        for (y = 0; y < y_span; y ++) {
            /* blend the current row */
            row_pix_off = 0;
            for (x = 0; x < x_span; x ++) {
                blend_fn(combined, &dest_cr_buf[row_pix_off], blend_mode);
                row_pix_off += pix_size;
            }

            /* go to the next row */
            dest_cr_buf += stride;
        }
    }
}

__IPGUI_API__ void ipgui_blend_color(
    ipgui_surf_t * surf,
    ipgui_aabb_t * clip,
    ipgui_aabb_t * dest,
    ipgui_color_t color,
    u8_t opacity,
    u8_t * mask,      /* mask覆盖mask_aabb区域 */
    ipgui_aabb_t * mask_aabb,  /* mask对应的坐标区域，必须大于或等于dest区域 */
    ipgui_blend_mode_t blend_mode)
{
    if ((!dest) || (!surf) || (opacity < 3))
        return;

    /* mask为空时退化为纯色填充 */
    if (!mask) {
        ipgui_fill_color(surf, clip, dest, color, opacity, blend_mode);
        return;
    }

    if (!mask_aabb)
        return;

    ipgui_aabb_t blend_aabb;
    if (0 != ipgui_aabb_overlap(&blend_aabb, dest, &(surf->surf)))
        return;

    if (clip) {
        if (0 != ipgui_aabb_overlap(&blend_aabb, &blend_aabb, clip))
            return;
    }

    ipgui_coord_t x, y, x_span, y_span;
    s8_t pix_size;
    s32_t stride;
    u8_t * dest_cr_buf;

    x = blend_aabb.start.x - surf->surf.start.x;
    y = blend_aabb.start.y - surf->surf.start.y;
    dest_cr_buf = ipgui_surf_color_get(surf, x, y);
    x_span = ipgui_aabb_width(&blend_aabb);
    y_span = ipgui_aabb_height(&blend_aabb);
    pix_size = surf->pix_size;
    stride = surf->stride;

    ipgui_coord_t mask_stride = ipgui_aabb_width(mask_aabb);
    ipgui_coord_t mask_x0 = blend_aabb.start.x - mask_aabb->start.x;
    ipgui_coord_t mask_y0 = blend_aabb.start.y - mask_aabb->start.y;
    u8_t * mask_row = mask + mask_y0 * mask_stride + mask_x0;

    premult_blend_func_t blend_fn = premult_blend_table[surf->pix_fmt];
    if (!blend_fn) return;

    s32_t row_pix_off;
    u8_t mask_val, mask_opacity_combined;
    ipgui_color_t premult;
    for (y = 0; y < y_span; y ++) {
        row_pix_off = 0;
        for (x = 0; x < x_span; x ++) {
            mask_val = mask_row[x];

            if (mask_val > 2) {
                /* mix mask and opacity */
                mask_opacity_combined =
                    (u8_t)(((u32_t)opacity * mask_val + 128) >> 8);
                
                premult = ipgui_color_combine_opacity_and_premultiply(
                    &color, mask_opacity_combined);

                if (IPGUI_COLOR_A(premult) > 2) {
                    blend_fn(premult, &dest_cr_buf[row_pix_off], blend_mode);
                }
            }

            row_pix_off += pix_size;
        }

        dest_cr_buf += stride;
        mask_row += mask_stride;
    }
}
