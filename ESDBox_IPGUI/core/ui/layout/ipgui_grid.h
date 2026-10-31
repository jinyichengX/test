#ifndef IPGUI_GRID_H
#define IPGUI_GRID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_utils.h"
#include "ipgui_types.h"
#include "ipgui_widget.h"

/*
 * 网格布局（CSS Grid 的极简子集）
 *
 * 一次调用完成整容器的子控件排布：
 *   - 轨道定义：固定像素 / fr 弹性份额 / auto 内容自适应
 *   - 行列间距 gap、内边距 padding
 *   - 跨行跨列 span、显式指定起始格
 *   - 未定义的轨道自动按 auto 隐式补齐
 *   - 子控件按控件树顺序（first_child → sib_next）参与排布
 *
 * 尺寸来源：框架没有独立的 measure 阶段，auto 轨道直接以子控件当前的
 * w/h 作为内容尺寸；排布完成后子控件会被拉伸填满所占单元格（含跨格）。
 *
 * 嵌套：网格容器本身可以是另一个网格/布局容器的子控件，互不影响。
 *
 * 使用方式：
 *   ipgui_grid_apply(container, &cfg);
 * 每次容器尺寸或子控件集合变化后重新调用即可。
 */

/* 轨道数量上限，可在工程级重定义 */
#ifndef IPGUI_GRID_MAX_COLS
#define IPGUI_GRID_MAX_COLS     16
#endif
#ifndef IPGUI_GRID_MAX_ROWS
#define IPGUI_GRID_MAX_ROWS     32
#endif

#if IPGUI_GRID_MAX_ROWS > 32
#error "IPGUI_GRID_MAX_ROWS must not be greater than 32"
#endif

/* 自动放置哨兵：ipgui_grid_item_t.col / row 取此值表示由自动流决定 */
#define IPGUI_GRID_AUTO         0xFF

/* 轨道类型 */
typedef enum {
    IPGUI_GRID_TRACK_FIXED = 0,     /* 固定像素：value 为像素宽度 */
    IPGUI_GRID_TRACK_FR,            /* 弹性份额：value 为份额，按剩余空间分配 */
    IPGUI_GRID_TRACK_AUTO,          /* 内容自适应：value 忽略，取子控件 w/h */
} ipgui_grid_track_type_t;

/* 单条轨道定义 */
typedef struct {
    u8_t            type;           /* ipgui_grid_track_type_t */
    ipgui_coord_t   value;          /* FIXED: 像素; FR: 份额; AUTO: 忽略 */
} ipgui_grid_track_t;

/*
 * 单个子控件的排布覆写
 * 数组中第 i 项对应控件树中第 i 个子控件（从 first_child 起按 sib_next 顺序）。
 * 子控件数量多于 item_count 时，多出的控件按默认 1x1 自动放置。
 */
typedef struct {
    u8_t col;                       /* 起始列，IPGUI_GRID_AUTO 表示自动 */
    u8_t row;                       /* 起始行，IPGUI_GRID_AUTO 表示自动 */
    u8_t col_span;                  /* 跨列数，0 视为 1 */
    u8_t row_span;                  /* 跨行数，0 视为 1 */
} ipgui_grid_item_t;

/* 网格配置 */
typedef struct {
    const ipgui_grid_track_t * cols;    /* 列轨道定义，不能为空 */
    u8_t                       col_count; /* 列轨道数量，至少为 1 */

    const ipgui_grid_track_t * rows;    /* 行轨道定义，可为空（全部 auto） */
    u8_t                       row_count;

    const ipgui_grid_item_t  * items;   /* 子控件覆写表，可为空 */
    u8_t                       item_count;

    ipgui_coord_t              col_gap; /* 列间距 */
    ipgui_coord_t              row_gap; /* 行间距 */

    ipgui_coord_t              pad_left;
    ipgui_coord_t              pad_top;
    ipgui_coord_t              pad_right;
    ipgui_coord_t              pad_bottom;
} ipgui_grid_cfg_t;

/*
 * 对容器的所有子控件执行一次网格排布
 *
 * 以 container->w/h 为可用区域，扣除 padding 后按轨道定义分配空间，
 * 并直接改写子控件的 x/y/w/h（相对容器坐标）。
 *
 * 返回 IPGUI_ERR_OK；参数非法返回 IPGUI_ERR_PARAM；
 * 容器已脱离控件树返回 IPGUI_ERR_OBJ_INVALID。
 */
extern __IPGUI_API__ ipgui_err_t ipgui_grid_apply(
    ipgui_widget_t *        container,
    const ipgui_grid_cfg_t * cfg);

#ifdef __cplusplus
}
#endif

#endif /* IPGUI_GRID_H */
