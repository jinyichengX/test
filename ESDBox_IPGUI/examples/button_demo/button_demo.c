/*
 * 按钮控件渲染演示
 * 采用黄金比例设计：w = h * 2.618
 * 统一圆角/阴影规范，对齐现代 UI 设计
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static FILE *fopen_utf8(const char *path, const char *mode)
{
    wchar_t wpath[MAX_PATH], wmode[16];
    if (MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH) <= 0) return NULL;
    MultiByteToWideChar(CP_ACP, 0, mode, -1, wmode, 16);
    return _wfopen(wpath, wmode);
}
#else
#define fopen_utf8(path, mode) fopen(path, mode)
#endif

#include "ipgui_utils.h"
#include "ipgui_core.h"
#include "ipgui_color.h"
#include "ipgui_prim.h"
#include "ipgui_blend.h"
#include "ipgui_blend_color.h"
#include "ipgui_blend_gradient_color.h"
#include "ipgui_gradient_color.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_draw_box_shadow.h"
#include "ipgui_draw_pixel.h"

/* ---- 框架桩 ---- */
void ipgui_printk(char *fmt, ...) { (void)fmt; }
void ipgui_memset(void *s, int c, unsigned int n) { memset(s, c, n); }
void *ipgui_memcpy(void *dst, const void *src, unsigned long long n)
{ return memcpy(dst, src, (size_t)n); }
void *ipgui_mem_alloc_def(unsigned long long size) { return calloc(1, (size_t)size); }
void *ipgui_mem_alloc(unsigned long long size, void *pool) { (void)pool; return calloc(1, (size_t)size); }
void ipgui_mem_free_def(void *ptr) { free(ptr); }
void ipgui_mem_free_pool(void *ptr, void *pool) { (void)pool; free(ptr); }
void *ipgui_mem_realoc(void *ptr, unsigned long long size, void *pool)
{ (void)pool; return realloc(ptr, (size_t)size); }

/* ---- 黄金比例常量 ---- */
#define GOLDEN_RATIO  2.618
#define BTN_H_SMALL   36
#define BTN_W_SMALL   ((int)(BTN_H_SMALL * GOLDEN_RATIO))
#define BTN_H_NORMAL  40
#define BTN_W_NORMAL  ((int)(BTN_H_NORMAL * GOLDEN_RATIO))
#define BTN_H_LARGE   48
#define BTN_W_LARGE   ((int)(BTN_H_LARGE * GOLDEN_RATIO))
#define BTN_H_XLARGE  56
#define BTN_W_XLARGE  ((int)(BTN_H_XLARGE * GOLDEN_RATIO))

#define CANVAS_W      800
#define CANVAS_H      600
#define OUT_DIR       "ESDBox_IPGUI/examples/button_demo"
#define PADDING       40  /* 按钮间距 */

/* ---- PPM ---- */
static int surf_to_ppm(const ipgui_surf_t *surf, const char *path)
{
    FILE *fp;
    int x, y, w = surf->surf.end.x - surf->surf.start.x + 1,
        h = surf->surf.end.y - surf->surf.start.y + 1;
    if (!surf || !surf->color || !path) return -1;
    fp = fopen_utf8(path, "wb");
    if (!fp) return -1;
    fprintf(fp, "P6\n%d %d\n255\n", w, h);
    for (y = 0; y < h; y++) {
        unsigned char *row = surf->color + (size_t)y * surf->stride;
        for (x = 0; x < w; x++) {
            unsigned char rgb[3];
            unsigned char *px = row + (size_t)x * surf->pix_size;
            rgb[0] = px[1]; rgb[1] = px[2]; rgb[2] = px[3];
            fwrite(rgb, 1, 3, fp);
        }
    }
    fclose(fp);
    return 0;
}

/* ---- 辅助 ---- */
static void fill_solid_bg(ipgui_surf_t *surf, ipgui_color_t c)
{
    ipgui_blend_color(surf, (ipgui_aabb_t *)0, &surf->surf,
                      c, 255, (u8_t *)0, (ipgui_aabb_t *)0, IPGUI_BLEND_NORMAL);
}

/* 创建线性渐变：上→下 */
static void make_grad_ver(ipgui_grad_src_t *grad,
                          ipgui_color_t top, ipgui_color_t bot,
                          ipgui_coord_t x1, ipgui_coord_t y1,
                          ipgui_coord_t x2, ipgui_coord_t y2)
{
    ipgui_aabb_t a = {{x1, y1}, {x2, y2}};
    grad->grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
    ipgui_liner_gradient_init(&grad->grad.liner_grad, 0.0f, 0.0f, 0.0f, 1.0f);
    ipgui_gradient_color_stop_t s0 = {.color = top, .pos = 0};
    ipgui_gradient_color_stop_t s1 = {.color = bot, .pos = 255};
    ipgui_liner_gradient_add_stop(&grad->grad.liner_grad, &s0);
    ipgui_liner_gradient_add_stop(&grad->grad.liner_grad, &s1);
    ipgui_liner_gradient_apply_to_aabb(&grad->grad.liner_grad, &a);
}

/* 创建线性渐变：左→右 */
static void make_grad_hor(ipgui_grad_src_t *grad,
                          ipgui_color_t left, ipgui_color_t right,
                          ipgui_coord_t x1, ipgui_coord_t y1,
                          ipgui_coord_t x2, ipgui_coord_t y2)
{
    ipgui_aabb_t a = {{x1, y1}, {x2, y2}};
    grad->grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
    ipgui_liner_gradient_init(&grad->grad.liner_grad, 0.0f, 0.0f, 1.0f, 0.0f);
    ipgui_gradient_color_stop_t s0 = {.color = left,  .pos = 0};
    ipgui_gradient_color_stop_t s1 = {.color = right, .pos = 255};
    ipgui_liner_gradient_add_stop(&grad->grad.liner_grad, &s0);
    ipgui_liner_gradient_add_stop(&grad->grad.liner_grad, &s1);
    ipgui_liner_gradient_apply_to_aabb(&grad->grad.liner_grad, &a);
}

/* ---- 单按钮绘制 ---- */
static void draw_one_button(
    ipgui_surf_t *surf,
    ipgui_coord_t x, ipgui_coord_t y,
    ipgui_coord_t w, ipgui_coord_t h,
    ipgui_box_style_t *shape,
    ipgui_box_bg_style_t *bg,
    ipgui_box_border_style_t *border,
    ipgui_box_shadow_style_t *shadow)
{
    ipgui_aabb_t box = {
        .start = {.x = x, .y = y},
        .end   = {.x = x + w - 1, .y = y + h - 1}
    };

    if (shadow)
        ipgui_draw_box_shadow(surf, (ipgui_aabb_t *)0, &box, shape, shadow);

    ipgui_draw_box_background(surf, (ipgui_aabb_t *)0, &box, shape, bg);

    ipgui_draw_box_border(surf, (ipgui_aabb_t *)0, &box, shape, border);
}

/* ---- 便捷宏 ---- */
static void bg_solid(ipgui_box_bg_style_t *bg, u32_t rgb, u8_t opacity)
{
    bg->paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(bg->paint.src.color, opacity, rgb);
    bg->opacity = 255;
    bg->blend_mode = IPGUI_BLEND_NORMAL;
}

static void bg_grad(ipgui_box_bg_style_t *bg, ipgui_grad_src_t *grad)
{
    bg->paint.type = IPGUI_PAINT_GRADIENT;
    bg->paint.src.grad_src = *grad;
    bg->opacity = 255;
    bg->blend_mode = IPGUI_BLEND_NORMAL;
}

static void border_set(ipgui_box_border_style_t *b,
                       ipgui_coord_t width, u32_t rgb, u8_t opacity)
{
    b->paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(b->paint.src.color, opacity, rgb);
    b->opacity = opacity;
    b->width = width;
    b->blend_mode = IPGUI_BLEND_NORMAL;
}

static void shadow_set(ipgui_box_shadow_style_t *s,
                       u32_t rgb, u8_t opacity,
                       ipgui_coord_t blur, ipgui_coord_t spread,
                       ipgui_coord_t ox, ipgui_coord_t oy)
{
    IPGUI_COLOR_SET(s->color, 255, rgb);
    s->opacity  = opacity;
    s->blur     = blur;
    s->spread   = spread;
    s->offset_x = ox;
    s->offset_y = oy;
}

static void radius_set(ipgui_box_style_t *s, ipgui_coord_t r)
{
    s->left_padding = s->right_padding = 0;
    s->top_padding  = s->bottom_padding  = 0;
    s->left_top_radius = s->right_top_radius =
    s->left_bottom_radius = s->right_bottom_radius = r;
}

int main(void)
{
    int ret = 1;
    const u32_t pix_size = 4;
    const u32_t stride   = CANVAS_W * pix_size;
    u8_t *buf = (u8_t *)calloc(1, (size_t)CANVAS_H * stride);
    if (!buf) { printf("ERROR: alloc\n"); return 1; }

    ipgui_surf_t surf = {
        .surf     = {{0, 0}, {CANVAS_W - 1, CANVAS_H - 1}},
        .color    = buf,
        .stride   = stride,
        .pix_fmt  = PIX_FMT_ARGB8888,
        .pix_size = pix_size
    };

    ipgui_color_t white;
    IPGUI_COLOR_SET(white, 255, 0xFFFFFF);
    fill_solid_bg(&surf, white);

    int x = PADDING, y = PADDING;
    ipgui_box_style_t sh = {0};
    ipgui_box_bg_style_t bg;
    ipgui_box_border_style_t bo;
    ipgui_box_shadow_style_t sd;
    ipgui_color_t c1, c2;
    ipgui_grad_src_t grad;

    /* ================================================================
     * Row 1: 基础纯色按钮（黄金比例 BTN_W_NORMAL x BTN_H_NORMAL）
     * ================================================================ */

    /* 1. 浅灰基础 */
    radius_set(&sh, 6);
    bg_solid(&bg, 0xE2E6EA, 255);
    border_set(&bo, 1, 0xCCD2D9, 255);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, NULL);
    x += BTN_W_NORMAL + PADDING;

    /* 2. 蓝色主按钮 + 阴影 */
    bg_solid(&bg, 0x4080FF, 255);
    border_set(&bo, 1, 0x3070EF, 255);
    shadow_set(&sd, 0x4080FF, 60, 6, 0, 0, 4);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);
    x += BTN_W_NORMAL + PADDING;

    /* 3. 绿色成功按钮 */
    bg_solid(&bg, 0x00B42A, 255);
    border_set(&bo, 1, 0x009A2A, 255);
    shadow_set(&sd, 0x00B42A, 60, 6, 0, 0, 4);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);
    x += BTN_W_NORMAL + PADDING;

    /* 4. 红色危险按钮 */
    bg_solid(&bg, 0xF53F3F, 255);
    border_set(&bo, 1, 0xD93636, 255);
    shadow_set(&sd, 0xF53F3F, 60, 6, 0, 0, 4);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);
    x += BTN_W_NORMAL + PADDING;

    /* 5. 橙色警告按钮 */
    bg_solid(&bg, 0xFF7D00, 255);
    border_set(&bo, 1, 0xE66A00, 255);
    shadow_set(&sd, 0xFF7D00, 60, 6, 0, 0, 4);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);

    /* 第二行 */
    x = PADDING;
    y += BTN_H_NORMAL + PADDING;

    /* ================================================================
     * Row 2: 垂直渐变按钮
     * ================================================================ */

    /* 6. 蓝→深紫渐变 */
    IPGUI_COLOR_SET(c1, 255, 0x667EEA);
    IPGUI_COLOR_SET(c2, 255, 0x764BA2);
    make_grad_ver(&grad, c1, c2, x, y, x+BTN_W_NORMAL-1, y+BTN_H_NORMAL-1);
    bg_grad(&bg, &grad);
    border_set(&bo, 1, 0x5A6ED6, 255);
    shadow_set(&sd, 0x667EEA, 70, 8, 0, 0, 5);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);
    x += BTN_W_NORMAL + PADDING;

    /* 7. 绿→青渐变 */
    IPGUI_COLOR_SET(c1, 255, 0x13C2C2);
    IPGUI_COLOR_SET(c2, 255, 0x0FC6C2);
    make_grad_ver(&grad, c1, c2, x, y, x+BTN_W_NORMAL-1, y+BTN_H_NORMAL-1);
    bg_grad(&bg, &grad);
    border_set(&bo, 1, 0x0EAAAA, 255);
    shadow_set(&sd, 0x13C2C2, 70, 8, 0, 0, 5);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);
    x += BTN_W_NORMAL + PADDING;

    /* 8. 红→橙渐变 */
    IPGUI_COLOR_SET(c1, 255, 0xF66A6A);
    IPGUI_COLOR_SET(c2, 255, 0xFF7D00);
    make_grad_ver(&grad, c1, c2, x, y, x+BTN_W_NORMAL-1, y+BTN_H_NORMAL-1);
    bg_grad(&bg, &grad);
    border_set(&bo, 1, 0xE65C5C, 255);
    shadow_set(&sd, 0xF66A6A, 70, 8, 0, 0, 5);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);
    x += BTN_W_NORMAL + PADDING;

    /* 9. 紫→粉渐变 */
    IPGUI_COLOR_SET(c1, 255, 0x9254FF);
    IPGUI_COLOR_SET(c2, 255, 0xF759AB);
    make_grad_ver(&grad, c1, c2, x, y, x+BTN_W_NORMAL-1, y+BTN_H_NORMAL-1);
    bg_grad(&bg, &grad);
    border_set(&bo, 1, 0x8345EE, 255);
    shadow_set(&sd, 0x9254FF, 70, 8, 0, 0, 5);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);
    x += BTN_W_NORMAL + PADDING;

    /* 10. 深灰→黑渐变 */
    IPGUI_COLOR_SET(c1, 255, 0x4E5969);
    IPGUI_COLOR_SET(c2, 255, 0x1D2129);
    make_grad_ver(&grad, c1, c2, x, y, x+BTN_W_NORMAL-1, y+BTN_H_NORMAL-1);
    bg_grad(&bg, &grad);
    border_set(&bo, 1, 0x364052, 255);
    shadow_set(&sd, 0x4E5969, 70, 8, 0, 0, 5);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);

    /* 第三行 */
    x = PADDING;
    y += BTN_H_NORMAL + PADDING;

    /* ================================================================
     * Row 3: 水平渐变 + 大按钮
     * ================================================================ */
    radius_set(&sh, 8); /* 大圆角 */

    /* 11. 蓝水平渐变 大按钮 */
    IPGUI_COLOR_SET(c1, 255, 0x165DFF);
    IPGUI_COLOR_SET(c2, 255, 0x0FC6C2);
    make_grad_hor(&grad, c1, c2, x, y, x+BTN_W_LARGE-1, y+BTN_H_LARGE-1);
    bg_grad(&bg, &grad);
    border_set(&bo, 0, 0, 0);
    shadow_set(&sd, 0x165DFF, 80, 10, 0, 0, 6);
    draw_one_button(&surf, x, y, BTN_W_LARGE, BTN_H_LARGE, &sh, &bg, &bo, &sd);
    x += BTN_W_LARGE + PADDING;

    /* 12. 橙水平渐变 大按钮 */
    IPGUI_COLOR_SET(c1, 255, 0xFF7D00);
    IPGUI_COLOR_SET(c2, 255, 0xF53F3F);
    make_grad_hor(&grad, c1, c2, x, y, x+BTN_W_LARGE-1, y+BTN_H_LARGE-1);
    bg_grad(&bg, &grad);
    border_set(&bo, 0, 0, 0);
    shadow_set(&sd, 0xFF7D00, 80, 10, 0, 0, 6);
    draw_one_button(&surf, x, y, BTN_W_LARGE, BTN_H_LARGE, &sh, &bg, &bo, &sd);
    x += BTN_W_LARGE + PADDING;

    /* 13. 绿水平渐变 大按钮 */
    IPGUI_COLOR_SET(c1, 255, 0x00B42A);
    IPGUI_COLOR_SET(c2, 255, 0x7BC616);
    make_grad_hor(&grad, c1, c2, x, y, x+BTN_W_LARGE-1, y+BTN_H_LARGE-1);
    bg_grad(&bg, &grad);
    border_set(&bo, 0, 0, 0);
    shadow_set(&sd, 0x00B42A, 80, 10, 0, 0, 6);
    draw_one_button(&surf, x, y, BTN_W_LARGE, BTN_H_LARGE, &sh, &bg, &bo, &sd);

    /* 第四行 */
    x = PADDING;
    y += BTN_H_LARGE + PADDING;

    /* ================================================================
     * Row 4: 特殊样式
     * ================================================================ */
    radius_set(&sh, 6);

    /* 14. 白色细边框阴影按钮 */
    bg_solid(&bg, 0xFFFFFF, 255);
    border_set(&bo, 1, 0xE5E6EB, 255);
    shadow_set(&sd, 0x000000, 40, 4, 0, 0, 2);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);
    x += BTN_W_NORMAL + PADDING;

    /* 15. 半透明按钮 */
    bg_solid(&bg, 0x9254FF, 180);
    border_set(&bo, 0, 0, 0);
    shadow_set(&sd, 0x9254FF, 50, 5, 0, 0, 3);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);
    x += BTN_W_NORMAL + PADDING;

    /* 16. 三色渐变 */
    {
        ipgui_aabb_t a = {{x, y}, {x+BTN_W_NORMAL-1, y+BTN_H_NORMAL-1}};
        grad.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
        ipgui_liner_gradient_init(&grad.grad.liner_grad, 0.0f, 0.0f, 1.0f, 0.0f);
        ipgui_gradient_color_stop_t s0, s1, s2;
        IPGUI_COLOR_SET(s0.color, 255, 0xF53F3F); s0.pos = 0;
        IPGUI_COLOR_SET(s1.color, 255, 0xFF7D00); s1.pos = 128;
        IPGUI_COLOR_SET(s2.color, 255, 0x00B42A); s2.pos = 255;
        ipgui_liner_gradient_add_stop(&grad.grad.liner_grad, &s0);
        ipgui_liner_gradient_add_stop(&grad.grad.liner_grad, &s1);
        ipgui_liner_gradient_add_stop(&grad.grad.liner_grad, &s2);
        ipgui_liner_gradient_apply_to_aabb(&grad.grad.liner_grad, &a);
        bg_grad(&bg, &grad);
        border_set(&bo, 1, 0xE65C5C, 255);
        shadow_set(&sd, 0xF53F3F, 60, 6, 0, 0, 4);
        draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);
    }
    x += BTN_W_NORMAL + PADDING;

    /* 17. 径向渐变 */
    {
        ipgui_aabb_t a = {{x, y}, {x+BTN_W_NORMAL-1, y+BTN_H_NORMAL-1}};
        grad.grad_type = IPGUI_GRADIENT_TYPE_RADIAL;
        ipgui_radial_gradient_init(&grad.grad.radial_grad, BTN_W_NORMAL/2, BTN_H_NORMAL/2, IPGUI_MAX(BTN_W_NORMAL/2, BTN_H_NORMAL/2));
        ipgui_gradient_color_stop_t s0, s1;
        IPGUI_COLOR_SET(s0.color, 255, 0xFFFFFF); s0.pos = 0;
        IPGUI_COLOR_SET(s1.color, 255, 0x4080FF); s1.pos = 255;
        ipgui_radial_gradient_add_stop(&grad.grad.radial_grad, &s0);
        ipgui_radial_gradient_add_stop(&grad.grad.radial_grad, &s1);
        bg_grad(&bg, &grad);
        border_set(&bo, 1, 0x3070EF, 255);
        shadow_set(&sd, 0x4080FF, 60, 6, 0, 0, 4);
        draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);
    }
    x += BTN_W_NORMAL + PADDING;

    /* 18. 超大圆角 */
    radius_set(&sh, 20);
    bg_solid(&bg, 0x4080FF, 255);
    border_set(&bo, 0, 0, 0);
    shadow_set(&sd, 0x4080FF, 80, 8, 0, 0, 4);
    draw_one_button(&surf, x, y, BTN_W_NORMAL, BTN_H_NORMAL, &sh, &bg, &bo, &sd);

    /* 第五行 */
    x = PADDING;
    y += BTN_H_NORMAL + PADDING;

    /* ================================================================
     * Row 5: 大尺寸 + 扩散阴影
     * ================================================================ */
    radius_set(&sh, 10);

    /* 19. 渐变大按钮 + 强扩散阴影 */
    IPGUI_COLOR_SET(c1, 255, 0x1D2129);
    IPGUI_COLOR_SET(c2, 255, 0x4E5969);
    make_grad_ver(&grad, c1, c2, x, y, x+BTN_W_XLARGE-1, y+BTN_H_XLARGE-1);
    bg_grad(&bg, &grad);
    border_set(&bo, 1, 0x1D2129, 255);
    shadow_set(&sd, 0x000000, 100, 12, 2, 0, 6);
    draw_one_button(&surf, x, y, BTN_W_XLARGE, BTN_H_XLARGE, &sh, &bg, &bo, &sd);
    x += BTN_W_XLARGE + PADDING;

    /* 20. 无圆角直角按钮 */
    radius_set(&sh, 0);
    bg_solid(&bg, 0xF53F3F, 255);
    border_set(&bo, 2, 0xD93636, 255);
    shadow_set(&sd, 0xF53F3F, 80, 10, 0, 0, 6);
    draw_one_button(&surf, x, y, BTN_W_XLARGE, BTN_H_XLARGE, &sh, &bg, &bo, &sd);

    /* ---- 输出 PPM ---- */
    const char *filename = OUT_DIR "/buttons.ppm";
    if (0 == surf_to_ppm(&surf, filename)) {
        printf("OK: %s (%d x %d)\n", filename, CANVAS_W, CANVAS_H);
        ret = 0;
    } else {
        printf("ERROR: write PPM failed\n");
    }

    free(buf);
    return ret;
}
