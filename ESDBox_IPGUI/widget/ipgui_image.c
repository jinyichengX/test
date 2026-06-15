#include "ipgui_image.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void img_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_image_t * img = (ipgui_image_t *)w;
    if (!img->style.img || !img->style.img->pixmap) return;

    ipgui_aabb_t box;
    box.start.x = 0;
    box.start.y = 0;
    box.end.x   = w->w - 1;
    box.end.y   = w->h - 1;

    ipgui_draw_image_in_rect(
        ctx->surf,
        img->style.img,
        &box,
        img->style.align,
        img->style.fit,
        &img->style.draw_style);
}

__IPGUI_API__ void ipgui_image_style_init(ipgui_image_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->draw_style.opacity   = 255;
    s->draw_style.blend_mode = IPGUI_BLEND_NORMAL;
    s->align  = IPGUI_IMG_ALIGN_CENTER;
    s->fit    = IPGUI_IMG_FIT_STRETCH;
    s->img    = (ipgui_image_data_t *)0;
}

__IPGUI_API__ ipgui_image_t * ipgui_image_create(ipgui_widget_t * parent)
{
    ipgui_image_t * img = (ipgui_image_t *)ipgui_widget_create(parent);
    if (!img) return (ipgui_image_t *)0;
    ipgui_image_style_init(&img->style);
    ipgui_widget_set_render(&img->base, img_render);
    return img;
}
