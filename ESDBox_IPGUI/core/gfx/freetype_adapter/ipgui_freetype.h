/**
 * @file    ipgui_freetype.h
 * @brief   FreeType 字体渲染引擎适配层 — 公开 API
 * 
 * 本模块为 ESDBox_IPGUI 图形库提供 FreeType 字体渲染能力。
 * 与现有 ipgui_draw_builtin_font 保持 API 一致，实现无缝切换。
 * 
 * ## 功能概述
 *   - 支持 TrueType (.ttf)、OpenType (.otf) 等 FreeType 支持的字体格式
 *   - 支持从文件系统或内存缓冲区加载字体
 *   - Glyph 缓存（LRU 淘汰策略）减少重复光栅化开销
 *   - 灰度抗锯齿渲染
 *   - 兼容现有的 ipgui_font_style_t / ipgui_paint_t 体系
 * 
 * ## 使用示例
 * @code
 *   // 1. 加载字体
 *   ipgui_ft_font_t ft_font;
 *   ipgui_ft_load_font_from_file(&ft_font, "/fonts/NotoSans.ttf");
 *   ipgui_ft_set_size(&ft_font, 24);
 * 
 *   // 2. 配置绘制样式
 *   ipgui_font_style_t style;
 *   style.font    = &ft_font.base;        // 指向 base
 *   style.paint   = paint;                 // 颜色/渐变/图像
 *   style.opacity = 255;
 *   style.blend_mode = IPGUI_BLEND_NORMAL;
 * 
 *   // 3. 绘制文本
 *   ipgui_ft_draw_text(&surf, NULL, &style, "Hello 世界", 10, 20);
 * 
 *   // 4. 释放资源
 *   ipgui_ft_unload_font(&ft_font);
 * @endcode
 */

#ifndef IPGUI_FREETYPE_H
#define IPGUI_FREETYPE_H

#include "ipgui_core.h"
#include "ipgui_draw_builtin_font.h"   /* 复用 ipgui_font_t / ipgui_glyph_t / ipgui_font_style_t */
#include "ipgui_freetype_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * FreeType 字体句柄（不透明指针）
 * ================================================================ */

/** FreeType 字体内部实现的前向声明 */
struct ipgui_ft_font_internal;

/**
 * @brief FreeType 字体实例
 * 
 * 封装了 FT_Library、FT_Face、glyph 缓存等内部状态。
 * 外部代码只应通过本模块的 API 访问。
 */
typedef struct ipgui_ft_font {
    /** 
     * 兼容现有字体系统的基类结构。
     * 可直接作为 ipgui_font_style_t.font 使用，
     * 通过 ipgui_font_get_glyph() 查询 glyph。
     */
    ipgui_font_t              base;

    /** 内部实现细节（不透明指针） */
    struct ipgui_ft_font_internal * internal;
} ipgui_ft_font_t;

/* ================================================================
 * 字体样式扩展（可选）
 * ================================================================ */

/**
 * @brief FreeType 字体渲染样式扩展
 * 
 * 在现有 ipgui_font_style_t 基础上增加 FreeType 特有参数。
 * 不需要扩展参数时仍可直接使用 ipgui_font_style_t。
 */
typedef struct ipgui_ft_font_style {
    ipgui_font_style_t        base;         /**< 基础字体样式            */
    u16_t                     weight;       /**< 字重 (400=Normal, 700=Bold) */
    s8_t                      italic : 1;   /**< 是否斜体                */
    s8_t                      underline : 1;/**< 是否下划线              */
    s8_t                      strikethrough : 1; /**< 是否删除线          */
    u8_t                      reserved : 5;
} ipgui_ft_font_style_t;

/* ================================================================
 * 字体加载与卸载
 * ================================================================ */

/**
 * @brief 从文件系统加载字体
 * @param font      输出参数，字体实例
 * @param filepath  字体文件路径（支持 TTF/OTF 等）
 * @return IPGUI_ERR_OK 成功，其他值表示失败
 * 
 * @note 内部使用 VFS 接口，支持跨平台路径
 */
extern __IPGUI_API__ ipgui_err_t ipgui_ft_load_font_from_file(
    ipgui_ft_font_t    * font,
    const char         * filepath);

/**
 * @brief 从内存缓冲区加载字体
 * @param font      输出参数，字体实例
 * @param data      字体数据缓冲区（函数内部会拷贝一份）
 * @param data_size 数据大小（字节）
 * @return IPGUI_ERR_OK 成功，其他值表示失败
 * 
 * @note 适用于字体数据已预加载到 RAM 的场景（如嵌入式 ROMFS）
 */
extern __IPGUI_API__ ipgui_err_t ipgui_ft_load_font_from_memory(
    ipgui_ft_font_t    * font,
    const u8_t         * data,
    u32_t                data_size);

/**
 * @brief 卸载字体并释放所有关联资源
 * @param font 要释放的字体实例
 * 
 * 释放内容包括：FreeType face、FT_Library、glyph 缓存、字体数据副本。
 */
extern __IPGUI_API__ void ipgui_ft_unload_font(
    ipgui_ft_font_t    * font);

/* ================================================================
 * 字体参数配置
 * ================================================================ */

/**
 * @brief 设置字体渲染大小
 * @param font      字体实例
 * @param pixel_size 高度（像素）
 * 
 * 设置后 glyph 缓存将被清空并通过 FreeType 重新设置字符大小。
 */
extern __IPGUI_API__ void ipgui_ft_set_size(
    ipgui_ft_font_t    * font,
    ipgui_coord_t        pixel_size);

/**
 * @brief 获取当前字体大小
 * @param font 字体实例
 * @return 当前设置的像素高度
 */
extern __IPGUI_API__ ipgui_coord_t ipgui_ft_get_size(
    const ipgui_ft_font_t * font);

/* ================================================================
 * Glyph 查询（兼容 ipgui_font_get_glyph 接口）
 * ================================================================ */

/**
 * @brief 获取指定字符的 glyph 信息（兼容现有接口）
 * @param font      字体实例（作为 ipgui_font_t* 传入）
 * @param char_code Unicode 码点（0-0x10FFFF）
 * @return glyph 指针，失败返回 NULL
 * 
 * @note 此函数签名为兼容 ipgui_font_get_glyph 而设计。
 *       内部自动处理 FreeType 渲染和缓存。
 */
extern __IPGUI_API__ const ipgui_glyph_t * ipgui_ft_get_glyph(
    const ipgui_font_t * font,
    u32_t                char_code);

/* ================================================================
 * 文本渲染（兼容 ipgui_draw_builtin_* 接口）
 * ================================================================ */

/**
 * @brief 绘制单个字符
 * @return 字符的 advance 宽度（像素）
 */
extern __IPGUI_API__ ipgui_coord_t ipgui_ft_draw_char(
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_font_style_t * style,
    ipgui_coord_t        x,
    ipgui_coord_t        y,
    u32_t                char_code);

/**
 * @brief 绘制 UTF-8 编码文本
 * @param surf  目标绘制面
 * @param clip  裁剪区域（NULL = 不裁剪）
 * @param style 字体绘制样式
 * @param text  UTF-8 编码文本（以 '\0' 结尾）
 * @param x     起始 x 坐标（基线点）
 * @param y     起始 y 坐标（基线上方 = 顶部 + 基线偏移）
 * @return 最后一行 y 坐标 + 行高
 */
extern __IPGUI_API__ ipgui_coord_t ipgui_ft_draw_text(
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_font_style_t * style,
    const char         * text,
    ipgui_coord_t        x,
    ipgui_coord_t        y);

/**
 * @brief 计算 UTF-8 文本的渲染宽度
 * @param font 字体实例（基类指针）
 * @param text UTF-8 编码文本
 * @return 文本像素宽度
 */
extern __IPGUI_API__ ipgui_coord_t ipgui_ft_text_width(
    const ipgui_font_t * font,
    const char         * text);

/* ================================================================
 * 缓存管理
 * ================================================================ */

/**
 * @brief 清空指定字体的 glyph 缓存
 * @param font 字体实例
 * 
 * 在字体大小变更时自动调用，也可手动调用以释放内存。
 */
extern __IPGUI_API__ void ipgui_ft_cache_clear_public(
    ipgui_ft_font_t    * font);

/**
 * @brief 获取缓存统计信息
 * @param font    字体实例
 * @param count   输出：缓存字形数
 * @param pixels  输出：缓存总像素（可为 NULL）
 */
extern __IPGUI_API__ void ipgui_ft_cache_stats(
    const ipgui_ft_font_t * font,
    s32_t                 * count,
    u32_t                 * pixels);

#ifdef __cplusplus
}
#endif

#endif /* IPGUI_FREETYPE_H */
