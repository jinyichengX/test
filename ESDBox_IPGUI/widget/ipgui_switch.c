#include "ipgui_switch.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void sw_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_switch_t * sw = (ipgui_switch_t *)w;
    ipgui_aabb_t box;
    box.start.x = 0; box.start.y = 0;
    box.end.x = w->w - 1; box.end.y = w->h - 1;

    ipgui_box_bg_style_t * bg = sw->style.toggled ? &sw->style.bg_on : &sw->style.bg_off;
    ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &box, &sw->style.shape, bg);
    ipgui_draw_box_border(ctx->surf, (ipgui_aabb_t *)0, &box, &sw->style.shape, &sw->style.border);

    /* knob — 半径 = h/2 - 2 */
    int kw = w->h - 6, kh = kw;
    if (kw < 6) kw = kh = 6;
    int kx = sw->style.toggled ? (w->w - kw - 3) : 3;
    int ky = (w->h - kh) / 2;

    ipgui_aabb_t knob = {{kx, ky}, {kx + kw - 1, ky + kh - 1}};
    ipgui_box_style_t knob_shape = {0};
    int kr = kw / 2;
    knob_shape.left_top_radius = knob_shape.right_top_radius =
        knob_shape.left_bottom_radius = knob_shape.right_bottom_radius = kr;
    ipgui_box_bg_style_t knob_bg;
    knob_bg.paint.type = IPGUI_PAINT_COLOR;
    knob_bg.paint.src.color = sw->style.knob_color;
    knob_bg.opacity = 255;
    knob_bg.blend_mode = IPGUI_BLEND_NORMAL;
    ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &knob, &knob_shape, &knob_bg);
}

__IPGUI_API__ void ipgui_switch_toggle(ipgui_switch_t * sw)
{
    if (!sw) return;
    sw->style.toggled = !sw->style.toggled;
}

__IPGUI_API__ void ipgui_switch_style_init(ipgui_switch_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius = 20;

    s->bg_on.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->bg_on.paint.src.color, 255, 0x34C759);
    s->bg_on.opacity = 255;
    s->bg_on.blend_mode = IPGUI_BLEND_NORMAL;

    s->bg_off.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->bg_off.paint.src.color, 255, 0xCCCCCC);
    s->bg_off.opacity = 255;
    s->bg_off.blend_mode = IPGUI_BLEND_NORMAL;

    s->border.opacity = 0;
    IPGUI_COLOR_SET(s->knob_color, 255, 0xFFFFFF);
    s->toggled = 0;
}

__IPGUI_API__ ipgui_switch_t * ipgui_switch_create(ipgui_widget_t * parent)
{
    ipgui_switch_t * sw = (ipgui_switch_t *)ipgui_widget_create(parent);
    if (!sw) return (ipgui_switch_t *)0;
    ipgui_switch_style_init(&sw->style);
    ipgui_widget_set_render(&sw->base, sw_render);
    return sw;
}
