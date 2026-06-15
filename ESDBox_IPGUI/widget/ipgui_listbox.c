#include "ipgui_listbox.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void listbox_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_listbox_t * lb = (ipgui_listbox_t *)w;

    ipgui_aabb_t box;
    box.start.x = 0;
    box.start.y = 0;
    box.end.x   = w->w - 1;
    box.end.y   = w->h - 1;

    ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &box, &lb->style.shape, &lb->style.bg);
    ipgui_draw_box_border(ctx->surf, (ipgui_aabb_t *)0, &box, &lb->style.shape, &lb->style.border);

    int ih = lb->style.item_h > 0 ? lb->style.item_h : 28;
    int i;
    for (i = 0; i < lb->style.count && i < 16; i++) {
        int iy = 2 + i * ih;
        ipgui_aabb_t item_box = {{1, iy}, {w->w - 2, iy + ih - 2}};

        /* selected highlight */
        if (i == lb->style.selected) {
            ipgui_box_bg_style_t sel_bg;
            sel_bg.paint.type = IPGUI_PAINT_COLOR;
            IPGUI_COLOR_SET(sel_bg.paint.src.color, 255, 0x4080FF);
            sel_bg.opacity    = 40;
            sel_bg.blend_mode = IPGUI_BLEND_NORMAL;
            ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &item_box, &lb->style.shape, &sel_bg);
        }
    }
}

__IPGUI_API__ void ipgui_listbox_style_init(ipgui_listbox_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius = 6;

    s->bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->bg.paint.src.color, 255, 0xFFFFFF);
    s->bg.opacity = 255;
    s->bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->border.paint.src.color, 255, 0xCCCCCC);
    s->border.opacity = 255;
    s->border.width   = 1;
    s->border.blend_mode = IPGUI_BLEND_NORMAL;

    IPGUI_COLOR_SET(s->text_color, 255, 0x333333);
    s->count    = 0;
    s->selected = -1;
    s->item_h   = 28;
}

__IPGUI_API__ ipgui_listbox_t * ipgui_listbox_create(ipgui_widget_t * parent)
{
    ipgui_listbox_t * lb = (ipgui_listbox_t *)ipgui_widget_create(parent);
    if (!lb) return (ipgui_listbox_t *)0;
    ipgui_listbox_style_init(&lb->style);
    ipgui_widget_set_render(&lb->base, listbox_render);
    return lb;
}
