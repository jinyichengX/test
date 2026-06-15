#include "ipgui_spinner.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void sp_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_spinner_t * sp = (ipgui_spinner_t *)w;
    int cx = w->w / 2, cy = w->h / 2;
    int r = (IPGUI_MIN(w->w, w->h) - sp->style.line_width) / 2;
    if (r < 2) r = 2;

    /* track ring */
    if (sp->style.track_color.a > 0) {
        ipgui_arc_t arc = {
            .cx = cx, .cy = cy, .er = r,
            .ir = r - sp->style.line_width,
            .start = 0, .dir = IPGUI_ARC_DRAW_DIR_CW, .angle = 360
        };
        ipgui_arc_style_t st = {
            .paint = {.type = IPGUI_PAINT_COLOR},
            .opacity = 255,
            .sep_type = IPGUI_ARC_ENDPOINT_TYPE_BUTT,
            .eep_type = IPGUI_ARC_ENDPOINT_TYPE_BUTT,
            .blend_mode = IPGUI_BLEND_NORMAL
        };
        st.paint.src.color = sp->style.track_color;
        st.paint.src.color.a = 60;
        ipgui_draw_arc(ctx->surf, 0, &arc, &st);
    }

    /* progress arc */
    int angle = sp->style.indeterminate ? 270 : (sp->style.progress * 360 / 100);
    if (angle < 1) angle = 1;

    ipgui_arc_t arc = {
        .cx = cx, .cy = cy, .er = r,
        .ir = r - sp->style.line_width,
        .start = -90, .dir = IPGUI_ARC_DRAW_DIR_CW, .angle = angle
    };
    ipgui_arc_style_t st = {
        .paint = {.type = IPGUI_PAINT_COLOR},
        .opacity = 255,
        .sep_type = IPGUI_ARC_ENDPOINT_TYPE_ROUND,
        .eep_type = IPGUI_ARC_ENDPOINT_TYPE_ROUND,
        .blend_mode = IPGUI_BLEND_NORMAL
    };
    st.paint.src.color = sp->style.color;
    ipgui_draw_arc(ctx->surf, 0, &arc, &st);
}

__IPGUI_API__ void ipgui_spinner_style_init(ipgui_spinner_style_t * s)
{
    if (!s) return;
    IPGUI_COLOR_SET(s->color, 255, 0x4080FF);
    IPGUI_COLOR_SET(s->track_color, 255, 0xCCCCCC);
    s->progress = 60;
    s->line_width = 6;
    s->indeterminate = 0;
}

__IPGUI_API__ ipgui_spinner_t * ipgui_spinner_create(ipgui_widget_t * parent)
{
    ipgui_spinner_t * sp = (ipgui_spinner_t *)ipgui_widget_create(parent);
    if (!sp) return (ipgui_spinner_t *)0;
    ipgui_spinner_style_init(&sp->style);
    ipgui_widget_set_render(&sp->base, sp_render);
    return sp;
}
