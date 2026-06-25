#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_draw_arc.h"
#include "ipgui_widget.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_draw_filled_circle.h"
#include "ipgui_draw_builtin_font.h"
#include "ipgui_anim_bounce.h"
#include "open_sans.h"
#include <math.h>
#include <stddef.h>
void widget1_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
   
}

void widget_arc_label_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    // ipgui_font_style_t font_style;
    // font_style.blend_mode = 0;

    // font_style.opacity = 200;
    // font_style.line_spacing = 0;
    // font_style.font = &open_sans_40px;
    // font_style.paint.type = IPGUI_PAINT_COLOR;
    // IPGUI_COLOR_SET(font_style.paint.src.color, 255, IPGUI_COLOR_WHITE);
  
    // ipgui_draw_builtin_text(
    //     ctx->surf,
    //     (ipgui_aabb_t *)0,
    //     &font_style,
    //     "1 %",
    //     20,
    //     0);
    
    ipgui_font_style_t font1_style;
    font1_style.blend_mode = 0;

    font1_style.opacity = 200;
    font1_style.line_spacing = 0;
    font1_style.font = &open_sans_18px;
    font1_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(font1_style.paint.src.color, 255, IPGUI_COLOR_WHITE);
        ipgui_draw_builtin_text(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &font1_style,
        "Brightness",
        10,
        37);
}

#include "ipgui_time.h"
void widget_switch_label_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_font_style_t font_style;
    font_style.blend_mode = 0;

    font_style.opacity = 255;
    font_style.line_spacing = 0;
    font_style.font = &open_sans_19px;
    font_style.paint.type = IPGUI_PAINT_GRADIENT;

    /* 使用 ipgui_sys_tick 驱动渐变色平滑变化 */
    /* 周期 = 800 ticks，相位归一化到 [0, 2π) */
    float t = (float)(ipgui_sys_tick % 533) * 6.283185f / 533.0f;

    ipgui_color_t col_stop0;
    ipgui_color_t col_stop255;

    /* stop0: 三个通道各自正弦，峰值在 64~240 范围，避免过暗/过亮 */
    col_stop0.a = 255;
    col_stop0.r = (u8_t)(152.0f + 88.0f * sinf(t));
    col_stop0.g = (u8_t)(152.0f + 88.0f * sinf(t + 2.094395f));
    col_stop0.b = (u8_t)(152.0f + 88.0f * sinf(t + 4.188790f));

    /* stop255: 相对于 stop0 相位偏移一半周期，形成对比 */
    col_stop255.a = 255;
    col_stop255.r = (u8_t)(152.0f + 88.0f * sinf(t + 3.141593f));
    col_stop255.g = (u8_t)(152.0f + 88.0f * sinf(t + 5.235988f));
    col_stop255.b = (u8_t)(152.0f + 88.0f * sinf(t + 7.330383f));

    font_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
    ipgui_liner_gradient_init_direct(&font_style.paint.src.grad_src.grad.liner_grad, 
        0,0, 50,50);

    ipgui_gradient_color_stop_t stop0;
    stop0.pos = 0;
    stop0.color = col_stop0;
    ipgui_liner_gradient_add_stop(&font_style.paint.src.grad_src.grad.liner_grad, &stop0);

    ipgui_gradient_color_stop_t stop255;
    stop255.pos = 255;
    stop255.color = col_stop255;
    ipgui_liner_gradient_add_stop(&font_style.paint.src.grad_src.grad.liner_grad, &stop255);

    ipgui_draw_builtin_text(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &font_style,
        "Auto-Adjust",
        0,
        0);
}

static ipgui_coord_t knob_cx = 21;
static ipgui_box_bg_style_t box_bg_style;

/* 开关动画状态：使用动画引擎 + bounce 曲线 */
static ipgui_anim_t *knob_anim = NULL;
static ipgui_coord_t knob_anim_start_pos = 0;
static ipgui_coord_t knob_anim_target_pos = 0;
static ipgui_tick_t knob_anim_begin_tick = 0;
#define KNOB_ANIM_DURATION 80

void widget_switch_event_handler(struct ipgui_widget * widget, ipgui_widget_evt_t * evt)
{
    if (evt->type == IPGUI_WIDGET_EVENT_RELEASED) {
        /* 动画播放中，忽略新的释放事件 */
        if (knob_anim != NULL) return;

        if(knob_cx == 21) {
            box_bg_style.opacity = 170;
            IPGUI_COLOR_SET(box_bg_style.paint.src.color, 255, 0x2196F3);
            knob_anim_start_pos = 21;
            knob_anim_target_pos = 70;
        } else {
            box_bg_style.opacity = 100;
            IPGUI_COLOR_SET(box_bg_style.paint.src.color, 255, 0x909090);
            knob_anim_start_pos = 70;
            knob_anim_target_pos = 21;
        }

        /* 创建弹性动画（bounce 曲线） */
        ipgui_anim_dsc_t dsc = {0};
        dsc.anim_func    = ipgui_anim_bounce;
        dsc.t1           = 0;
        dsc.t2           = KNOB_ANIM_DURATION;
        dsc.loop_count   = 1;
        dsc.auto_destroy = 1;

        knob_anim = ipgui_anim_create(&dsc);
        if (knob_anim) {
            ipgui_anim_start(knob_anim);
            knob_anim_begin_tick = ipgui_sys_tick;
        }

        ipgui_widget_mark_dirty(widget);
    }
}

int widget_switch_is_animating(void)
{
    return knob_anim != NULL;
}

void widget_switch_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    /* 弹性动画插值：从动画引擎获取 bounce 曲线值 */
    if (knob_anim != NULL) {
        ipgui_tick_t elapsed = ipgui_sys_tick - knob_anim_begin_tick;
        if (elapsed >= KNOB_ANIM_DURATION) {
            /* 动画引擎已 auto_destroy，仅归位并清空指针 */
            knob_cx = knob_anim_target_pos;
            knob_anim = NULL;
        } else {
            ipgui_anim_value_t v = ipgui_anim_get_value(knob_anim);
            ipgui_coord_t diff = knob_anim_target_pos - knob_anim_start_pos;
            /* v ∈ [0, t2] 线性映射到 [start, target]，bounce 振荡叠加其上 */
            knob_cx = knob_anim_start_pos
                + (ipgui_coord_t)(diff * (s32_t)v / KNOB_ANIM_DURATION);
        }
    }

    static ipgui_coord_t box_offset = 5;
    ipgui_aabb_t box;
    box.start.x = 0 + box_offset;
    box.start.y = 0 + box_offset;
    box.end.x = 80 + box_offset;
    box.end.y = 30 + box_offset;
    
    ipgui_box_style_t box_style;
    box_style.bottom_padding = 0;
    box_style.left_padding = 0;
    box_style.top_padding = 0;
    box_style.right_padding = 0;

    box_style.left_bottom_radius = 15;
    box_style.left_top_radius = 15;
    box_style.right_bottom_radius = 15;
    box_style.right_top_radius = 15;


    box_bg_style.blend_mode = 0;

    box_bg_style.paint.type = IPGUI_PAINT_COLOR;

    ipgui_box_border_style_t box_border_style;
    box_border_style.blend_mode = 0;
    box_border_style.width = 1;
    box_border_style.opacity = 155;
    box_border_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(box_border_style.paint.src.color, 255, IPGUI_COLOR_164);

    ipgui_draw_box_background(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &box,
        &box_style,
        &box_bg_style);

    ipgui_draw_box_border(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &box,
        &box_style,
        &box_border_style);



    static ipgui_coord_t knob_cy = 20;
    static ipgui_coord_t knob_r = 13;
    static ipgui_filled_circle_style_t knob_style;

    knob_style.opacity = 200;
    knob_style.blend_mode = IPGUI_BLEND_NORMAL;
    knob_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(knob_style.paint.src.color, 255, IPGUI_COLOR_WHITE);
    ipgui_draw_filled_circle(
        ctx->surf,
        (ipgui_aabb_t *) 0,
        knob_cx,
        knob_cy,
        knob_r,
        &knob_style
    );
}