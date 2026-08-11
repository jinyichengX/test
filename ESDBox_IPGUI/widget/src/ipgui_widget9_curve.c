#include "ipgui_widget.h"
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_draw_line.h"
#include "ipgui_time.h"
#include <math.h>

#define CURVE_MAX_POINTS 800
#define CURVE_LINE_WIDTH  2

/* 环形数据缓冲区 */
static int curve_buf[CURVE_MAX_POINTS];
static int curve_cnt = 0;
static int curve_t   = 0;

/* HTML Demo 同款数据生成：频率=10(speed=10), 噪声=0 */
static int curve_gen_data(void)
{
    float base = 40.0f * sinf((float)curve_t * 0.05f)
               + 20.0f * sinf((float)curve_t * 0.13f)
               + 15.0f * sinf((float)curve_t * 0.03f);
    curve_t++;
    return (int)base;
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

    ipgui_line_style_t style;
    style.width      = CURVE_LINE_WIDTH;
    style.cap        = IPGUI_LINE_CAP_ROUND;
    style.opacity    = 255;
    style.blend_mode = IPGUI_BLEND_NORMAL;
    style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(style.paint.src.color, 255, 0x00BFFF);

    float x_step = (float)widget->w / (float)(CURVE_MAX_POINTS - 1);

    /* 环形缓冲区读取起点 */
    int start = (curve_cnt > CURVE_MAX_POINTS)
                ? (curve_cnt % CURVE_MAX_POINTS) : 0;

    ipgui_line_t line;
    int i;
    for (i = 0; i < total - 1; i++) {
        int i0 = (start + i)     % CURVE_MAX_POINTS;
        int i1 = (start + i + 1) % CURVE_MAX_POINTS;

        /* 缩放因子匹配 HTML：h/200 使纵向比例与 255/120 视觉效果一致 */
        line.start.x = (ipgui_coord_t)((float)i * x_step);
        line.start.y = (ipgui_coord_t)((float)widget->h / 2.0f
                       - (float)curve_buf[i0] * (float)widget->h / 200.0f);
        line.end.x   = (ipgui_coord_t)((float)(i + 1) * x_step);
        line.end.y   = (ipgui_coord_t)((float)widget->h / 2.0f
                       - (float)curve_buf[i1] * (float)widget->h / 200.0f);

        ipgui_draw_line_generic(ctx->surf, (ipgui_aabb_t *)0, &line, &style);
    }
}

void widget_curve_update(ipgui_widget_t * w)
{
    ipgui_widget_mark_dirty(w);
}
