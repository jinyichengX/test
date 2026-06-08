/**
 * @file    ipgui_freetype_conf.h
 * @brief   FreeType 适配层配置项
 * 
 * 本文件集中管理 FreeType 适配器的所有可配置参数。
 * 通过修改宏定义即可调整缓存策略、内存限制等行为。
 */

#ifndef IPGUI_FREETYPE_CONF_H
#define IPGUI_FREETYPE_CONF_H

#include "ipgui_conf.h"

/* ================================================================
 * FreeType 引擎开关
 * ================================================================ */
#ifndef IPGUI_USE_FREETYPE
#define IPGUI_USE_FREETYPE          1   /**< 1=启用 FreeType 适配, 0=禁用 */
#endif

/* ================================================================
 * Glyph 缓存配置
 * ================================================================ */

/** glyph 缓存最大容量（字形数量），超过后按 LRU 淘汰 */
#ifndef IPGUI_FT_GLYPH_CACHE_MAX
#define IPGUI_FT_GLYPH_CACHE_MAX    256
#endif

/** 单个 glyph 位图最大尺寸（像素），超过此阈值不缓存 */
#ifndef IPGUI_FT_GLYPH_CACHE_SIZE_LIMIT
#define IPGUI_FT_GLYPH_CACHE_SIZE_LIMIT  4096
#endif

/** 允许缓存的最大 glyph 总像素面积 */
#ifndef IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS
#define IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS  524288  /* 512KB worth of 1-byte pixels */
#endif

/* ================================================================
 * 渲染配置
 * ================================================================ */

/** 默认字体大小（像素） */
#ifndef IPGUI_FT_DEFAULT_FONT_SIZE
#define IPGUI_FT_DEFAULT_FONT_SIZE  16
#endif

/** 字形渲染模式: 0=单色, 1=灰度抗锯齿(LCD未实现) */
#ifndef IPGUI_FT_RENDER_MODE
#define IPGUI_FT_RENDER_MODE        1   /**< 灰度抗锯齿 */
#endif

/** 最大支持的字体文件大小（字节） */
#ifndef IPGUI_FT_FONT_FILE_MAX_SIZE
#define IPGUI_FT_FONT_FILE_MAX_SIZE  (8 * 1024 * 1024)  /* 8MB */
#endif

/* ================================================================
 * 调试配置
 * ================================================================ */

/** 是否启用调试日志 */
#ifndef IPGUI_FT_DEBUG
#define IPGUI_FT_DEBUG              0
#endif

#if IPGUI_FT_DEBUG
#include "ipgui_debug.h"
#define FT_DBG_INFO(fmt, ...)   ipgui_dbg_info("[ft] " fmt, ##__VA_ARGS__)
#define FT_DBG_ERROR(fmt, ...)  ipgui_dbg_error("[ft] " fmt, ##__VA_ARGS__)
#else
#define FT_DBG_INFO(fmt, ...)
#define FT_DBG_ERROR(fmt, ...)
#endif

#endif /* IPGUI_FREETYPE_CONF_H */
