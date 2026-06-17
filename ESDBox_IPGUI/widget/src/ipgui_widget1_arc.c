#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_draw_arc.h"
#include "ipgui_widget.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_draw_filled_circle.h"
#include "ipgui_draw_builtin_font.h"
#include "open_sans.h"
void widget1_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
   
}

void widget_arc_label_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_font_style_t font_style;
    font_style.blend_mode = 0;

    font_style.opacity = 200;
    font_style.line_spacing = 0;
    font_style.font = &open_sans_40px;
    font_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(font_style.paint.src.color, 255, IPGUI_COLOR_WHITE);
  
    ipgui_draw_builtin_text(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &font_style,
        "1 %",
        20,
        0);
    
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
        "brightness",
        5,
        37);
}


void widget_switch_label_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_font_style_t font_style;
    font_style.blend_mode = 0;

    font_style.opacity = 255;
    font_style.line_spacing = 0;
    font_style.font = &open_sans_18px;
    font_style.paint.type = IPGUI_PAINT_GRADIENT;
    IPGUI_COLOR_SET(font_style.paint.src.color, 255, IPGUI_COLOR_288);
        font_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
    ipgui_liner_gradient_init_direct(&font_style.paint.src.grad_src.grad.liner_grad, 
        0,0, 400,0);
    ipgui_gradient_color_stop_t stop111;
    stop111.pos = 0;
    IPGUI_COLOR_SET(stop111.color, 255, IPGUI_COLOR_WHITE);
    ipgui_gradient_color_stop_t stop222;
    stop222.pos = 255;
    IPGUI_COLOR_SET(stop222.color, 255, IPGUI_COLOR_483);
    ipgui_liner_gradient_add_stop(&font_style.paint.src.grad_src.grad.liner_grad, &stop111);
    ipgui_liner_gradient_add_stop(&font_style.paint.src.grad_src.grad.liner_grad, &stop222);
    ipgui_draw_builtin_text(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &font_style,
        "Auto-Adjust",
        0,
        0);
}

void widget_switch_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
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

    ipgui_box_bg_style_t box_bg_style;
    box_bg_style.blend_mode = 0;
    box_bg_style.opacity = 100;
    box_bg_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(box_bg_style.paint.src.color, 255, 0x909090);

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


    static ipgui_coord_t knob_cx = 21;
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