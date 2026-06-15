#include "ipgui_label.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void label_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_label_t * lbl = (ipgui_label_t *)w;

    ipgui_aabb_t box;
    box.start.x = 0;
    box.start.y = 0;
    box.end.x   = w->w - 1;
    box.end.y   = w->h - 1;

    /* bg + border */
    ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &box, &lbl->style.shape, &lbl->style.bg);
    ipgui_draw_box_border(ctx->surf, (ipgui_aabb_t *)0, &box, &lbl->style.shape, &lbl->style.border);
}

__IPGUI_API__ void ipgui_label_style_init(ipgui_label_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->bg.paint.type = IPGUI_PAINT_COLOR;
    s->bg.paint.src.color = (ipgui_color_t){.r=240,.g=240,.b=240,.a=255};
    s->bg.opacity    = 255;
    s->bg.blend_mode = IPGUI_BLEND_NORMAL;
    s->border = (ipgui_box_border_style_t){0};
    s->border.opacity = 0;
    IPGUI_COLOR_SET(s->text_color, 255, 0x333333);
    s->align = IPGUI_TEXT_ALIGN_LEFT;
    s->text  = (const char *)0;
}

__IPGUI_API__ ipgui_label_t * ipgui_label_create(ipgui_widget_t * parent)
{
    ipgui_label_t * lbl = (ipgui_label_t *)ipgui_widget_create(parent);
    if (!lbl) return (ipgui_label_t *)0;
    ipgui_label_style_init(&lbl->style);
    ipgui_widget_set_render(&lbl->base, label_render);
    return lbl;
}
