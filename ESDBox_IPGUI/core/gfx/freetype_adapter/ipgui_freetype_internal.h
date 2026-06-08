/**
 * @file    ipgui_freetype_internal.h
 * @brief   FreeType 适配层内部数据结构（不对外暴露）
 */

#ifndef IPGUI_FREETYPE_INTERNAL_H
#define IPGUI_FREETYPE_INTERNAL_H

#include "ipgui_freetype.h"
#include "ipgui_freetype_conf.h"
#include "ipgui_list.h"

/* ================================================================
 * 前向声明 FreeType 类型（避免对用户暴露 FT 头文件）
 * ================================================================ */

/* FT_Library 句柄 */
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  * FT_Library;

/* FT_Face 句柄 */
struct FT_FaceRec_;
typedef struct FT_FaceRec_     * FT_Face;

/* FT_GlyphSlot */
struct FT_GlyphSlotRec_;
typedef struct FT_GlyphSlotRec_  FT_GlyphSlot;

/* ================================================================
 * Glyph 缓存项
 * ================================================================ */

typedef struct ipgui_ft_glyph_cache_item {
    struct list_head    node;           /**< LRU 链表节点             */
    u32_t               char_code;      /**< Unicode 码点             */
    u8_t              * bitmap;         /**< 灰度位图数据 (0-255)     */
    ipgui_coord_t       width;          /**< 位图宽度                 */
    ipgui_coord_t       height;         /**< 位图高度                 */
    ipgui_coord_t       bearing_x;      /**< 左方位偏移               */
    ipgui_coord_t       bearing_y;      /**< 上方位偏移               */
    ipgui_coord_t       advance_x;      /**< 水平步进宽度             */
    ipgui_coord_t       advance_y;      /**< 垂直步进高度             */
    u32_t               last_used;      /**< 最后使用时间戳           */
#if IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS > 0
    u32_t               pixel_count;    /**< 位图像素数量             */
#endif
} ipgui_ft_glyph_cache_item_t;

/* ================================================================
 * FreeType 字体实例（内部实现）
 * ================================================================ */

struct ipgui_ft_font_internal {
    FT_Library          library;        /**< FreeType 库句柄          */
    FT_Face             face;           /**< FreeType 字体面           */
    u8_t              * font_data;      /**< 字体文件数据（内存加载时有效） */
    u32_t               font_data_size; /**< 字体数据大小             */
    s32_t               load_from_mem;  /**< 1=从内存加载, 0=从文件加载 */
    
    ipgui_coord_t       font_size;      /**< 当前字体大小(像素)       */
    ipgui_coord_t       line_height;    /**< 行高（像素）             */
    ipgui_coord_t       baseline;       /**< 基线偏移（顶部到基线）   */
    ipgui_coord_t       space_width;    /**< 空格宽度                 */
    
    /* glyph 缓存系统 */
    struct list_head    glyph_cache;    /**< 缓存链表头               */
    s32_t               glyph_count;    /**< 当前缓存字形数           */
    u32_t               tick_counter;   /**< LRU 时间戳计数器         */
#if IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS > 0
    u32_t               total_pixels;   /**< 缓存总像素面积           */
#endif
    
    /* 渲染上下文快照（减少逐帧重建） */
    s32_t               need_reload;    /**< 字体大小变更标志         */
};

/* ================================================================
 * FreeType 适配器内部函数
 * ================================================================ */

/** 从缓存获取或从 FreeType 渲染一个 glyph */
ipgui_ft_glyph_cache_item_t * ipgui_ft_cache_get_glyph(
    ipgui_ft_font_t    * font,
    u32_t                char_code);

/** 将 glyph 插入缓存（触发 LRU 淘汰） */
void ipgui_ft_cache_put_glyph(
    ipgui_ft_font_t    * font,
    ipgui_ft_glyph_cache_item_t * item);

/** 清空指定字体的所有 glyph 缓存 */
void ipgui_ft_cache_clear(ipgui_ft_font_t * font);

/** 从 FreeType FT_Face 渲染单个 glyph 到 ipgui_ft_glyph_cache_item_t */
ipgui_ft_glyph_cache_item_t * ipgui_ft_render_glyph_internal(
    ipgui_ft_font_t    * font,
    u32_t                char_code);

/** 释放单个 glyph 缓存项的内存 */
void ipgui_ft_cache_item_free(ipgui_ft_glyph_cache_item_t * item);

#endif /* IPGUI_FREETYPE_INTERNAL_H */
