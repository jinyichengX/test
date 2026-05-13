#include "ipgui_draw_builtin_font.h"
#include "ipgui_debug.h"

/* Internal function to draw a single glyph */
__IPGUI_STATIC__ void font_draw_glyph(
    ipgui_surf_t        * surf,
    ipgui_aabb_t        * clip,
    ipgui_coord_t         baseline_x,
    ipgui_coord_t         baseline_y,
    const ipgui_glyph_t * glyph,
    ipgui_font_style_t  * style)
{
    ipgui_aabb_t glyph_mask, draw;
    
    if (!glyph || !glyph->cover_map || 
        glyph->width == 0 || glyph->height == 0) {
        return;
    }
    
    if (clip) {
        if (ipgui_aabb_overlap(&draw, clip, &surf->surf) != 0) {
            return; /* No intersection with clip area */
        }
    } else draw = surf->surf;
    
    /* calculate glyph bounding box and clip with draw */
    glyph_mask.start.x = baseline_x + glyph->bearing_x;
    glyph_mask.start.y = baseline_y - glyph->bearing_y;
    glyph_mask.end.x   = glyph_mask.start.x + glyph->width  - 1;
    glyph_mask.end.y   = glyph_mask.start.y + glyph->height - 1;
    if (ipgui_aabb_overlap(&draw, &draw, &glyph_mask) != 0) {
        return;
    }
    
    /* blend glyph pixels */
    const u8_t * mask = glyph->cover_map;

    ipgui_blend(
        surf,
        (ipgui_aabb_t *)0,
        &draw,
        &style->paint,
        style->opacity,
        mask,
        &glyph_mask,
        style->blend_mode);
    
}

/* 返回字符的宽度(advance) */
__IPGUI_API__ ipgui_coord_t ipgui_draw_builtin_char(
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_font_style_t * style,
    ipgui_coord_t        x,
    ipgui_coord_t        y,
    u8_t                 ch)
{
    if (!style || !style->font || style->opacity < 3) {
        return 0;
    }
    
    /* Handle special characters */
    if (ch == ' ') {
        return style->font->space_width;
    }
    
    if (ch == '\n' || ch == '\r') {
        return 0;
    }
    
    const ipgui_glyph_t * glyph = ipgui_font_get_glyph(style->font, ch);
    if (!glyph || !glyph->cover_map || glyph->width == 0) {
        /* use question mark for unknown characters */
        glyph = ipgui_font_get_glyph(style->font, '?');
        if (!glyph) {
            return style->font->space_width;
        }
    }
    
    /* render the glyph */
    font_draw_glyph(
        surf, 
        clip, 
        x, 
        y + style->font->baseline, 
        glyph, 
        style);
    
    return glyph->advance;
}


/* x,y is the left top coordinate */
__IPGUI_API__ ipgui_coord_t ipgui_draw_builtin_text(
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_font_style_t * style,
    const s8_t         * text,
    ipgui_coord_t        x,
    ipgui_coord_t        y)
{
    ipgui_coord_t start_x = x;        /* 每行起始x */
    ipgui_coord_t cur_baseline_x = x; /* currentx, currenty是基线上的点 */
    ipgui_coord_t cur_baseline_y = y + style->font->baseline;
    u16_t word_width = 0;
    const s8_t * word_start = (const s8_t *)0;
    
    if (!surf || !style || !style->font || !text || style->opacity < 3) {
        return y;
    }
    
    while (* text) {
        u8_t ch = (u8_t)(* text);
        
        if (ch == '\n') {
            /* new line */
            cur_baseline_x = start_x;
            cur_baseline_y += style->font->line_height + style->line_spacing;
            word_width = 0;
            word_start = (const s8_t *)0;
            text ++;
            continue;
        }
        
        if (ch == ' ') {
            /* render space */
            cur_baseline_x += style->font->space_width;
            word_width = 0;
            word_start = (const s8_t *)0;
            text ++;
            continue;
        }
        
        if (ch == '\r') {
            /* ignore carriage return */
            text ++;
            continue;
        }
        
        /* get glyph for character */
        const ipgui_glyph_t * glyph = ipgui_font_get_glyph(style->font, ch);
        if (!glyph) {
            /* use question mark for unknown characters */
            glyph = ipgui_font_get_glyph(style->font, '?');
            if (!glyph) {
                text ++;
                continue;
            }
        }
        
        /* render the character */
        if (glyph->width > 0 && glyph->height > 0) {
            font_draw_glyph(surf, clip, cur_baseline_x, cur_baseline_y, glyph, style);
        }
        
        cur_baseline_x += glyph->advance;
        text ++;
    }
    
    /* return y position after last line */
    return cur_baseline_y + style->font->line_height;
}

/* 计算文本长度（pixel为单位） */
__IPGUI_API__ ipgui_coord_t ipgui_builtin_text_width(
    const ipgui_font_t * font, 
    const s8_t * text)
{
    ipgui_coord_t width = 0;
    
    if (!font || !text) {
        return 0;
    }
    
    while (* text) {
        u8_t ch = (u8_t)*text;
        
        /* handle special characters */
        if (ch == '\n' || ch == '\r') {
            /* newlines don't affect width calculation */
            text ++;
            continue;
        }
        
        const ipgui_glyph_t * glyph = ipgui_font_get_glyph(font, ch);
        if (glyph) {
            width += glyph->advance;
        } else {
            /* use space width for unknown characters */
            width += font->space_width;
        }
        
        text ++;
    }
    
    return width;
}