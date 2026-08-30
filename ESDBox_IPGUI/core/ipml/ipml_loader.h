#ifndef IPML_LOADER_H
#define IPML_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_types.h"
#include "ipgui_widget.h"

/* ==========================================================================
 * IPML — IPGUI Markup Language 运行时加载器
 *
 * 工作流: main.ipml → ipml_compiler.py → main.ipb → ipml_load() → widget 树
 *
 * 符号绑定: 回调函数在 .ipml 中以字符串名指定，运行时查符号表绑定函数指针。
 *
 * 二进制格式: 见 ipml_loader.c 文件头注释
 * ========================================================================== */

/* 符号条目 */
typedef struct {
    const char *name;    /* 函数名字符串 */
    void       *fn;      /* 函数指针 */
} ipml_symbol_entry_t;

/* 加载结果 */
typedef struct {
    ipgui_widget_t *root;             /* 根 widget（对应 .ipml 最外层） */
    u16_t           widget_count;     /* 创建的 widget 总数 */
    u16_t           unresolved;       /* 未解析的符号数 */
} ipml_result_t;

/* ==========================================================================
 * API
 * ========================================================================== */

/**
 * @brief 加载 .ipb 二进制数据，创建 widget 树并绑定符号
 *
 * @param data        .ipb 数据
 * @param len         数据字节数
 * @param symtab      符号表（回调函数名 → 函数指针）
 * @param sym_count   符号表条目数
 * @param result      [out] 加载结果
 * @return IPGUI_ERR_OK 成功
 */
ipgui_err_t ipml_load(const u8_t *data, u32_t len,
                      const ipml_symbol_entry_t *symtab, u16_t sym_count,
                      ipml_result_t *result);

/**
 * @brief 在 widget 子树中按名称查找控件（递归 DFS）
 *
 * @param root  搜索起点（NULL 查整个 main_screen）
 * @param name  控件名称（.ipml 中的 name 属性）
 * @return 找到的 widget，未找到返回 NULL
 */
ipgui_widget_t * ipml_widget_find_by_name(ipgui_widget_t *root, const char *name);

/* 便捷宏: 注册一个符号 */
#define IPML_SYM(_fn)  { #_fn, (void *)(_fn) }

#ifdef __cplusplus
}
#endif

#endif /* IPML_LOADER_H */
