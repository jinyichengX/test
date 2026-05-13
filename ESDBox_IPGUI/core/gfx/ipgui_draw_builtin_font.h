#ifndef IPGUI_DRAW_BUILTIN_FONT_H
#define IPGUI_DRAW_BUILTIN_FONT_H

#include "ipgui_core.h"
#include "ipgui_blend.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 一些基本概念：
 * 字体风格：每种字体的所表现的风格形状不同 
 * 字型font：是字体风格的表现形式，特定风格+特定字号的具体实例，同一种字体不同大小也属于不同font
 * glyph字形：一种font中每个符号形状，是font的子集 
 */
typedef struct ipgui_glyph_t{
    ipgui_coord_t width;
    ipgui_coord_t height;

    ipgui_coord_t bearing_x, bearing_y, advance;
    // ipgui_coord_t verti_bearingX, verti_bearingY, verti_advance; /* 在垂直排版时才用 */ /* 垂直排版参数与换行参数无关，换行参数是font统一指定的 */

    const u8_t  * cover_map; /* 抗锯齿0-255 */
}ipgui_glyph_t;

typedef struct ipgui_font_t {
    ipgui_glyph_t * glyphs; /* 字形 */
    ipgui_coord_t   line_height; /* 行高 */
    ipgui_coord_t   baseline; /* 基线 */
    ipgui_coord_t   space_width; /* 空格或者其他无效字符偏移此距离 */
    ipgui_coord_t   font_size; /* 暂时无效 */
    ipgui_coord_t   max_height;/* 暂时无效 */
}ipgui_font_t;

typedef struct {
    const ipgui_font_t * font;            /* pointer to font */
    ipgui_paint_t        paint;           /* paint type */
    u8_t                 opacity;         /* global opacity (0-255) */
    u16_t                line_spacing;    /* additional line spacing */ /* 行间距 */
    ipgui_blend_mode_t   blend_mode;
} ipgui_font_style_t;

typedef enum {
    IPGUI_TEXT_ALIGN_LEFT = 0,      /* left alignment */
    IPGUI_TEXT_ALIGN_CENTER,        /* center alignment */
    IPGUI_TEXT_ALIGN_RIGHT,         /* right alignment */
} ipgui_text_align_t;

__IPGUI_STATIC__ __IPGUI_INLINE__ 
const ipgui_glyph_t * ipgui_font_get_glyph(
    const ipgui_font_t * font, 
    u8_t char_code)
{
    if (!font || !font->glyphs || char_code >= 128) {
        return (ipgui_glyph_t *)0;
    }
    return &font->glyphs[char_code];
}

extern __IPGUI_API__ ipgui_coord_t ipgui_draw_builtin_char(
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_font_style_t * style,
    ipgui_coord_t        x,
    ipgui_coord_t        y,
    u8_t                 ch);

extern __IPGUI_API__ ipgui_coord_t ipgui_draw_builtin_text(
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_font_style_t * style,
    const s8_t         * text,
    ipgui_coord_t        x,
    ipgui_coord_t        y);

extern __IPGUI_API__ ipgui_coord_t ipgui_builtin_text_width(
    const ipgui_font_t * font, 
    const s8_t         * text);
    
#ifdef __cplusplus
}
#endif

#endif /* IPGUI_FONT_RAS_H */