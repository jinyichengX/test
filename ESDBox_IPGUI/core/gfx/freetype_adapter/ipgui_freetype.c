/**
 * @file    ipgui_freetype.c
 * @brief   FreeType 适配层 — 字体加载、glyph 缓存、核心渲染
 * 
 * 架构设计:
 *   FT_Library (全局共享) → FT_Face (每字体) → FT_GlyphSlot → ipgui_glyph_t
 * 
 * 缓存策略:
 *   采用 LRU 双向链表管理 glyph 缓存项。
 *   命中 → 移到链表头部，更新 last_used 时间戳。
 *   未命中 → 调用 FreeType 光栅化，创建新缓存项插入头部。
 *   超量 → 从链表尾部淘汰（last_used 最旧 + refcnt=0）。
 * 
 * 内存管理:
 *   glyph 位图内存由缓存项独立管理（ipgui_mem_alloc_def / ipgui_mem_free_def），
 *   字体卸载时遍历链表统一释放。
 */

#include "ipgui_freetype.h"
#include "ipgui_freetype_internal.h"
#include "ipgui_memory.h"
#include "ipgui_vfs.h"
#include "ipgui_debug.h"

/* ================================================================
 * FreeType 头文件 — 条件包含
 * 实际集成时取消注释下方的 include，并链接 libfreetype
 * ================================================================ */
#if IPGUI_USE_FREETYPE
/* #include <ft2build.h> */
/* #include FT_FREETYPE_H */
/* #include FT_GLYPH_H */

/* 
 * 以下为 FreeType 关键 API 的适配桩代码注释，
 * 实际链接 FreeType 库后替换为真实调用。
 */

/* FT_Error FT_Init_FreeType(FT_Library *alibrary); */
/* FT_Error FT_Done_FreeType(FT_Library library); */
/* FT_Error FT_New_Face(FT_Library library, const char *path, FT_Long idx, FT_Face *face); */
/* FT_Error FT_New_Memory_Face(FT_Library library, const FT_Byte *base, FT_Long size, FT_Long idx, FT_Face *face); */
/* FT_Error FT_Done_Face(FT_Face face); */
/* FT_Error FT_Set_Pixel_Sizes(FT_Face face, FT_UInt w, FT_UInt h); */
/* FT_Error FT_Load_Char(FT_Face face, FT_ULong charcode, FT_Int32 flags); */
/* FT_Error FT_Render_Glyph(FT_GlyphSlot slot, FT_Render_Mode mode); */

#define FT_LOAD_DEFAULT               0x0
#define FT_LOAD_RENDER                0x4
#define FT_RENDER_MODE_NORMAL         0x0
#define FT_RENDER_MODE_MONO           0x1

#endif /* IPGUI_USE_FREETYPE */

/* ================================================================
 * 全局 FreeType 库句柄（单例）
 * ================================================================ */
#if IPGUI_USE_FREETYPE
static FT_Library g_ft_library = NULL;
static s32_t      g_ft_ref_count = 0;
#endif

/* ================================================================
 * UTF-8 解码器
 * ================================================================ */

/**
 * @brief 解码 UTF-8 序列，返回 Unicode 码点并推进指针
 * @param p 指向 UTF-8 字节指针的指针（函数会推进 *p）
 * @return Unicode 码点；无效序列返回 0
 */
__IPGUI_STATIC__ u32_t utf8_decode(const u8_t ** p)
{
    u32_t c;
    const u8_t * s = *p;

    if ((s[0] & 0x80) == 0) {
        /* 1-byte: 0xxxxxxx */
        c = s[0];
        *p += 1;
    } else if ((s[0] & 0xE0) == 0xC0) {
        /* 2-byte: 110xxxxx 10xxxxxx */
        c  = (u32_t)(s[0] & 0x1F) << 6;
        c |= (u32_t)(s[1] & 0x3F);
        *p += 2;
    } else if ((s[0] & 0xF0) == 0xE0) {
        /* 3-byte: 1110xxxx 10xxxxxx 10xxxxxx */
        c  = (u32_t)(s[0] & 0x0F) << 12;
        c |= (u32_t)(s[1] & 0x3F) << 6;
        c |= (u32_t)(s[2] & 0x3F);
        *p += 3;
    } else if ((s[0] & 0xF8) == 0xF0) {
        /* 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        c  = (u32_t)(s[0] & 0x07) << 18;
        c |= (u32_t)(s[1] & 0x3F) << 12;
        c |= (u32_t)(s[2] & 0x3F) << 6;
        c |= (u32_t)(s[3] & 0x3F);
        *p += 4;
    } else {
        /* invalid */
        c = 0;
        *p += 1;
    }
    return c;
}

/**
 * @brief ASCII 空格检查（0-127 范围内的空白符）
 */
__IPGUI_STATIC__ __IPGUI_INLINE__ s32_t is_ascii_space(u32_t ch)
{
    return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
}

/* ================================================================
 * 全局 FreeType 库初始化 / 销毁
 * ================================================================ */

#if IPGUI_USE_FREETYPE
__IPGUI_STATIC__ ipgui_err_t ft_library_acquire(FT_Library * lib)
{
    if (g_ft_library) {
        g_ft_ref_count++;
        *lib = g_ft_library;
        return IPGUI_ERR_OK;
    }

    /* FT_Error err = FT_Init_FreeType(&g_ft_library); */
    /* if (err) { */
    /*     FT_DBG_ERROR("FT_Init_FreeType failed: %d", err); */
    /*     return IPGUI_ERR_NOK; */
    /* } */
    /* g_ft_ref_count = 1; */
    /* *lib = g_ft_library; */
    /* return IPGUI_ERR_OK; */

    /* 桩代码 — 实际集成时替换为上面注释的逻辑 */
    (void)lib;
    return IPGUI_ERR_NOK;
}

__IPGUI_STATIC__ void ft_library_release(void)
{
    g_ft_ref_count--;
    if (g_ft_ref_count <= 0 && g_ft_library) {
        /* FT_Done_FreeType(g_ft_library); */
        g_ft_library = NULL;
        g_ft_ref_count = 0;
    }
}
#endif

/* ================================================================
 * Glyph 缓存实现
 * ================================================================ */

__IPGUI_API__ void ipgui_ft_cache_item_free(ipgui_ft_glyph_cache_item_t * item)
{
    if (item) {
        if (item->bitmap) {
            ipgui_mem_free_def(item->bitmap);
        }
        ipgui_mem_free_def(item);
    }
}

__IPGUI_API__ void ipgui_ft_cache_clear(ipgui_ft_font_t * font)
{
    struct ipgui_ft_font_internal * fi;
    struct list_head * pos, * n;
    ipgui_ft_glyph_cache_item_t * item;

    if (!font || !font->internal) return;

    fi = font->internal;
    list_for_each_safe(pos, n, &fi->glyph_cache) {
        item = list_entry(pos, ipgui_ft_glyph_cache_item_t, node);
        list_del(&item->node);
        ipgui_ft_cache_item_free(item);
    }
    fi->glyph_count = 0;
#if IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS > 0
    fi->total_pixels = 0;
#endif
    FT_DBG_INFO("glyph cache cleared for font@%p", (void *)font);
}

/**
 * @brief LRU 淘汰：从链表尾部移除最旧的缓存项，直到 ≤ 限制
 */
__IPGUI_STATIC__ void ft_cache_evict(struct ipgui_ft_font_internal * fi)
{
    ipgui_ft_glyph_cache_item_t * item;
    struct list_head * pos;

    while (fi->glyph_count > IPGUI_FT_GLYPH_CACHE_MAX) {
        if (list_empty(&fi->glyph_cache)) break;

        /* 从尾部取最旧的 */
        pos = fi->glyph_cache.prev;
        item = list_entry(pos, ipgui_ft_glyph_cache_item_t, node);
        list_del(&item->node);
#if IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS > 0
        fi->total_pixels -= item->pixel_count;
#endif
        ipgui_ft_cache_item_free(item);
        fi->glyph_count--;
    }

#if IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS > 0
    /* 总像素也做限制 */
    while (fi->total_pixels > IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS) {
        if (list_empty(&fi->glyph_cache)) break;
        pos = fi->glyph_cache.prev;
        item = list_entry(pos, ipgui_ft_glyph_cache_item_t, node);
        list_del(&item->node);
        fi->total_pixels -= item->pixel_count;
        ipgui_ft_cache_item_free(item);
        fi->glyph_count--;
    }
#endif
}

__IPGUI_API__ ipgui_ft_glyph_cache_item_t *
ipgui_ft_cache_get_glyph(ipgui_ft_font_t * font, u32_t char_code)
{
    struct ipgui_ft_font_internal * fi;
    struct list_head * pos;
    ipgui_ft_glyph_cache_item_t * item;

    if (!font || !font->internal) return NULL;

    fi = font->internal;

    /* 1. 查找缓存命中 */
    list_for_each(pos, &fi->glyph_cache) {
        item = list_entry(pos, ipgui_ft_glyph_cache_item_t, node);
        if (item->char_code == char_code) {
            /* 命中 → 移到头部（LRU 最近） */
            list_del(&item->node);
            list_add(&item->node, &fi->glyph_cache);
            item->last_used = ++fi->tick_counter;
            return item;
        }
    }

    /* 2. 缺页 → FreeType 渲染 */
    item = ipgui_ft_render_glyph_internal(font, char_code);
    if (!item) return NULL;

    /* 3. 插入缓存 */
    item->last_used = ++fi->tick_counter;
    list_add(&item->node, &fi->glyph_cache);  /* 插入头部 */
    fi->glyph_count++;
#if IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS > 0
    fi->total_pixels += item->pixel_count;
#endif

    /* 4. 触发淘汰 */
    ft_cache_evict(fi);

    return item;
}

__IPGUI_API__ void ipgui_ft_cache_put_glyph(
    ipgui_ft_font_t * font, ipgui_ft_glyph_cache_item_t * item)
{
    struct ipgui_ft_font_internal * fi;
    if (!font || !font->internal || !item) return;

    fi = font->internal;
    list_add(&item->node, &fi->glyph_cache);
    fi->glyph_count++;
#if IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS > 0
    fi->total_pixels += item->pixel_count;
#endif
    ft_cache_evict(fi);
}

/* ================================================================
 * FreeType Glyph 渲染（核心路径）
 * ================================================================ */

__IPGUI_API__ ipgui_ft_glyph_cache_item_t *
ipgui_ft_render_glyph_internal(ipgui_ft_font_t * font, u32_t char_code)
{
    struct ipgui_ft_font_internal * fi;
    ipgui_ft_glyph_cache_item_t * item;

    if (!font || !font->internal) return NULL;
    fi = font->internal;

#if IPGUI_USE_FREETYPE
    /* ================================================================
     * 以下为生产代码 — 直接调用 FreeType C API。
     * 当前以注释形式保留，链接 FreeType 库后取消注释即生效。
     * ================================================================ */

    /*
    FT_Face face = fi->face;
    FT_GlyphSlot slot;
    FT_Error fterr;

    // 1. 加载字符到 glyph slot
    fterr = FT_Load_Char(face, (FT_ULong)char_code, FT_LOAD_RENDER);
    if (fterr) {
        FT_DBG_INFO("FT_Load_Char(0x%X) failed: %d", char_code, fterr);
        return NULL;
    }

    slot = face->glyph;

    // 2. 检查渲染模式：确保是灰度抗锯齿
    if (slot->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
        // 如果不是灰度模式，重新以灰度模式渲染
        fterr = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
        if (fterr) {
            FT_DBG_INFO("FT_Render_Glyph failed: %d", fterr);
            return NULL;
        }
    }

    // 3. 分配缓存项
    item = (ipgui_ft_glyph_cache_item_t *)
        ipgui_mem_alloc_def(sizeof(ipgui_ft_glyph_cache_item_t));
    if (!item) return NULL;
    ipgui_memset(item, 0, sizeof(*item));

    item->char_code = char_code;
    item->width     = (ipgui_coord_t)slot->bitmap.width;
    item->height    = (ipgui_coord_t)slot->bitmap.rows;
    item->bearing_x = (ipgui_coord_t)slot->bitmap_left;
    item->bearing_y = (ipgui_coord_t)slot->bitmap_top;
    item->advance_x = (ipgui_coord_t)(slot->advance.x >> 6);    // 26.6 → 整数
    item->advance_y = (ipgui_coord_t)(slot->advance.y >> 6);

    // 4. 拷贝位图数据（FreeType 位图生命周期在下次 FT_Load_Char 时失效）
    if (item->width > 0 && item->height > 0 &&
        item->width * item->height <= IPGUI_FT_GLYPH_CACHE_SIZE_LIMIT)
    {
        u32_t pixel_count = (u32_t)item->width * item->height;
        item->bitmap = (u8_t *)ipgui_mem_alloc_def(pixel_count);
        if (item->bitmap) {
            if (slot->bitmap.pitch == item->width) {
                // 紧凑位图：直接 memcpy
                ipgui_memcpy(item->bitmap, slot->bitmap.buffer, pixel_count);
            } else {
                // 带 padding 的位图：逐行拷贝
                u8_t * dst = item->bitmap;
                u8_t * src = slot->bitmap.buffer;
                ipgui_coord_t row;
                for (row = 0; row < item->height; row++) {
                    ipgui_memcpy(dst, src, item->width);
                    dst += item->width;
                    src += slot->bitmap.pitch;
                }
            }
        }
#if IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS > 0
        item->pixel_count = pixel_count;
#endif
    }

    return item;
    */

    /* 桩代码：实际集成 FreeType 后删除下面这行 */
    (void)char_code;
    item = NULL;
    return item;

#else
    /* FreeType 未启用时返回 NULL */
    (void)fi;
    (void)char_code;
    return NULL;
#endif
}

/* ================================================================
 * 字体加载
 * ================================================================ */

__IPGUI_API__ ipgui_err_t ipgui_ft_load_font_from_file(
    ipgui_ft_font_t * font, const char * filepath)
{
    struct ipgui_ft_font_internal * fi;
    ipgui_file_t file;
    ipgui_err_t err;
    u8_t * buf = NULL;
    u32_t file_size;
    u32_t br;

    if (!font || !filepath) return IPGUI_ERR_PARAM;

    ipgui_memset(font, 0, sizeof(*font));

    /* 1. 打开文件 */
    err = ipgui_file_init(&file);
    if (err != IPGUI_ERR_OK) return err;
    err = ipgui_vfs_fopen(&file, filepath, IPGUI_FILE_MODE_READ, 0, 0);
    if (err != IPGUI_ERR_OK) {
        FT_DBG_ERROR("failed to open font file: %s", filepath);
        return IPGUI_ERR_FS_FOPEN;
    }

    /* 2. 读取整个文件到内存 */
    file_size = file.fs->fsize(file.fs, file.pri_file);
    if (file_size == 0 || file_size > IPGUI_FT_FONT_FILE_MAX_SIZE) {
        file.fs->fclose(file.fs, file.pri_file);
        return IPGUI_ERR_OVERFLOW;
    }

    buf = (u8_t *)ipgui_mem_alloc_def(file_size);
    if (!buf) {
        file.fs->fclose(file.fs, file.pri_file);
        return IPGUI_ERR_NOMEM;
    }

    err = ipgui_vfs_fread(&file, buf, file_size, &br);
    file.fs->fclose(file.fs, file.pri_file);

    if (err != IPGUI_ERR_OK || br != file_size) {
        ipgui_mem_free_def(buf);
        return IPGUI_ERR_FS_FREAD;
    }

    /* 3. 作为内存字体加载 */
    err = ipgui_ft_load_font_from_memory(font, buf, file_size);
    ipgui_mem_free_def(buf);

    if (err != IPGUI_ERR_OK) {
        FT_DBG_ERROR("failed to parse font from file: %s", filepath);
    } else {
        FT_DBG_INFO("font loaded from file: %s (%u bytes)", filepath, file_size);
    }

    return err;
}

__IPGUI_API__ ipgui_err_t ipgui_ft_load_font_from_memory(
    ipgui_ft_font_t * font, const u8_t * data, u32_t data_size)
{
    struct ipgui_ft_font_internal * fi;
    ipgui_err_t err;

    if (!font || !data || data_size == 0) return IPGUI_ERR_PARAM;

    ipgui_memset(font, 0, sizeof(*font));

    /* 1. 分配内部结构 */
    fi = (struct ipgui_ft_font_internal *)
        ipgui_mem_alloc_def(sizeof(*fi));
    if (!fi) return IPGUI_ERR_NOMEM;
    ipgui_memset(fi, 0, sizeof(*fi));

    /* 2. 拷贝字体数据 */
    fi->font_data = (u8_t *)ipgui_mem_alloc_def(data_size);
    if (!fi->font_data) {
        ipgui_mem_free_def(fi);
        return IPGUI_ERR_NOMEM;
    }
    ipgui_memcpy(fi->font_data, data, data_size);
    fi->font_data_size = data_size;
    fi->load_from_mem = 1;

    /* 3. 初始化 glyph 缓存链表 */
    INIT_LIST_HEAD(&fi->glyph_cache);
    fi->glyph_count = 0;
    fi->tick_counter = 0;

#if IPGUI_USE_FREETYPE
    /* 4. 获取全局 FreeType 库句柄 */
    err = ft_library_acquire(&fi->library);
    if (err != IPGUI_ERR_OK) {
        ipgui_mem_free_def(fi->font_data);
        ipgui_mem_free_def(fi);
        return err;
    }

    /* 5. 创建 FT_Face（从内存） */
    /* FT_Error fterr = FT_New_Memory_Face( */
    /*     fi->library,                     */
    /*     (const FT_Byte *)fi->font_data,  */
    /*     (FT_Long)data_size,              */
    /*     0,                               // face_index  */
    /*     &fi->face);                      */
    /* if (fterr) { */
    /*     FT_DBG_ERROR("FT_New_Memory_Face failed: %d", fterr); */
    /*     ft_library_release(); */
    /*     ipgui_mem_free_def(fi->font_data); */
    /*     ipgui_mem_free_def(fi); */
    /*     return IPGUI_ERR_NOK; */
    /* } */

    /* 6. 设置默认字体大小 */
    /* ipgui_ft_set_size_internal(fi, IPGUI_FT_DEFAULT_FONT_SIZE); */
#else
    (void)err;
    fi->library = NULL;
    fi->face = NULL;
#endif

    /* 7. 组装基类 ipgui_font_t */
    font->base.glyphs     = NULL;   /* FreeType 不使用数组，运行时查询 */
    font->base.space_width = IPGUI_FT_DEFAULT_FONT_SIZE / 3;
    font->base.baseline   = IPGUI_FT_DEFAULT_FONT_SIZE;
    font->internal        = fi;

    return IPGUI_ERR_OK;
}

__IPGUI_API__ void ipgui_ft_unload_font(ipgui_ft_font_t * font)
{
    struct ipgui_ft_font_internal * fi;

    if (!font || !font->internal) return;

    fi = font->internal;

    /* 清空 glyph 缓存 */
    ipgui_ft_cache_clear(font);

#if IPGUI_USE_FREETYPE
    /* 释放 FT_Face */
    if (fi->face) {
        /* FT_Done_Face(fi->face); */
        fi->face = NULL;
    }

    /* 释放 FT_Library */
    ft_library_release();
    fi->library = NULL;
#endif

    /* 释放字体数据副本 */
    if (fi->font_data) {
        ipgui_mem_free_def(fi->font_data);
        fi->font_data = NULL;
    }

    /* 释放内部结构 */
    ipgui_mem_free_def(fi);
    font->internal = NULL;

    FT_DBG_INFO("font unloaded");
}

/* ================================================================
 * 字体参数配置
 * ================================================================ */

__IPGUI_STATIC__ void ft_update_metrics(struct ipgui_ft_font_internal * fi)
{
#if IPGUI_USE_FREETYPE
    /*
    if (fi->face) {
        FT_Face face = fi->face;
        FT_Size_Metrics * m = &face->size->metrics;
        fi->line_height = (ipgui_coord_t)(m->height >> 6);
        fi->baseline    = (ipgui_coord_t)(m->ascender >> 6);
        fi->space_width = (ipgui_coord_t)(fi->font_size / 3); // 近似
    }
    */
#else
    (void)fi;
#endif
}

__IPGUI_API__ void ipgui_ft_set_size(ipgui_ft_font_t * font, ipgui_coord_t pixel_size)
{
    struct ipgui_ft_font_internal * fi;

    if (!font || !font->internal || pixel_size <= 0) return;
    fi = font->internal;

    if (fi->font_size == pixel_size && !fi->need_reload) return;

    fi->font_size = pixel_size;

#if IPGUI_USE_FREETYPE
    /* FT_Error fterr = FT_Set_Pixel_Sizes(fi->face, 0, (FT_UInt)pixel_size); */
    /* if (fterr) { */
    /*     FT_DBG_ERROR("FT_Set_Pixel_Sizes(%d) failed: %d", pixel_size, fterr); */
    /*     return; */
    /* } */
#endif

    /* 字体大小变更后必须清空缓存并重新计算度量 */
    ipgui_ft_cache_clear(font);
    ft_update_metrics(fi);
    fi->need_reload = 0;

    /* 更新基类字段 */
    font->base.space_width = fi->space_width;
    font->base.baseline    = fi->baseline;
    font->base.line_height = fi->line_height;
    font->base.font_size   = fi->font_size;

    FT_DBG_INFO("font size set to %d px, baseline=%d, line_height=%d",
                pixel_size, fi->baseline, fi->line_height);
}

__IPGUI_API__ ipgui_coord_t ipgui_ft_get_size(const ipgui_ft_font_t * font)
{
    if (!font || !font->internal) return 0;
    return font->internal->font_size;
}

/* ================================================================
 * Glyph 查询（对外 API）
 * ================================================================ */

__IPGUI_API__ const ipgui_glyph_t * ipgui_ft_get_glyph(
    const ipgui_font_t * font, u32_t char_code)
{
    ipgui_ft_font_t * ft_font;
    ipgui_ft_glyph_cache_item_t * item;

    if (!font) return NULL;

    /* 上溯到 ipgui_ft_font_t（font 是 ft_font->base 的地址） */
    ft_font = (ipgui_ft_font_t *)((u8_t *)font - 
        (u32_t)(uintptr_t)&(((ipgui_ft_font_t *)0)->base));

    if (!ft_font->internal) return NULL;

    item = ipgui_ft_cache_get_glyph(ft_font, char_code);
    if (!item) return NULL;

    /* 返回缓存的 glyph 信息——复用同一个静态占位符 */
    return (const ipgui_glyph_t *)item;  /* 字段对齐兼容 */
}

/* ================================================================
 * 缓存统计
 * ================================================================ */

__IPGUI_API__ void ipgui_ft_cache_clear_public(ipgui_ft_font_t * font)
{
    ipgui_ft_cache_clear(font);
}

__IPGUI_API__ void ipgui_ft_cache_stats(
    const ipgui_ft_font_t * font, s32_t * count, u32_t * pixels)
{
    struct ipgui_ft_font_internal * fi;

    if (!font || !font->internal) {
        if (count)  *count  = 0;
        if (pixels) *pixels = 0;
        return;
    }

    fi = font->internal;
    if (count)  *count  = fi->glyph_count;
#if IPGUI_FT_GLYPH_CACHE_TOTAL_PIXELS > 0
    if (pixels) *pixels = fi->total_pixels;
#else
    if (pixels) *pixels = 0;
#endif
}
