#include "ipgui_color_swatch.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void sw_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_color_swatch_t * sw = (ipgui_color_swatch_t *)w;
    ipgui_aabb_t box;
    box.start.x = 0; box.start.y = 0;
    box.end.x = w->w - 1; box.end.y = w->h - 1;

    /* fill with color */
    ipgui_box_bg_style_t bg;
    bg.paint.type = IPGUI_PAINT_COLOR;
    bg.paint.src.color = sw->style.color;
    bg.opacity = 255;
    bg.blend_mode = IPGUI_BLEND_NORMAL;
    ipgui_draw_box_background(ctx->surf, 0, &box, &sw->style.shape, &bg);

    if (sw->style.has_shadow)
        ipgui_draw_box_shadow(ctx->surf, 0, &box, &sw->style.shape, &sw->style.shadow);
    ipgui_draw_box_border(ctx->surf, 0, &box, &sw->style.shape, &sw->style.border);

    /* selected ring */
    if (sw->style.selected) {
        ipgui_box_border_style_t rb = {0};
        rb.paint.type = IPGUI_PAINT_COLOR;
        IPGUI_COLOR_SET(rb.paint.src.color, 255, 0x4080FF);
        rb.opacity = 255;
        rb.width = 3;
        rb.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_aabb_t out = {{-2, -2}, {w->w + 1, w->h + 1}};
        ipgui_draw_box_border(ctx->surf, 0, &out, &sw->style.shape, &rb);
    }
}

__IPGUI_API__ void ipgui_color_swatch_style_init(ipgui_color_swatch_style_t * s)
{
    if (!s) return;
    IPGUI_COLOR_SET(s->color, 255, 0xCCCCCC);
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius = 6;
    s->border = (ipgui_box_border_style_t){0};
    s->border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->border.paint.src.color, 255, 0xDDDDDD);
    s->border.opacity = 255;
    s->border.width = 1;
    s->border.blend_mode = IPGUI_BLEND_NORMAL;
    s->shadow = (ipgui_box_shadow_style_t){0};
    s->has_shadow = 0;
    s->selected = 0;
}

__IPGUI_API__ ipgui_color_swatch_t * ipgui_color_swatch_create(ipgui_widget_t * parent)
{
    ipgui_color_swatch_t * sw = (ipgui_color_swatch_t *)ipgui_widget_create(parent);
    if (!sw) return (ipgui_color_swatch_t *)0;
    ipgui_color_swatch_style_init(&sw->style);
    ipgui_widget_set_render(&sw->base, sw_render);
    return sw;
}
