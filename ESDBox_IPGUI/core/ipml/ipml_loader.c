/**
 * @file ipml_loader.c
 * @brief IPML 二进制 (.ipb) 加载器 — 反序列化为 widget 树
 *
 * ============ IPB 二进制格式规范 ============
 *
 * ┌──────────────────────────────────────┐
 * │ Header (8 bytes)                     │
 * │  magic[4]   = "IPB\0"               │
 * │  version    = u16 (当前 1)           │
 * │  total_size = u16 (不含 header 的              │
 * │                       数据总字节数)    │
 * ├──────────────────────────────────────┤
 * │ Widget Tree (total_size bytes)       │
 * │  递归 WidgetNode:                    │
 * │                                      │
 * │  attr_mask  = u16 (位掩码)           │
 * │    bit0: x (s16)                     │
 * │    bit1: y (s16)                     │
 * │    bit2: w (s16)                     │
 * │    bit3: h (s16)                     │
 * │    bit4: name (u8 len + char[])      │
 * │    bit5: render_sym (u8 len + char[]) │
 * │    bit6: event_sym  (u8 len + char[]) │
 * │    bit7: flags (u16)                 │
 * │    bit8: scroll_dir (u8)             │
 * │                                      │
 * │  [各属性按 bit 顺序编码]             │
 * │                                      │
 * │  child_count = u8                    │
 * │  children[]  = WidgetNode...         │
 * └──────────────────────────────────────┘
 *
 * 所有多字节整数为 little-endian。
 * 位序: bit0 (LSB) 最先编码。
 */

#include "ipml_loader.h"
#include <string.h>
#include "ipgui_widget_tree.h"

/* 内部字符串比较，兼容无标准库的环境 */
static int ipml_strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static u16_t ipml_read_u16(const u8_t **p)
{
    u16_t v = (u16_t)(*p)[0] | ((u16_t)(*p)[1] << 8);
    *p += 2;
    return v;
}

static s16_t ipml_read_s16(const u8_t **p)
{
    return (s16_t)ipml_read_u16(p);
}

static u8_t ipml_read_u8(const u8_t **p)
{
    return *(*p)++;
}

/* attr_mask 位定义 */
#define ATTR_X          0
#define ATTR_Y          1
#define ATTR_W          2
#define ATTR_H          3
#define ATTR_NAME       4
#define ATTR_RENDER     5
#define ATTR_EVENT      6
#define ATTR_FLAGS      7
#define ATTR_SCROLL_DIR 8

#define ATTR_MASK_BIT(_n)  (1u << (_n))

/* 符号表查找 */
static void *ipml_lookup_sym(const char *name,
                             const ipml_symbol_entry_t *tab, u16_t count)
{
    for (u16_t i = 0; i < count; i++) {
        if (ipml_strcmp(tab[i].name, name) == 0)
            return tab[i].fn;
    }
    return 0;
}

/* 递归解析一个 WidgetNode */
static const u8_t *ipml_parse_node(const u8_t *p, const u8_t *end,
                                   ipgui_widget_t *parent,
                                   const ipml_symbol_entry_t *symtab,
                                   u16_t sym_count,
                                   ipml_result_t *result)
{
    if (p + 2 > end) return end; /* attr_mask 至少 2 字节 */

    u16_t mask = ipml_read_u16(&p);

    /* 读取各属性 */
    s16_t x = 0, y = 0, w = 100, h = 100; /* 默认值 */
    const char *name_str = 0;
    const char *render_str = 0;
    const char *event_str = 0;
    u16_t flags = 0;
    u8_t scroll_dir = 0;
    u8_t has_scroll_dir = 0;

    if (mask & ATTR_MASK_BIT(ATTR_X))       x = ipml_read_s16(&p);
    if (mask & ATTR_MASK_BIT(ATTR_Y))       y = ipml_read_s16(&p);
    if (mask & ATTR_MASK_BIT(ATTR_W))       w = ipml_read_s16(&p);
    if (mask & ATTR_MASK_BIT(ATTR_H))       h = ipml_read_s16(&p);

    if (mask & ATTR_MASK_BIT(ATTR_NAME)) {
        u8_t nlen = ipml_read_u8(&p);
        if (nlen > 63) nlen = 63;
        if (p + nlen > end) return end;
        name_str = (const char *)p; /* 指向 IPB 数据流中的原地字符串 */
        p += nlen;
    }

    if (mask & ATTR_MASK_BIT(ATTR_RENDER)) {
        u8_t nlen = ipml_read_u8(&p);
        if (nlen > 63) nlen = 63;
        if (p + nlen > end) return end;
        render_str = (const char *)p;
        p += nlen;
    }

    if (mask & ATTR_MASK_BIT(ATTR_EVENT)) {
        u8_t nlen = ipml_read_u8(&p);
        if (nlen > 63) nlen = 63;
        if (p + nlen > end) return end;
        event_str = (const char *)p;
        p += nlen;
    }

    if (mask & ATTR_MASK_BIT(ATTR_FLAGS))   flags      = ipml_read_u16(&p);
    if (mask & ATTR_MASK_BIT(ATTR_SCROLL_DIR)) {
        scroll_dir    = ipml_read_u8(&p);
        has_scroll_dir = 1;
    }

    /* 创建 widget */
    ipgui_widget_t *wgt = ipgui_widget_create(parent);
    if (!wgt) return end;

    wgt->x = x;
    wgt->y = y;
    wgt->w = w;
    wgt->h = h;
    wgt->flags = flags;

    if (has_scroll_dir)
        wgt->scroll_dir = (ipgui_scroll_dir_t)scroll_dir;

    /* 名称指向 IPB 数据中的静态字符串（ROM），生命周期 = 程序生命周期 */
    if (name_str)
        wgt->name = name_str;

    /* 绑定符号 */
    if (render_str) {
        wgt->render = (void (*)(struct ipgui_widget*, ipgui_widget_render_ctx_t*))
                      ipml_lookup_sym(render_str, symtab, sym_count);
        if (!wgt->render) result->unresolved++;
    }
    if (event_str) {
        wgt->event_handler = (void (*)(struct ipgui_widget*, ipgui_widget_evt_t*))
                             ipml_lookup_sym(event_str, symtab, sym_count);
        if (!wgt->event_handler) result->unresolved++;
    }

    result->widget_count++;

    /* 设置 root */
    if (!parent && !result->root)
        result->root = wgt;

    /* 解析子节点 */
    if (p >= end) return end;
    u8_t child_count = ipml_read_u8(&p);

    for (u8_t i = 0; i < child_count; i++) {
        p = ipml_parse_node(p, end, wgt, symtab, sym_count, result);
        if (p >= end) break;
    }

    return p;
}

ipgui_err_t ipml_load(const u8_t *data, u32_t len,
                      const ipml_symbol_entry_t *symtab, u16_t sym_count,
                      ipml_result_t *result)
{
    if (!data || !result || len < 8) return IPGUI_ERR_PARAM;

    ipml_result_t res;
    memset(&res, 0, sizeof(res));

    /* 校验 magic */
    if (data[0] != 'I' || data[1] != 'P' || data[2] != 'B' || data[3] != 0)
        return IPGUI_ERR_PARAM;

    /* 版本号（暂不校验，向前兼容） */
    /* u16_t version = data[4] | (data[5] << 8); */

    u16_t total_size = data[6] | (data[7] << 8);
    if (8 + total_size > len) return IPGUI_ERR_PARAM;

    const u8_t *p = data + 8;
    const u8_t *end = p + total_size;

    p = ipml_parse_node(p, end, 0, symtab, sym_count, &res);

    *result = res;
    return IPGUI_ERR_OK;
}

/* ==========================================================================
 * 按名称查找 widget
 * ========================================================================== */

typedef struct {
    const char       *target;
    ipgui_widget_t   *found;
} ipml_find_ctx_t;

static int ipml_find_cb(struct widget_link_t *link, void *arg)
{
    ipml_find_ctx_t *ctx = (ipml_find_ctx_t *)arg;
    ipgui_widget_t *w = (ipgui_widget_t *)link;
    if (w->name && ipml_strcmp(w->name, ctx->target) == 0) {
        ctx->found = w;
        return 1; /* 找到则终止遍历 */
    }
    return 0;
}

ipgui_widget_t * ipml_widget_find_by_name(ipgui_widget_t *root, const char *name)
{
    if (!name) return 0;

    ipml_find_ctx_t ctx;
    ctx.target = name;
    ctx.found = 0;

    ipgui_widget_link_foreach_dfs(
        root ? &root->link : 0,
        ipml_find_cb, &ctx);

    return ctx.found;
}
