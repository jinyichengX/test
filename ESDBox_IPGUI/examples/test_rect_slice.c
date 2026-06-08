/*
 * ipgui_rect_slice 算法测试
 * 编译: gcc -o test_rect_slice.exe test_rect_slice.c && test_rect_slice.exe
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ============ 类型（与项目一致，独立编译无需项目头文件） ============ */
typedef int s32_t;
typedef int ipgui_coord_t;

#define IPGUI_MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct { ipgui_coord_t x, y; } ipgui_point_t;
typedef struct { ipgui_point_t start, end; } ipgui_rect_t;

typedef struct {
    ipgui_rect_t * rect;
    ipgui_coord_t   remain_w, remain_h, slice_len;
    ipgui_coord_t   full_w, full_h;
    ipgui_coord_t   orig_start_x, orig_end_x, orig_bottom;
} ipgui_rect_slice_ctx;

/* ============ 算法 ============ */
static void ctx_init(ipgui_rect_slice_ctx * ctx, ipgui_rect_t * rect, ipgui_coord_t slice_len)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->rect         = rect;
    ctx->remain_w     = rect->end.x - rect->start.x + 1;
    ctx->remain_h     = rect->end.y - rect->start.y + 1;
    ctx->slice_len    = slice_len;
    ctx->full_w       = ctx->remain_w;
    ctx->full_h       = ctx->remain_h;
    ctx->orig_start_x = rect->start.x;
    ctx->orig_end_x   = rect->end.x;
    ctx->orig_bottom  = rect->end.y + 1;
}

static s32_t next_slice(ipgui_rect_slice_ctx * ctx, ipgui_rect_t * res)
{
    ipgui_coord_t strip_w, rows;

    if (ctx->remain_w <= 0) return 0;

    if (ctx->remain_h <= 0) {
        ctx->remain_w -= IPGUI_MIN(ctx->remain_w, ctx->slice_len);
        ctx->remain_h  = ctx->full_h;
        if (ctx->remain_w <= 0) return 0;
    }

    strip_w = IPGUI_MIN(ctx->remain_w, ctx->slice_len);
    rows    = IPGUI_MIN(ctx->remain_h, ctx->slice_len / strip_w);

    res->start.x = ctx->orig_start_x + (ctx->full_w - ctx->remain_w);
    res->start.y = ctx->orig_bottom  - ctx->remain_h;
    res->end.x   = res->start.x + strip_w - 1;
    res->end.y   = res->start.y + rows    - 1;

    ctx->remain_h -= rows;
    return 1;
}

/* ============ 测试接口（你只需要看这个） ============ */
static void test_rect_slice(ipgui_rect_t rect, ipgui_coord_t slice_len)
{
    ipgui_rect_slice_ctx ctx;
    ipgui_rect_t slice;
    int count = 0;
    int total_pixels = 0;
    int rect_area = (rect.end.x - rect.start.x + 1) * (rect.end.y - rect.start.y + 1);

    ctx_init(&ctx, &rect, slice_len);

    printf("----------------------------------------\n");
    printf("矩形: (%d,%d)-(%d,%d)  %dx%d  area=%d  slice_len=%d\n",
           rect.start.x, rect.start.y, rect.end.x, rect.end.y,
           rect.end.x - rect.start.x + 1,
           rect.end.y - rect.start.y + 1,
           rect_area, slice_len);

    while (next_slice(&ctx, &slice)) {
        int w = slice.end.x - slice.start.x + 1;
        int h = slice.end.y - slice.start.y + 1;
        total_pixels += w * h;
        printf("  #%3d: (%3d,%3d)-(%3d,%3d)  w=%3d h=%3d\n",
               ++count, slice.start.x, slice.start.y,
               slice.end.x, slice.end.y, w, h);
    }

    int optimal = (rect_area + slice_len - 1) / slice_len;
    printf("总次数: %d  理论下界: %d  slack: %+d  总面积: %d/%d",
           count, optimal, count - optimal, total_pixels, rect_area);
    if (total_pixels != rect_area)
        printf("  *** 面积不匹配! ***");
    printf("\n");
}

/* ============ main ============ */
int main(void)
{
    /* ---- 1. 非零起点测试 ---- */
    printf("===== 非零起点矩形测试 =====\n");
    test_rect_slice((ipgui_rect_t){{10, 20}, {109, 119}}, 99);

    /* ---- 2. 零起点 100x100, 多组 slice_len ---- */
    printf("\n===== 100x100 矩形, 多组 slice_len =====\n");
    int lens[] = {1, 2, 5, 10, 20, 33, 50, 99, 100, 200, 500, 1000, 5000, 10000, 20000};
    int n = sizeof(lens) / sizeof(lens[0]);
    for (int i = 0; i < n; i++)
        test_rect_slice((ipgui_rect_t){{0, 0}, {99, 99}}, lens[i]);

    /* ---- 3. 随机起点，多种尺寸 ---- */
    printf("\n===== 其他尺寸和起点 =====\n");
    test_rect_slice((ipgui_rect_t){{5, 5},  {24, 24}},   50);   /* 20x20 */
    test_rect_slice((ipgui_rect_t){{100, 0}, {199, 49}}, 120);  /* 100x50 */
    test_rect_slice((ipgui_rect_t){{0, 100}, {79, 179}}, 300);  /* 80x80 */

    return 0;
}


