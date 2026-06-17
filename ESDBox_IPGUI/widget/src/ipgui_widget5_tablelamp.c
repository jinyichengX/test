#include "ipgui_draw_image_api.h"
#include "ipgui_widget.h"
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_draw_builtin_font.h"
#include "open_sans.h"

extern ipgui_image_data_t tablelamp_img;

void tablelamp_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_aabb_t box;
    box.start.x = 5;
    box.start.y = 5;
    box.end.x = 95;
    box.end.y = 85;

    ipgui_box_style_t box_style;
    box_style.bottom_padding = 0;
    box_style.left_padding = 0;
    box_style.top_padding = 0;
    box_style.right_padding = 0;
    box_style.left_top_radius = 10;
    box_style.right_top_radius = 10;
    box_style.left_bottom_radius = 10;
    box_style.right_bottom_radius = 10;

    ipgui_box_bg_style_t box_bg_style;
    box_bg_style.blend_mode = IPGUI_BLEND_NORMAL;
    box_bg_style.opacity = 150;
    box_bg_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(box_bg_style.paint.src.color, 255, IPGUI_COLOR_BLACK);

    ipgui_draw_box_background(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &box,
        &box_style,
        &box_bg_style);

    ipgui_box_border_style_t box_border_style;
    box_border_style.blend_mode = IPGUI_BLEND_NORMAL;
    box_border_style.width = 1;
    box_border_style.opacity = 100;
    box_border_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(box_border_style.paint.src.color, 255, IPGUI_COLOR_471);

    ipgui_draw_box_border(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &box,
        &box_style,
        &box_border_style);

    ipgui_image_draw_style_t img_style;
    img_style.blend_mode = IPGUI_BLEND_NORMAL;
    img_style.opacity = 200;

    ipgui_draw_image_at(
        ctx->surf,
        &tablelamp_img,
        32,
        15,
        &img_style);

    ipgui_font_style_t font_style;
    font_style.blend_mode = IPGUI_BLEND_NORMAL;
    font_style.opacity = 200;
    font_style.line_spacing = 0;
    font_style.font = &open_sans_15px;
    font_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(font_style.paint.src.color, 255, IPGUI_COLOR_WHITE);

    ipgui_draw_builtin_text(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &font_style,
        "Light ON",
        20,
        60);
}
