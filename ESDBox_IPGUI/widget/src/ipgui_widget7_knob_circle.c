#include "ipgui_widget.h"
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_draw_filled_circle.h"
#include <math.h>

/* 圆环参数（与 ipgui_widget6_arc_bg.c 中 arc 保持一致） */
#define ARC_CX    105
#define ARC_CY    283
#define ARC_RAD   170   /* (er + ir) / 2 ≈ 170，小圆圆心沿此半径运动 */
#define KNOB_R    13    /* 小圆半径 */
#define KNOB_MARGIN 20  /* 屏幕左边缘余量，给手指留空间 */

static float knob_angle = 0.0f;  /* 当前旋钮角度（弧度），供外部读取 */
static int   knob_moved = 0;     /* 本次拖动过，供 per-frame 复位 */

/* 弧线仅左侧被屏幕裁剪，只约束 cx >= MARGIN → 转化到角度域 clamp */
static void _knob_clamp_angle(float * angle)
{
    float ang_min = -3.14159265f;
    float ang_max =  3.14159265f;

    /* cx = ARC_CX + ARC_RAD * cos(angle) >= KNOB_MARGIN
     * → cos(angle) >= (KNOB_MARGIN - ARC_CX) / ARC_RAD */
    float cos_min = ((float)(KNOB_MARGIN - ARC_CX)) / (float)ARC_RAD;
    if (cos_min > -1.0f) {
        float a = acosf(cos_min);
        /* a ∈ [0,π], 对应 |angle| ≤ a  */
        if (-a > ang_min) ang_min = -a;
        if ( a < ang_max) ang_max =  a;
    }

    if (*angle < ang_min) *angle = ang_min;
    if (*angle > ang_max) *angle = ang_max;
}

void knob_circle_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_filled_circle_style_t style;
    style.opacity    = 200;
    style.blend_mode = IPGUI_BLEND_NORMAL;
    style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(style.paint.src.color, 255, IPGUI_COLOR_WHITE);

    ipgui_draw_filled_circle(
        ctx->surf,
        (ipgui_aabb_t *)0,
        KNOB_R,
        KNOB_R,
        KNOB_R,
        &style);
}

void knob_circle_event_handler(struct ipgui_widget * widget, ipgui_widget_evt_t * evt)
{
    if (evt->type == IPGUI_WIDGET_EVENT_PRESSED) {
        /* 移动前先标记旧位置脏 */
        ipgui_widget_mark_dirty(widget);

        /* 将 widget 本地坐标转为屏幕坐标 */
        ipgui_aabb_t aabb;
        ipgui_widget_abs_pos(widget, &aabb);
        ipgui_coord_t screen_x = aabb.start.x + evt->evt.pressed_evt.x;
        ipgui_coord_t screen_y = aabb.start.y + evt->evt.pressed_evt.y;

        /* 计算手指相对于圆环中心的角度 */
        float dx = (float)((s32_t)screen_x - ARC_CX);
        float dy = (float)((s32_t)screen_y - ARC_CY);
        float angle = atan2f(dy, dx);

        /* 约束角度，确保圆心在屏幕可见范围内（留手指余量） */
        _knob_clamp_angle(&angle);

        knob_angle = angle;  /* 保存当前角度 */
        knob_moved = 1;      /* 标记发生过拖动 */

        /* 将小圆圆心约束到圆环轨迹上（角度已 clamp，一定在圆上且在屏幕内） */
        ipgui_coord_t new_cx = ARC_CX + (ipgui_coord_t)(ARC_RAD * cosf(angle));
        ipgui_coord_t new_cy = ARC_CY + (ipgui_coord_t)(ARC_RAD * sinf(angle));

        widget->x = new_cx - KNOB_R;
        widget->y = new_cy - KNOB_R;

        ipgui_widget_mark_dirty(widget);
    }
}

/* 外部获取亮度值 0-100
 * 角度范围 [-120°, 120°] 映射到 [0, 100] */
int knob_get_value(void)
{
    float deg = knob_angle * 180.0f / 3.14159265f;
    int val = (int)((deg + 120.0f) * 100.0f / 240.0f + 0.5f);
    if (val < 0)   val = 0;
    if (val > 100) val = 100;
    return val;
}

/* 每帧复位，返回旧值，用于 brightness label 的 dirty 标记 */
static int knob_moved_reset(void)
{
    int prev = knob_moved;
    knob_moved = 0;
    return prev;
}

void widget_knob_update(ipgui_widget_t * w)
{
    if (knob_moved_reset())
        ipgui_widget_mark_dirty(w);
}
