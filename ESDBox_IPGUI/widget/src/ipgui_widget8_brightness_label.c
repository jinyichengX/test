#include "ipgui_widget.h"
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_draw_builtin_font.h"
#include "quicksand_medium.h"
#include <stdio.h>

extern int knob_get_value(void);

void brightness_label_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    int val = knob_get_value();

    s8_t buf[8];
    snprintf(buf, sizeof(buf), "%d%%", val);

    ipgui_font_style_t font_style;
    font_style.blend_mode   = IPGUI_BLEND_NORMAL;
    font_style.opacity      = 200;
    font_style.line_spacing = 0;
    font_style.font         = &quicksand_medium_30px;
    font_style.paint.type   = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(font_style.paint.src.color, 255, IPGUI_COLOR_WHITE);

    /* 柔和文字阴影：先阴影后正文；内存不足时内部会降级硬阴影 */
    ipgui_text_shadow_style_t shadow;
    shadow.opacity    = 255;
    shadow.blur       = 5;
    shadow.offset_x   = 0;
    shadow.offset_y   = 0;
    shadow.quality    = IPGUI_TEXT_SHADOW_QUALITY_HIGH;
    shadow.blend_mode = IPGUI_BLEND_NORMAL;
    IPGUI_COLOR_SET(shadow.color, 255, IPGUI_COLOR_RED);

    ipgui_coord_t tw = ipgui_builtin_text_width(&quicksand_medium_30px, buf);
    ipgui_coord_t x = (widget->w - tw) / 2;

    ipgui_draw_builtin_text_shadowed(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &font_style,
        &shadow,
        buf,
        x,
        0);
}
