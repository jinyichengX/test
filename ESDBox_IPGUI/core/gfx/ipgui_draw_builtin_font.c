#include "ipgui_draw_builtin_font.h"
#include "ipgui_debug.h"
#include "ipgui_blur.h"
#include "ipgui_mask_buf.h"
#include "ipgui_memory.h"

/* 限制最大模糊半径，避免单字临时 mask 过大 */
#ifndef IPGUI_TEXT_SHADOW_BLUR_MAX
#define IPGUI_TEXT_SHADOW_BLUR_MAX 16
#endif

__IPGUI_STATIC__ u8_t text_shadow_passes(ipgui_text_shadow_quality_t q)
{
    switch (q) {
    case IPGUI_TEXT_SHADOW_QUALITY_HIGH:   return 3;
    case IPGUI_TEXT_SHADOW_QUALITY_MEDIUM: return 2;
    case IPGUI_TEXT_SHADOW_QUALITY_LOW:
    default:                               return 1;
    }
}

__IPGUI_STATIC__ ipgui_coord_t text_shadow_clamp_blur(ipgui_coord_t blur)
{
    if (blur < 0) return 0;
    if (blur > IPGUI_TEXT_SHADOW_BLUR_MAX) return IPGUI_TEXT_SHADOW_BLUR_MAX;
    return blur;
}

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
        (u8_t *)mask,
        &glyph_mask,
        style->blend_mode);
}

/* 硬阴影：仅偏移复制，无模糊 */
__IPGUI_STATIC__ void font_draw_glyph_hard_shadow(
    ipgui_surf_t              * surf,
    ipgui_aabb_t              * clip,
    ipgui_coord_t               baseline_x,
    ipgui_coord_t               baseline_y,
    const ipgui_glyph_t       * glyph,
    ipgui_text_shadow_style_t * shadow)
{
    ipgui_font_style_t ss;

    if (!shadow || shadow->opacity < 3) return;

    ss.font         = (const ipgui_font_t *)0; /* unused by font_draw_glyph */
    ss.paint.type   = IPGUI_PAINT_COLOR;
    ss.paint.src.color = shadow->color;
    ss.opacity      = shadow->opacity;
    ss.line_spacing = 0;
    ss.blend_mode   = shadow->blend_mode;

    font_draw_glyph(
        surf,
        clip,
        baseline_x + shadow->offset_x,
        baseline_y + shadow->offset_y,
        glyph,
        &ss);
}

/*
 * 柔和阴影：
 *   1) 申请 glyph + pad 临时 A8 mask
 *   2) 拷贝 cover_map 到中心
 *   3) 居中对称 blur（质量档位决定遍数）
 *   4) 偏移后 blend 阴影色
 *
 * 内存失败 / 临时行缓冲失败时，降级为硬阴影，保证稳定可见。
 */
__IPGUI_STATIC__ void font_draw_glyph_soft_shadow(
    ipgui_surf_t              * surf,
    ipgui_aabb_t              * clip,
    ipgui_coord_t               baseline_x,
    ipgui_coord_t               baseline_y,
    const ipgui_glyph_t       * glyph,
    ipgui_text_shadow_style_t * shadow)
{
    ipgui_coord_t blur;
    u8_t passes;
    ipgui_coord_t pad;
    ipgui_coord_t mw, mh;
    ipgui_coord_t got_h;
    u8_t * mask;
    ipgui_aabb_t shadow_box, draw, view;
    ipgui_mask_surface_t ms;
    ipgui_paint_t paint;
    ipgui_coord_t y;

    if (!shadow || shadow->opacity < 3) return;

    blur   = text_shadow_clamp_blur(shadow->blur);
    passes = text_shadow_passes(shadow->quality);
    /* 多遍 box blur 的有效扩散约 passes * blur */
    pad = blur * (ipgui_coord_t)passes;
    if (pad < blur) {
        /* 溢出时退回硬阴影，避免错误尺寸 */
        font_draw_glyph_hard_shadow(surf, clip, baseline_x, baseline_y, glyph, shadow);
        return;
    }

    mw = glyph->width  + pad * 2;
    mh = glyph->height + pad * 2;
    /* 尺寸溢出/非法：降级硬阴影 */
    if (mw <= glyph->width || mh <= glyph->height) {
        font_draw_glyph_hard_shadow(surf, clip, baseline_x, baseline_y, glyph, shadow);
        return;
    }

    /* 阴影在屏幕上的包围盒（含 blur pad + offset） */
    shadow_box.start.x = baseline_x + glyph->bearing_x - pad + shadow->offset_x;
    shadow_box.start.y = baseline_y - glyph->bearing_y - pad + shadow->offset_y;
    shadow_box.end.x   = shadow_box.start.x + mw - 1;
    shadow_box.end.y   = shadow_box.start.y + mh - 1;

    if (clip) {
        if (ipgui_aabb_overlap(&view, clip, &surf->surf) != 0) return;
    } else {
        view = surf->surf;
    }
    if (ipgui_aabb_overlap(&draw, &view, &shadow_box) != 0) {
        return; /* 完全在裁剪外，跳过 */
    }

    mask = ipgui_mask_buf_acquire(mw, mh, &got_h);
    if (!mask || got_h != mh) {
        if (mask) ipgui_mask_buf_free(mask);
        /* 内存不足：降级硬阴影，保证效果可见 */
        font_draw_glyph_hard_shadow(surf, clip, baseline_x, baseline_y, glyph, shadow);
        return;
    }

    ipgui_memset(mask, 0, (u32_t)mw * (u32_t)mh);

    /* 将 glyph alpha 拷到 pad 后的中心 */
    for (y = 0; y < glyph->height; y ++) {
        ipgui_memcpy(
            mask + (uintptr_t)(y + pad) * mw + pad,
            glyph->cover_map + (uintptr_t)y * glyph->width,
            (u32_t)glyph->width);
    }

    ms.mask = mask;
    ms.w    = mw;
    ms.h    = mh;

    if (blur > 0) {
        ipgui_coord_t kn = blur * 2 + 1; /* 居中对称奇数核 */
        if (ipgui_blur(&ms, kn, kn, passes, &ms) != 0) {
            /* 行/列临时缓冲申请失败：释放 mask 后降级硬阴影 */
            ipgui_mask_buf_free(mask);
            font_draw_glyph_hard_shadow(
                surf, clip, baseline_x, baseline_y, glyph, shadow);
            return;
        }
    }

    paint.type = IPGUI_PAINT_COLOR;
    paint.src.color = shadow->color;

    ipgui_blend(
        surf,
        (ipgui_aabb_t *)0,
        &draw,
        &paint,
        shadow->opacity,
        mask,
        &shadow_box,
        shadow->blend_mode);

    ipgui_mask_buf_free(mask);
}

__IPGUI_STATIC__ void font_draw_glyph_with_shadow(
    ipgui_surf_t              * surf,
    ipgui_aabb_t              * clip,
    ipgui_coord_t               baseline_x,
    ipgui_coord_t               baseline_y,
    const ipgui_glyph_t       * glyph,
    ipgui_font_style_t        * style,
    ipgui_text_shadow_style_t * shadow)
{
    if (!glyph || !glyph->cover_map ||
        glyph->width == 0 || glyph->height == 0) {
        return;
    }

    if (shadow && shadow->opacity >= 3) {
        if (text_shadow_clamp_blur(shadow->blur) > 0) {
            font_draw_glyph_soft_shadow(
                surf, clip, baseline_x, baseline_y, glyph, shadow);
        } else {
            font_draw_glyph_hard_shadow(
                surf, clip, baseline_x, baseline_y, glyph, shadow);
        }
    }

    if (style) {
        font_draw_glyph(surf, clip, baseline_x, baseline_y, glyph, style);
    }
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

__IPGUI_API__ ipgui_coord_t ipgui_draw_builtin_char_shadowed(
    ipgui_surf_t              * surf,
    ipgui_aabb_t              * clip,
    ipgui_font_style_t        * style,
    ipgui_text_shadow_style_t * shadow,
    ipgui_coord_t               x,
    ipgui_coord_t               y,
    u8_t                        ch)
{
    if (!style || !style->font) {
        return 0;
    }

    if (ch == ' ') {
        return style->font->space_width;
    }
    if (ch == '\n' || ch == '\r') {
        return 0;
    }

    const ipgui_glyph_t * glyph = ipgui_font_get_glyph(style->font, ch);
    if (!glyph || !glyph->cover_map || glyph->width == 0) {
        glyph = ipgui_font_get_glyph(style->font, '?');
        if (!glyph) {
            return style->font->space_width;
        }
    }

    font_draw_glyph_with_shadow(
        surf,
        clip,
        x,
        y + style->font->baseline,
        glyph,
        (style->opacity >= 3) ? style : (ipgui_font_style_t *)0,
        shadow);

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

__IPGUI_API__ ipgui_coord_t ipgui_draw_builtin_text_shadowed(
    ipgui_surf_t              * surf,
    ipgui_aabb_t              * clip,
    ipgui_font_style_t        * style,
    ipgui_text_shadow_style_t * shadow,
    const s8_t                * text,
    ipgui_coord_t               x,
    ipgui_coord_t               y)
{
    ipgui_coord_t start_x = x;
    ipgui_coord_t cur_baseline_x = x;
    ipgui_coord_t cur_baseline_y;
    ipgui_font_style_t * body_style;

    if (!surf || !style || !style->font || !text) {
        return y;
    }

    cur_baseline_y = y + style->font->baseline;
    body_style = (style->opacity >= 3) ? style : (ipgui_font_style_t *)0;

    /* 无阴影且正文也不可见：直接返回 */
    if (!body_style && (!shadow || shadow->opacity < 3)) {
        return y;
    }

    while (* text) {
        u8_t ch = (u8_t)(* text);

        if (ch == '\n') {
            cur_baseline_x = start_x;
            cur_baseline_y += style->font->line_height + style->line_spacing;
            text ++;
            continue;
        }

        if (ch == ' ') {
            cur_baseline_x += style->font->space_width;
            text ++;
            continue;
        }

        if (ch == '\r') {
            text ++;
            continue;
        }

        const ipgui_glyph_t * glyph = ipgui_font_get_glyph(style->font, ch);
        if (!glyph) {
            glyph = ipgui_font_get_glyph(style->font, '?');
            if (!glyph) {
                text ++;
                continue;
            }
        }

        if (glyph->width > 0 && glyph->height > 0) {
            font_draw_glyph_with_shadow(
                surf, clip,
                cur_baseline_x, cur_baseline_y,
                glyph, body_style, shadow);
        }

        cur_baseline_x += glyph->advance;
        text ++;
    }

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
