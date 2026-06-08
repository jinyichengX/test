/**
 * @file    ipgui_freetype_render.c
 * @brief   FreeType 适配层 — 文本渲染函数
 * 
 * 实现了与 ipgui_draw_builtin_font 完全兼容的绘制 API。
 * 函数签名和调用方式与原有接口一致，可实现无缝替换。
 */

#include "ipgui_freetype.h"
#include "ipgui_freetype_internal.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

/* ================================================================
 * 单个 glyph 绘制
 * ================================================================ */

/**
 * @brief 将缓存的 glyph 位图混合到目标表面
 * 
 * 通过 ipgui_blend() 将 glyph 的灰度位图作为 mask 传入，
 * paint 决定了字体颜色（纯色/渐变/图像均可）。
 */
__IPGUI_STATIC__ void ft_draw_glyph_to_surf(
    ipgui_surf_t                * surf,
    ipgui_aabb_t                * clip,
    ipgui_coord_t                 baseline_x,
    ipgui_coord_t                 baseline_y,
    ipgui_ft_glyph_cache_item_t * glyph_item,
    ipgui_font_style_t          * style)
{
    ipgui_aabb_t glyph_mask, draw;

    if (!glyph_item || !glyph_item->bitmap ||
        glyph_item->width == 0 || glyph_item->height == 0)
        return;

    /* 裁剪检测 */
    if (clip) {
        if (ipgui_aabb_overlap(&draw, clip, &surf->surf) != 0) return;
    } else {
        draw = surf->surf;
    }

    /* 计算 glyph 覆盖区域（基于基线） */
    glyph_mask.start.x = baseline_x + glyph_item->bearing_x;
    glyph_mask.start.y = baseline_y - glyph_item->bearing_y;
    glyph_mask.end.x   = glyph_mask.start.x + glyph_item->width  - 1;
    glyph_mask.end.y   = glyph_mask.start.y + glyph_item->height - 1;

    /* 裁剪交叉检测 */
    if (ipgui_aabb_overlap(&draw, &draw, &glyph_mask) != 0) return;

    /* blend：位图作为 mask，颜色由 style->paint 控制 */
    ipgui_blend(
        surf,
        (ipgui_aabb_t *)0,   /* 已做裁剪，不再传 clip */
        &draw,
        &style->paint,
        style->opacity,
        glyph_item->bitmap,  /* glyph 灰度位图 = mask */
        &glyph_mask,
        style->blend_mode);
}

/* ================================================================
 * 绘制单个字符
 * ================================================================ */

__IPGUI_API__ ipgui_coord_t ipgui_ft_draw_char(
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_font_style_t * style,
    ipgui_coord_t        x,
    ipgui_coord_t        y,
    u32_t                char_code)
{
    ipgui_ft_font_t * ft_font;
    ipgui_ft_glyph_cache_item_t * glyph_item;

    if (!style || !style->font || style->opacity < 3) return 0;
    if (is_ascii_space(char_code)) {
        /* 空格等空白字符只推进位置 */
        return style->font->space_width;
    }

    ft_font = (ipgui_ft_font_t *)((u8_t *)style->font -
        (u32_t)(uintptr_t)&(((ipgui_ft_font_t *)0)->base));
    if (!ft_font->internal) return 0;

    glyph_item = ipgui_ft_cache_get_glyph(ft_font, char_code);
    if (!glyph_item) {
        /* 渲染失败：跳过，推进最小距离 */
        FT_DBG_INFO("glyph not found for U+%04X", char_code);
        return style->font->space_width;
    }

    /* 绘制 */
    ft_draw_glyph_to_surf(
        surf, clip,
        x,
        y + style->font->baseline,
        glyph_item, style);

    return glyph_item->advance_x;
}

/* ================================================================
 * 绘制 UTF-8 文本
 * ================================================================ */

__IPGUI_API__ ipgui_coord_t ipgui_ft_draw_text(
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_font_style_t * style,
    const char         * text,
    ipgui_coord_t        x,
    ipgui_coord_t        y)
{
    ipgui_ft_font_t * ft_font;
    ipgui_ft_glyph_cache_item_t * glyph_item;
    ipgui_coord_t cur_x, cur_y;
    u32_t char_code;

    if (!surf || !style || !style->font || !text || style->opacity < 3)
        return y;

    ft_font = (ipgui_ft_font_t *)((u8_t *)style->font -
        (u32_t)(uintptr_t)&(((ipgui_ft_font_t *)0)->base));
    if (!ft_font->internal) return y;

    cur_x = x;
    cur_y = y + style->font->baseline;

    while (*text) {
        const u8_t * p = (const u8_t *)text;
        char_code = utf8_decode(&p);
        text = (const char *)p;

        /* 换行处理 */
        if (char_code == '\n') {
            cur_x = x;
            cur_y += style->font->line_height + style->line_spacing;
            continue;
        }

        /* 回车跳过 */
        if (char_code == '\r') continue;

        /* 空格：推进 space_width */
        if (char_code == ' ' || char_code == '\t') {
            cur_x += style->font->space_width;
            continue;
        }

        /* 获取 glyph */
        glyph_item = ipgui_ft_cache_get_glyph(ft_font, char_code);
        if (!glyph_item) {
            FT_DBG_INFO("glyph not found for U+%04X", char_code);
            cur_x += style->font->space_width;
            continue;
        }

        /* 绘制 */
        ft_draw_glyph_to_surf(surf, clip, cur_x, cur_y, glyph_item, style);

        /* 推进 x 坐标 */
        cur_x += glyph_item->advance_x;
    }

    /* 返回最后一行底部坐标 */
    return cur_y + style->font->line_height;
}

/* ================================================================
 * 文本宽度计算
 * ================================================================ */

__IPGUI_API__ ipgui_coord_t ipgui_ft_text_width(
    const ipgui_font_t * font,
    const char         * text)
{
    ipgui_ft_font_t * ft_font;
    ipgui_ft_glyph_cache_item_t * glyph_item;
    ipgui_coord_t width = 0;
    u32_t char_code;

    if (!font || !text) return 0;

    ft_font = (ipgui_ft_font_t *)((u8_t *)font -
        (u32_t)(uintptr_t)&(((ipgui_ft_font_t *)0)->base));
    if (!ft_font->internal) return 0;

    while (*text) {
        const u8_t * p = (const u8_t *)text;
        char_code = utf8_decode(&p);
        text = (const char *)p;

        /* 换行/回车不影响 width 计算 */
        if (char_code == '\n' || char_code == '\r') continue;

        /* 空格直接加 space_width */
        if (char_code == ' ' || char_code == '\t') {
            width += font->space_width;
            continue;
        }

        glyph_item = ipgui_ft_cache_get_glyph(ft_font, char_code);
        if (glyph_item) {
            width += glyph_item->advance_x;
        } else {
            width += font->space_width;
        }
    }

    return width;
}
