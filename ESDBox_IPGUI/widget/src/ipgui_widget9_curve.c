#include "ipgui_widget.h"
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_draw_line.h"
#include "ipgui_time.h"
#include <math.h>

#define CURVE_MAX_POINTS 400
#define CURVE_LINE_WIDTH  3

/* 环形数据缓冲区 */
static int curve_buf[CURVE_MAX_POINTS];
static int curve_cnt = 0;
static int curve_t   = 0;

/* 模拟传感器数据：正弦波叠加 + 伪随机噪声 */
static int curve_gen_data(void)
{
    float base = 40.0f * sinf((float)curve_t * 0.05f)
               + 20.0f * sinf((float)curve_t * 0.13f)
               + 15.0f * sinf((float)curve_t * 0.03f);
    /* MCU 友好噪声：用 tick 取模 */
    int noise = (int)(ipgui_sys_tick % 41) - 20;
    curve_t++;
    return (int)base + noise;
}

void curve_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    if (!widget || !ctx || !ctx->surf) return;
    if (widget->w < 2 || widget->h < 2) return;

    /* 每帧产生新数据点，写入环形缓冲区 */
    curve_buf[curve_cnt % CURVE_MAX_POINTS] = curve_gen_data();
    curve_cnt++;

    int total = (curve_cnt < CURVE_MAX_POINTS) ? curve_cnt : CURVE_MAX_POINTS;
    if (total < 2) return;

    /* 准备线段样式：线宽 1，红色，平头端点 */
    ipgui_line_style_t style;
    style.width      = CURVE_LINE_WIDTH;
    style.cap        = IPGUI_LINE_CAP_BUTT;
    style.opacity    = 255;
    style.blend_mode = IPGUI_BLEND_NORMAL;
    style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(style.paint.src.color, 255, 0x00BFFF);

    ipgui_coord_t mid_y = widget->h / 2;
    float x_step = (float)widget->w / (float)(CURVE_MAX_POINTS - 1);

    /* 环形缓冲区读取起点 */
    int start = (curve_cnt > CURVE_MAX_POINTS)
                ? (curve_cnt % CURVE_MAX_POINTS) : 0;

    ipgui_line_t line;
    int i;
    for (i = 0; i < total - 1; i++) {
        int i0 = (start + i)     % CURVE_MAX_POINTS;
        int i1 = (start + i + 1) % CURVE_MAX_POINTS;

        /* 数据值映射到 Y 坐标：范围 [-60, 60] -> 上下各半 */
        line.start.x = (ipgui_coord_t)((float)i * x_step);
        line.start.y = mid_y - (ipgui_coord_t)((float)curve_buf[i0] * (float)widget->h / 120.0f);
        line.end.x   = (ipgui_coord_t)((float)(i + 1) * x_step);
        line.end.y   = mid_y - (ipgui_coord_t)((float)curve_buf[i1] * (float)widget->h / 120.0f);

        ipgui_draw_line_generic(ctx->surf, (ipgui_aabb_t *)0, &line, &style);
    }
}

void widget_curve_update(ipgui_widget_t * w)
{
    ipgui_widget_mark_dirty(w);
}
