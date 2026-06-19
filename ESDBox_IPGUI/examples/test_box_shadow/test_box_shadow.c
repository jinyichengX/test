/*===========================================================================
 * test_box_shadow.c — IPGUI 盒阴影全面测试
 *
 * 生成两个 PPM：
 *   v2: blur/spread/offset/opacity 参数对比
 *   v3: 边界情况（直角 / 大圆角 / 小盒子 / 有色阴影）
 *===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static FILE *fopen_utf8(const char *path, const char *mode)
{ wchar_t wp[MAX_PATH],wm[16]; MultiByteToWideChar(CP_UTF8,0,path,-1,wp,MAX_PATH);
  MultiByteToWideChar(CP_ACP,0,mode,-1,wm,16); return _wfopen(wp,wm); }
#else
#define fopen_utf8(path,mode) fopen(path,mode)
#endif

#include "ipgui_utils.h"
#include "ipgui_core.h"
#include "ipgui_color.h"
#include "ipgui_blend.h"
#include "ipgui_blend_color.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_draw_box_shadow.h"
#include "ipgui_draw_builtin_font.h"
#include "open_sans.h"

/* ---- 平台桩 ---- */
void ipgui_printk(char*fmt,...){}
void ipgui_memset(void*s,int c,unsigned int n){memset(s,c,n);}
void*ipgui_memcpy(void*d,const void*s,unsigned long long n){return memcpy(d,s,(size_t)n);}
void*ipgui_mem_alloc_def(unsigned long long s){return calloc(1,(size_t)s);}
void*ipgui_mem_alloc(unsigned long long s,void*p){(void)p;return calloc(1,(size_t)s);}
void ipgui_mem_free_def(void*p){free(p);}
void ipgui_mem_free_pool(void*p,void*pl){(void)pl;free(p);}
void*ipgui_mem_realoc(void*p,unsigned long long s,void*pl){(void)pl;return realloc(p,(size_t)s);}

#define CW   1200
#define CH   900
#define ODIR "."

/* ---- PPM 输出 ---- */
static int ppm_save(const ipgui_surf_t *s, const char *path)
{
    FILE *fp;
    int x, y;
    int w = s->surf.end.x - s->surf.start.x + 1;
    int h = s->surf.end.y - s->surf.start.y + 1;
    if (!s || !s->color || !path) return -1;
    fp = fopen_utf8(path, "wb");
    if (!fp) return -1;
    fprintf(fp, "P6\n%d %d\n255\n", w, h);
    for (y = 0; y < h; y++) {
        unsigned char *row = s->color + (size_t)y * s->stride;
        for (x = 0; x < w; x++) {
            unsigned char *px = row + (size_t)x * s->pix_size;
            unsigned char rgb[3] = {px[1], px[2], px[3]};
            fwrite(rgb, 1, 3, fp);
        }
    }
    fclose(fp);
    return 0;
}

static void fill_bg(ipgui_surf_t *s, u32_t rgb)
{
    ipgui_color_t c; IPGUI_COLOR_SET(c, 255, rgb);
    ipgui_blend_color(s, 0, &s->surf, c, 255, 0, 0, IPGUI_BLEND_NORMAL);
}

static void draw_label(ipgui_surf_t *s, int x, int y, const char *text)
{
    ipgui_font_style_t fs;
    fs.blend_mode = IPGUI_BLEND_NORMAL; fs.opacity = 200;
    fs.line_spacing = 0; fs.font = &open_sans_14px;
    fs.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(fs.paint.src.color, 255, 0x222222);
    ipgui_draw_builtin_text(s, 0, &fs, text, x, y);
}

/* ---- 绘制一个带阴影的盒 ---- */
static void draw_shadow_box(
    ipgui_surf_t *surf, int x, int y, int w, int h, int r,
    u32_t bg, u32_t bd, int bw, ipgui_box_shadow_style_t *ss)
{
    ipgui_aabb_t box = {{x,y},{x+w-1,y+h-1}};
    ipgui_box_style_t st;
    st.left_padding = st.right_padding = st.top_padding = st.bottom_padding = 0;
    st.left_top_radius = st.right_top_radius =
    st.left_bottom_radius = st.right_bottom_radius = r;

    if (ss) ipgui_draw_box_shadow(surf, 0, &box, &st, ss);

    ipgui_box_bg_style_t bgst;
    bgst.blend_mode = IPGUI_BLEND_NORMAL; bgst.opacity = 255;
    bgst.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(bgst.paint.src.color, 255, bg);
    ipgui_draw_box_background(surf, 0, &box, &st, &bgst);

    if (bw > 0) {
        ipgui_box_border_style_t bo;
        bo.blend_mode = IPGUI_BLEND_NORMAL; bo.opacity = 255;
        bo.width = bw; bo.paint.type = IPGUI_PAINT_COLOR;
        IPGUI_COLOR_SET(bo.paint.src.color, 255, bd);
        ipgui_draw_box_border(surf, 0, &box, &st, &bo);
    }
}

/* ---- 底部参数标签 ---- */
static void draw_params(ipgui_surf_t *s, int x, int y,
    const char *label, int blur, int spread, int ox, int oy)
{
    char buf[120];
    draw_label(s, x, y, label);
    snprintf(buf, sizeof(buf), "b=%d s=%d o=(%d,%d)", blur, spread, ox, oy);
    draw_label(s, x, y+18, buf);
}

/* ---- 带额外信息的参数标签 ---- */
static void draw_params_ex(ipgui_surf_t *s, int x, int y,
    const char *label, int blur, int spread, int ox, int oy, const char *extra)
{
    char buf[120];
    draw_label(s, x, y, label);
    snprintf(buf, sizeof(buf), "b=%d s=%d o=(%d,%d) %s", blur, spread, ox, oy, extra);
    draw_label(s, x, y+18, buf);
}

/* ---- 初始化阴影样式 ---- */
static void ss_init(ipgui_box_shadow_style_t *ss, u32_t rgb,
                    u8_t opacity, int blur, int spread, int ox, int oy)
{
    memset(ss, 0, sizeof(*ss));
    IPGUI_COLOR_SET(ss->color, 255, rgb);
    ss->opacity  = opacity;
    ss->blur     = blur;
    ss->spread   = spread;
    ss->offset_x = ox;
    ss->offset_y = oy;
}

/* ---- 章节标题 ---- */
static void section(ipgui_surf_t *s, int x, int y, const char *title)
{
    ipgui_font_style_t fs;
    fs.blend_mode = IPGUI_BLEND_NORMAL; fs.opacity = 255;
    fs.line_spacing = 0; fs.font = &open_sans_14px;
    fs.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(fs.paint.src.color, 255, 0x333333);
    /* 粗体效果：同位置画两次，偏移1px */
    ipgui_draw_builtin_text(s, 0, &fs, title, x+1, y);
    IPGUI_COLOR_SET(fs.paint.src.color, 255, 0x222288);
    ipgui_draw_builtin_text(s, 0, &fs, title, x, y);
}

/* =================================================================== */
int main(void)
{
    u32_t ps = 4, stride = CW * ps;
    u8_t *buf = (u8_t *)calloc(1, (size_t)CH * stride);
    if (!buf) return 1;

    ipgui_surf_t sf = {
        .surf = {{0,0},{CW-1,CH-1}}, .color = buf,
        .stride = stride, .pix_fmt = PIX_FMT_ARGB8888, .pix_size = ps
    };

    fill_bg(&sf, 0xF0F2F5);

    /* ================================================================
     * PPM v2: 参数对比测试
     *   1. Blur 渐变 (4级)
     *   2. Spread 渐变 (4级)
     *   3. 四方偏移
     *   4. 不透明度渐变 (4级)
     *=============================================================== */
    {
        int bx, by, bw=200, bh=100, r=12, gap=60;
        ipgui_box_shadow_style_t ss;

        /* ---- 1. Blur: 2 / 6 / 10 / 18 ---- */
        section(&sf, 30, 10, "1. Blur Progression (s=0, o=0, op=120, r=12)");
        bx=30; by=35;
        ss_init(&ss, 0x000000, 120, 2, 0, 0, 0);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0x4A90D9, 0xFFFFFF, 2, &ss);
        draw_params(&sf, bx, by+bh+4, "A", 2, 0, 0, 0); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 6, 0, 0, 0);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0x4A90D9, 0xFFFFFF, 2, &ss);
        draw_params(&sf, bx, by+bh+4, "B", 6, 0, 0, 0); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 10, 0, 0, 0);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0x4A90D9, 0xFFFFFF, 2, &ss);
        draw_params(&sf, bx, by+bh+4, "C", 10, 0, 0, 0); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 18, 0, 0, 0);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0x4A90D9, 0xFFFFFF, 2, &ss);
        draw_params(&sf, bx, by+bh+4, "D", 18, 0, 0, 0);

        /* ---- 2. Spread: 0 / 3 / 6 / 10 ---- */
        section(&sf, 30, 180, "2. Spread Progression (b=6, o=0, op=100, r=12)");
        bx=30; by=205;
        ss_init(&ss, 0x000000, 100, 6, 0, 0, 0);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xE85D75, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "A", 6, 0, 0, 0); bx += bw+gap;

        ss_init(&ss, 0x000000, 100, 6, 3, 0, 0);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xE85D75, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "B", 6, 3, 0, 0); bx += bw+gap;

        ss_init(&ss, 0x000000, 100, 6, 6, 0, 0);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xE85D75, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "C", 6, 6, 0, 0); bx += bw+gap;

        ss_init(&ss, 0x000000, 100, 6, 10, 0, 0);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xE85D75, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "D", 6, 10, 0, 0);

        /* ---- 3. 四方偏移: 右下 / 左下 / 右上 / 左上 ---- */
        section(&sf, 30, 350, "3. Offset Directions (b=8, s=2, op=110, r=12)");
        bx=30; by=375;

        ss_init(&ss, 0x000000, 110, 8, 2, 6, 6);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0x50B86C, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "A", 8, 2, 6, 6); bx += bw+gap;

        ss_init(&ss, 0x000000, 110, 8, 2, -6, 6);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0x50B86C, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "B", 8, 2, -6, 6); bx += bw+gap;

        ss_init(&ss, 0x000000, 110, 8, 2, 6, -6);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0x50B86C, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "C", 8, 2, 6, -6); bx += bw+gap;

        ss_init(&ss, 0x000000, 110, 8, 2, -6, -6);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0x50B86C, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "D", 8, 2, -6, -6);

        /* ---- 4. Opacity: 40 / 80 / 130 / 200 ---- */
        section(&sf, 30, 520, "4. Opacity (b=8, s=2, o=(4,4), r=12)");
        bx=30; by=545;

        ss_init(&ss, 0x000000, 40, 8, 2, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xF5A623, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "A", 8, 2, 4, 4); bx += bw+gap;

        ss_init(&ss, 0x000000, 80, 8, 2, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xF5A623, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "B", 8, 2, 4, 4); bx += bw+gap;

        ss_init(&ss, 0x000000, 130, 8, 2, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xF5A623, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "C", 8, 2, 4, 4); bx += bw+gap;

        ss_init(&ss, 0x000000, 200, 8, 2, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xF5A623, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "D", 8, 2, 4, 4);

        /* ---- 5. 无边框对比 (blur 8 s=2 vs s=0) ---- */
        section(&sf, 30, 690, "5. No Border (b=8, o=(4,4), op=100, r=12)");
        bx=30; by=715;
        r=10;

        ss_init(&ss, 0x000000, 100, 8, 0, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xFFFFFF, 0, 0, &ss);
        draw_params(&sf, bx, by+bh+4, "A", 8, 0, 4, 4); bx += bw+gap;

        ss_init(&ss, 0x000000, 100, 8, 3, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xFFFFFF, 0, 0, &ss);
        draw_params(&sf, bx, by+bh+4, "B", 8, 3, 4, 4); bx += bw+gap;

        ss_init(&ss, 0x000000, 100, 12, 2, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xFFFFFF, 0, 0, &ss);
        draw_params(&sf, bx, by+bh+4, "C", 12, 2, 4, 4); bx += bw+gap;

        ss_init(&ss, 0x000000, 100, 4, 4, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, r, 0xFFFFFF, 0, 0, &ss);
        draw_params(&sf, bx, by+bh+4, "D", 4, 4, 4, 4);
    }
    ppm_save(&sf, ODIR "/test_box_shadow_v2.ppm");

    /* ================================================================
     * PPM v3: 边界情况
     *   1. 直角矩形 (r=0)
     *   2. 小圆角 (r=3)
     *   3. 大圆角 (r=25)
     *   4. 小盒子 (60x60)
     *   5. 有颜色阴影
     *=============================================================== */
    fill_bg(&sf, 0xF0F2F5);
    {
        int bx, by, bw=180, bh=100, gap=60;
        ipgui_box_shadow_style_t ss;

        /* ---- 1. 直角 r=0 ---- */
        section(&sf, 30, 10, "6. Sharp Corners r=0 (b=4/8/14 s=0/2, op=120)");
        bx=30; by=35;

        ss_init(&ss, 0x000000, 120, 4, 0, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 0, 0x6C5CE7, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "A", 4, 0, 3, 3); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 8, 0, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 0, 0x6C5CE7, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "B", 8, 0, 3, 3); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 14, 0, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 0, 0x6C5CE7, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "C", 14, 0, 3, 3); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 8, 4, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 0, 0x6C5CE7, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "D", 8, 4, 3, 3);

        /* ---- 2. 小圆角 r=3 ---- */
        section(&sf, 30, 180, "7. Small Radius r=3 (b=4/8/12 s=0/3, op=120)");
        bx=30; by=205;

        ss_init(&ss, 0x000000, 120, 4, 0, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 3, 0x00B894, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "A", 4, 0, 3, 3); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 8, 0, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 3, 0x00B894, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "B", 8, 0, 3, 3); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 12, 0, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 3, 0x00B894, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "C", 12, 0, 3, 3); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 8, 3, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 3, 0x00B894, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "D", 8, 3, 3, 3);

        /* ---- 3. 大圆角 r=25 ---- */
        section(&sf, 30, 350, "8. Large Radius r=25 (b=4/8/16 s=0/4, op=110)");
        bx=30; by=375;

        ss_init(&ss, 0x000000, 110, 4, 0, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 25, 0xE17055, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "A", 4, 0, 3, 3); bx += bw+gap;

        ss_init(&ss, 0x000000, 110, 8, 0, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 25, 0xE17055, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "B", 8, 0, 3, 3); bx += bw+gap;

        ss_init(&ss, 0x000000, 110, 16, 0, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 25, 0xE17055, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "C", 16, 0, 3, 3); bx += bw+gap;

        ss_init(&ss, 0x000000, 110, 8, 4, 3, 3);
        draw_shadow_box(&sf, bx, by, bw, bh, 25, 0xE17055, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "D", 8, 4, 3, 3);

        /* ---- 4. 小盒子 60x60 ---- */
        section(&sf, 30, 510, "9. Small Box 60x60 (b=3/6/10 s=0/2, op=120, r=8)");
        bw=60; bh=60; gap=80;
        bx=30; by=535;

        ss_init(&ss, 0x000000, 120, 3, 0, 2, 2);
        draw_shadow_box(&sf, bx, by, bw, bh, 8, 0x0984E3, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "A", 3, 0, 2, 2); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 6, 0, 2, 2);
        draw_shadow_box(&sf, bx, by, bw, bh, 8, 0x0984E3, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "B", 6, 0, 2, 2); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 10, 0, 2, 2);
        draw_shadow_box(&sf, bx, by, bw, bh, 8, 0x0984E3, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "C", 10, 0, 2, 2); bx += bw+gap;

        ss_init(&ss, 0x000000, 120, 6, 3, 2, 2);
        draw_shadow_box(&sf, bx, by, bw, bh, 8, 0x0984E3, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "D", 6, 3, 2, 2);

        /* ---- 5. 有颜色阴影 ---- */
        section(&sf, 30, 670, "10. Colored Shadows (b=8, s=2, o=(4,4), op=130, r=12)");
        bw=180; bh=100; gap=60;
        bx=30; by=695;

        /* 同色系 */
        ss_init(&ss, 0x4A90D9, 130, 8, 2, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, 12, 0x4A90D9, 0xFFFFFF, 1, &ss);
        draw_params_ex(&sf, bx, by+bh+4, "A", 8, 2, 4, 4, "match"); bx += bw+gap;

        /* 对比色 */
        ss_init(&ss, 0xE85D75, 130, 8, 2, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, 12, 0x4A90D9, 0xFFFFFF, 1, &ss);
        draw_params_ex(&sf, bx, by+bh+4, "B", 8, 2, 4, 4, "contrast"); bx += bw+gap;

        /* 发光效果 (浅色背景+同色系阴影) */
        ss_init(&ss, 0x50B86C, 150, 10, 4, 0, 0);
        draw_shadow_box(&sf, bx, by, bw, bh, 12, 0x50B86C, 0xFFFFFF, 1, &ss);
        draw_params_ex(&sf, bx, by+bh+4, "C", 10, 4, 0, 0, "glow"); bx += bw+gap;

        /* 悬浮卡片 (深色阴影+大偏移) */
        ss_init(&ss, 0x000000, 100, 12, 2, 0, 8);
        draw_shadow_box(&sf, bx, by, bw, bh, 12, 0xFFFFFF, 0xE0E0E0, 1, &ss);
        draw_params_ex(&sf, bx, by+bh+4, "D", 12, 2, 0, 8, "float");

        /* ---- 6. 极端参数压力测试 ---- */
        section(&sf, 30, 810, "11. Extreme: blur=30 r=4 | blur=2 spread=15 | blur=20 spread=8 o=(-8,-8)");
        bw=180; bh=100; gap=60;
        bx=30; by=835;

        ss_init(&ss, 0x000000, 120, 30, 0, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, 4, 0xFDCB6E, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "A", 30, 0, 4, 4); bx += bw+gap;

        ss_init(&ss, 0x000000, 100, 2, 15, 4, 4);
        draw_shadow_box(&sf, bx, by, bw, bh, 4, 0xFDCB6E, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "B", 2, 15, 4, 4); bx += bw+gap;

        ss_init(&ss, 0x000000, 110, 20, 8, -8, -8);
        draw_shadow_box(&sf, bx, by, bw, bh, 4, 0xFDCB6E, 0xFFFFFF, 1, &ss);
        draw_params(&sf, bx, by+bh+4, "C", 20, 8, -8, -8);
    }
    ppm_save(&sf, ODIR "/test_box_shadow_v3.ppm");

    free(buf);
    printf("PPM saved: v2, v3\n");
    return 0;
}
