#ifndef IPGUI_DRAW_POLYGON_H
#define IPGUI_DRAW_POLYGON_H

#include "ipgui_coord.h"
#include "ipgui_graphic2.h"
#include "ipgui_avl.h"
#include "ipgui_membox.h"
#include "ipgui_blend.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef s32_t ipgui_area_t;

#define IPGUI_WHOLE_COVER (IPGUI_PIXEL_AREA << 1) /* 真实子像素面积 * 2 */

typedef struct {
    ipgui_scoord_t inte;                    /* 不管是ipgui_edge_xstep_t还是ipgui_x_cur_t，inte必须是IPGUI_PIXEL_PRECI的整数倍 */
    s32_t frac;                               /* the frac of dy of edge */ /* formula: frac/dy = xx.xxxxxx% */
}ipgui_edge_xstep_t,
ipgui_x_cur_t;

typedef enum {
    IPGUI_EDGE_DIR_UP = 1,
    IPGUI_EDGE_DIR_DOWN = -1
}ipgui_edge_dir_t;

/* 扫描线从y_min（屏幕上方）向y_max扫描
 * 扫描方向与缠绕方向不一样！！！
 */
typedef struct _edge ipgui_edge_t;
typedef struct _edge{
    avl_node_t         hook;                /* 链接边表 */

    struct _edge *     next;                /* 链接活性边表 */
    struct _edge *     prev;                /* 链接活性边表 */

    ipgui_edge_dir_t   dir;                 /* +1:up -1:down */

    ipgui_coord_t      y_min;               /* 扫描线经过的y坐标最小值 */
    ipgui_coord_t      y_max;               /* 扫描线经过的y坐标最大值 */

    ipgui_scoord_t     y_start;             /* 扫描线开始的y坐标，在碰到y_min时有效 */
    ipgui_scoord_t     y_end;               /* 扫描线结束的y坐标，在碰到y_max时有效 */
    ipgui_x_cur_t      x_cur;               /* 扫描线当前的x坐标（实时计算） */
    ipgui_edge_xstep_t x_full_step;         /* 距离为1像素（64子像素）的扫描线x坐标的步进，在x_cur基础上的步进值 */

    ipgui_scoord_t     dx;                  /* 用于修正x_cur（特例：垂直边缘） */
    ipgui_scoord_t     dy;                  /* 正数，用于修正x_cur */
}ipgui_edge_t;
/* 
 * +-------------------------+-----------------------+
 * |                         |                       |
 * |                         |                       |
 * |_________________________|_______________________|
 * |     \...................|.......................|\
 * |      \..................|.......................| |
 * |       \.................|.......................| |
 * |        \................|.......................| |
 * |         \...............|.......................| } covered height
 * |          \..............|.......................| |
 * |(un)covered\.............|.......................| |
 * |  area      \............|.......................| |
 * |_____________\...........|.......................|/
 * |                         |                       |
 * |                         |                       |
 * |                         |                       |
 * +-------------------------+-----------------------+
 */
typedef struct _cell ipgui_cell_t;
typedef struct _cell{
    avl_node_t      hook;
    ipgui_coord_t   x;
    ipgui_area_t    area;          /* left area, and it is a positive number */
    ipgui_scoord_t  wind_height; /* one pixel crossed height by edge */
}ipgui_cell_t;

typedef enum {
    IPGUI_FILL_RULE_NONZERO,
    IPGUI_FILL_RULE_EVENODD
}ipgui_fill_rule_t;

typedef struct {
    ipgui_coord_t y_min;/* scan y */
    ipgui_coord_t y_max;

    avl_t edge_buckets;                 /* sorted by y_start of ipgui_edge_t */
    // s32_t num_buckets;
    ipgui_edge_t * active;              /* active edges*/
    avl_t scanline_cells;               /* sorted by x of ipgui_cell_t       */

    ipgui_membox_t edge_membox;
    ipgui_membox_t cell_membox;

    ipgui_fill_rule_t fill_rule;

    s32_t err;                            /* 错误码，只有内存分配失败时置1，其余为0 */
}ipgui_polygon_ras_t;

typedef struct {
    ipgui_color_t color;
    u8_t alpha;
    ipgui_blend_mode_t blend_mode;
}ipgui_polygon_style_t;

ipgui_polygon_ras_t g_ras;

__IPGUI_API__ ipgui_err_t ipgui_polygon_ras_init(ipgui_polygon_ras_t * ras);
__IPGUI_API__ void ipgui_polygon_ras_set_fill_rule(
                ipgui_polygon_ras_t * ras,
                ipgui_fill_rule_t rule);
__IPGUI_API__ ipgui_err_t ipgui_draw_polygon(ipgui_surf_t * surf, /* 使用这个API时，p中的元素必须按围成多边形轮廓的顺序排列，否则光栅化出来的多边形会与预期不符 */
                ipgui_aabb_t * clip,
                ipgui_point_t * p, s32_t num, 
                ipgui_polygon_ras_t * ras,
                ipgui_polygon_style_t * attr);
__IPGUI_API__ ipgui_err_t ipgui_draw_spolygon(ipgui_surf_t * surf,/* 使用这个API时，p中的元素必须按围成多边形轮廓的顺序排列，否则光栅化出来的多边形会与预期不符 */
                ipgui_aabb_t * clip,
                ipgui_spoint_t * p, s32_t num,
                ipgui_polygon_ras_t * ras,
                ipgui_polygon_style_t * attr);
                
#ifdef __cplusplus
}
#endif

#endif