/*
 * ipgui_draw_image_in_rect 全覆盖测试
 *
 * 目标：9 对齐 × 4 缩放 = 36 组合 + 边界异常
 *
 * 编译（m:\test 目录）：
 *   gcc -o ESDBox_IPGUI/examples/test_image/test_draw_image_api.exe
 *       ESDBox_IPGUI/examples/test_image/test_draw_image_api.c
 *       ESDBox_IPGUI/core/gfx/ipgui_draw_image_api.c
 *       ESDBox_IPGUI/core/gfx/ipgui_draw_image.c
 *       ESDBox_IPGUI/core/gfx/ipgui_image_buf.c
 *       ESDBox_IPGUI/core/gfx/ipgui_mask_buf.c
 *       ESDBox_IPGUI/core/composite/blend_image/ipgui_blend_image.c
 *       ESDBox_IPGUI/core/composite/blend_color/ipgui_blend_color.c
 *       ESDBox_IPGUI/core/misc/ipgui_prim.c
 *       ESDBox_IPGUI/core/image/proc/ipgui_image_geometry_transform.c
 *       ESDBox_IPGUI/base/src/ipgui_math.c
 *       -I ESDBox_IPGUI -I ESDBox_IPGUI/Include -I ESDBox_IPGUI/base/inc
 *       -I ESDBox_IPGUI/base/src -I ESDBox_IPGUI/core -I ESDBox_IPGUI/core/misc
 *       -I ESDBox_IPGUI/core/composite -I ESDBox_IPGUI/core/composite/blend_color
 *       -I ESDBox_IPGUI/core/composite/blend_image
 *       -I ESDBox_IPGUI/core/composite/blend_gradient
 *       -I ESDBox_IPGUI/core/gfx -I ESDBox_IPGUI/core/image
 *       -I ESDBox_IPGUI/core/image/proc -Wall -Wextra
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static FILE * fopen_utf8(const char * path, const char * mode)
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
#include "ipgui_blend_image.h"
#include "ipgui_draw_image.h"
#include "ipgui_draw_image_api.h"
#include "ipgui_image.h"

/* ---- 框架桩 ---- */
void ipgui_printk(char * fmt, ...) { (void)fmt; }
void ipgui_memset(void * s, int c, unsigned int n) { memset(s, c, n); }
void * ipgui_memcpy(void * dst, const void * src, unsigned long long n)
    { return memcpy(dst, src, (size_t)n); }
void * ipgui_mem_alloc_def(unsigned long long size) { return calloc(1, (size_t)size); }
void * ipgui_mem_alloc(unsigned long long size, void * pool) { (void)pool; return calloc(1, (size_t)size); }
void ipgui_mem_free_def(void * ptr) { free(ptr); }
void ipgui_mem_free_pool(void * ptr, void * pool) { (void)pool; free(ptr); }
void * ipgui_mem_realoc(void * ptr, unsigned long long size, void * pool) { (void)pool; return realloc(ptr, (size_t)size); }

/* ---- 常量 ---- */
#define IMG_PATH    "ESDBox_IPGUI/core/image/decoder/material/bmp/keli.bmp"
#define OUT_DIR     "ESDBox_IPGUI/examples/test_image"
#define SURF_W      150
#define SURF_H      100
#define OPACITY     180
#define BORDER_W    2

/* ==================================================================
 * BMP 加载
 * ================================================================== */
static int load_bmp(const char * path, ipgui_img_dsc_t * dsc)
{
    FILE * fp;
    unsigned char fh[14], bh[40];
    unsigned int pix_off, w, h, bpp, comp, row_size;
    int y;

    if (!path || !dsc) return -1;
    fp = fopen(path, "rb");
    if (!fp) return -1;
    if (fread(fh, 1, 14, fp) != 14 || fh[0] != 'B' || fh[1] != 'M') { fclose(fp); return -1; }
    pix_off = fh[10]|((unsigned)fh[11]<<8)|((unsigned)fh[12]<<16)|((unsigned)fh[13]<<24);
    if (fread(bh, 1, 40, fp) != 40) { fclose(fp); return -1; }
    w   = bh[4]|((unsigned)bh[5]<<8)|((unsigned)bh[6]<<16)|((unsigned)bh[7]<<24);
    h   = bh[8]|((unsigned)bh[9]<<8)|((unsigned)bh[10]<<16)|((unsigned)bh[11]<<24);
    bpp = bh[14]|((unsigned)bh[15]<<8);
    comp= bh[16]|((unsigned)bh[17]<<8)|((unsigned)bh[18]<<16)|((unsigned)bh[19]<<24);
    if (comp || (bpp != 24 && bpp != 32)) { fclose(fp); return -1; }
    fseek(fp, pix_off, SEEK_SET);
    row_size = ((bpp/8)*w + 3) & ~3u;
    {
        unsigned char * buf = calloc(1, (size_t)row_size * h);
        if (!buf) { fclose(fp); return -1; }
        for (y = (int)h - 1; y >= 0; y--) {
            if (fread(buf + (size_t)y * row_size, 1, row_size, fp) != row_size)
                { free(buf); fclose(fp); return -1; }
        }
        dsc->w = (int)w; dsc->h = (int)h; dsc->stride = (int)row_size;
        dsc->pixmap = buf; dsc->mask = (unsigned char *)0;
        dsc->fmt = (bpp == 24) ? IPGUI_IMG_FMT_BGR888 : IPGUI_IMG_FMT_BGRA8888;
    }
    fclose(fp); return 0;
}

/* ==================================================================
 * PPM (P6 binary)
 * ================================================================== */
static int surf_to_ppm(const ipgui_surf_t * surf, const char * path)
{
    FILE * fp;
    int x, y, w = surf->surf.end.x - surf->surf.start.x + 1,
               h = surf->surf.end.y - surf->surf.start.y + 1;
    if (!surf || !surf->color || !path) return -1;
    fp = fopen_utf8(path, "wb");
    if (!fp) return -1;
    fprintf(fp, "P6\n%d %d\n255\n", w, h);
    for (y = 0; y < h; y++) {
        unsigned char * row = surf->color + (size_t)y * surf->stride;
        for (x = 0; x < w; x++) {
            static unsigned char rgb[3];
            unsigned char * px = row + (size_t)x * surf->pix_size;
            rgb[0] = px[0]; rgb[1] = px[1]; rgb[2] = px[2];
            fwrite(rgb, 1, 3, fp);
        }
    }
    fclose(fp);
    return 0;
}

/* ==================================================================
 * 蓝色边框 (width=2)
 * ================================================================== */
static void draw_blue_border(ipgui_surf_t * surf, const ipgui_aabb_t * box)
{
    ipgui_aabb_t e;
    ipgui_color_t blue;
    IPGUI_COLOR_SET(blue, 255, 0x0000FF);

    /* top */
    e.start.x = box->start.x; e.start.y = box->start.y;
    e.end.x   = box->end.x;   e.end.y   = box->start.y + BORDER_W - 1;
    ipgui_fill_color(surf, 0, &e, blue, 255, IPGUI_BLEND_NORMAL);

    /* bottom */
    e.start.x = box->start.x; e.start.y = box->end.y - BORDER_W + 1;
    e.end.x   = box->end.x;   e.end.y   = box->end.y;
    ipgui_fill_color(surf, 0, &e, blue, 255, IPGUI_BLEND_NORMAL);

    /* left */
    e.start.x = box->start.x; e.start.y = box->start.y;
    e.end.x   = box->start.x + BORDER_W - 1; e.end.y = box->end.y;
    ipgui_fill_color(surf, 0, &e, blue, 255, IPGUI_BLEND_NORMAL);

    /* right */
    e.start.x = box->end.x - BORDER_W + 1; e.start.y = box->start.y;
    e.end.x   = box->end.x;                e.end.y   = box->end.y;
    ipgui_fill_color(surf, 0, &e, blue, 255, IPGUI_BLEND_NORMAL);
}

/* ==================================================================
 * 枚举 → 文件名片段
 * ================================================================== */
static const char * align_str(ipgui_image_align_t a) {
    static const char * t[] = {
        "top_left","top","top_right",
        "left","center","right",
        "bottom_left","bottom","bottom_right"
    };
    return t[a];
}
static const char * fit_str(ipgui_image_fit_t f) {
    static const char * t[] = {"none","fit","fill","stretch"};
    return t[f];
}

/* ==================================================================
 * 单次测试
 * ================================================================== */
static int run_one_case(
    ipgui_image_data_t * img,
    ipgui_image_align_t align,
    ipgui_image_fit_t   fit,
    ipgui_image_draw_style_t * style,
    const char * filename)
{
    unsigned char * buf = (unsigned char *)calloc(1, (size_t)SURF_W * SURF_H * 4);
    if (!buf) return 0;

    /* ----- surf & target 同尺寸 150×100 ----- */
    ipgui_surf_t surf;
    surf.color       = buf;
    surf.surf.start.x = 0;  surf.surf.start.y = 0;
    surf.surf.end.x   = SURF_W - 1;  surf.surf.end.y = SURF_H - 1;
    surf.pix_fmt      = PIX_FMT_RGBA8888;
    surf.pix_size     = 4;
    surf.stride       = SURF_W * 4;

    ipgui_aabb_t target = surf.surf;  /* 必须一样大 */

    /* 1. 白底 */
    ipgui_color_t white; IPGUI_COLOR_SET(white, 255, 0xFFFFFF);
    ipgui_fill_color(&surf, 0, &target, white, 255, IPGUI_BLEND_NORMAL);

    /* 2. target 涂黑 */
    ipgui_color_t black; IPGUI_COLOR_SET(black, 255, 0x000000);
    ipgui_fill_color(&surf, 0, &target, black, 255, IPGUI_BLEND_NORMAL);

    /* 3. 画图 (opacity=180) */
    ipgui_draw_image_in_rect(&surf, img, &target, align, fit, style);

    /* 4. 蓝色边框 (width=2) */
    draw_blue_border(&surf, &target);

    /* 5. PPM */
    int ok = (0 == surf_to_ppm(&surf, filename));
    free(buf);
    return ok;
}

/* ==================================================================
 * main
 * ================================================================== */
int main(void)
{
    ipgui_img_dsc_t          dsc;
    ipgui_image_data_t       img;
    ipgui_image_draw_style_t style;
    int a, f, total = 0, pass = 0, fail = 0;

    printf("=== ipgui_draw_image_in_rect full coverage test ===\n");

    /* ==== load ==== */
    printf("[load] %s\n", IMG_PATH);
    if (0 != load_bmp(IMG_PATH, &dsc)) {
        fprintf(stderr, "FATAL: load failed\n");
        return EXIT_FAILURE;
    }
    img.pixmap  = dsc.pixmap;
    img.px_size = dsc.stride / dsc.w;
    img.fmt     = dsc.fmt;
    img.stride  = dsc.stride;
    img.w       = dsc.w;
    img.h       = dsc.h;
    printf("  size=%dx%d px_size=%d\n", (int)img.w, (int)img.h, (int)img.px_size);

    style.opacity    = OPACITY;
    style.blend_mode = IPGUI_BLEND_NORMAL;

    /* ==== 9 align × 4 fit = 36 ==== */
    printf("\n[36 cases] align × fit (surf=target=%dx%d, opacity=%d)\n",
           SURF_W, SURF_H, OPACITY);

    for (a = IPGUI_IMG_ALIGN_TOP_LEFT; a <= IPGUI_IMG_ALIGN_BOTTOM_RIGHT; a++) {
        for (f = IPGUI_IMG_FIT_NONE; f <= IPGUI_IMG_FIT_STRETCH; f++) {
            char path[256];
            sprintf(path, "%s/fit_%s_align_%s.ppm",
                    OUT_DIR, fit_str((ipgui_image_fit_t)f),
                    align_str((ipgui_image_align_t)a));
            total++;
            if (run_one_case(&img, (ipgui_image_align_t)a,
                              (ipgui_image_fit_t)f, &style, path)) {
                pass++;
                printf("  PASS  %s\n", path);
            } else {
                fail++;
                printf("  FAIL  %s\n", path);
            }
        }
    }

    /* ==== NULL 边界 ==== */
    printf("\n[edge] NULL parameters\n");
    {
        unsigned char buf[SURF_W * SURF_H * 4];
        ipgui_surf_t s;
        ipgui_aabb_t t = { {0,0}, {SURF_W-1, SURF_H-1} };
        s.color = buf; s.surf = t; s.pix_fmt = PIX_FMT_RGBA8888;
        s.pix_size = 4; s.stride = SURF_W * 4;

        printf("  TEST  NULL surf... ");
        ipgui_draw_image_in_rect(NULL, &img, &t, IPGUI_IMG_ALIGN_CENTER,
                                  IPGUI_IMG_FIT_NONE, &style);
        printf("OK\n"); total++; pass++;

        printf("  TEST  NULL img... ");
        ipgui_draw_image_in_rect(&s, NULL, &t, IPGUI_IMG_ALIGN_CENTER,
                                  IPGUI_IMG_FIT_NONE, &style);
        printf("OK\n"); total++; pass++;

        printf("  TEST  NULL target... ");
        ipgui_draw_image_in_rect(&s, &img, NULL, IPGUI_IMG_ALIGN_CENTER,
                                  IPGUI_IMG_FIT_NONE, &style);
        printf("OK\n"); total++; pass++;

        printf("  TEST  NULL style... ");
        ipgui_draw_image_in_rect(&s, &img, &t, IPGUI_IMG_ALIGN_CENTER,
                                  IPGUI_IMG_FIT_NONE, NULL);
        printf("OK\n"); total++; pass++;
    }

    /* ==== 结果 ==== */
    free(dsc.pixmap);
    printf("\n========================================\n");
    printf("TOTAL=%d PASS=%d FAIL=%d\n", total, pass, fail);
    printf("========================================\n");
    return (fail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
