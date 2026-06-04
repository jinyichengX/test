#ifndef IPGUI_PATH_H
#define IPGUI_PATH_H

#include "ipgui_utils.h"
#include "ipgui_prim.h"
#include "ipgui_coord.h"

IPGUI_HEADER_BEGIN
#define FALSE 0
#define TRUE 1
typedef enum
{
    IPGUI_PATH_CODE_MOVE_TO     = 0,
    IPGUI_PATH_CODE_LINE_TO     = 1,
    IPGUI_PATH_CODE_QUAD_TO     = 2,
    IPGUI_PATH_CODE_CUBIC_TO    = 3,
    IPGUI_PATH_CODE_CLOSE       = 4,
}ipgui_path_code_e;

// typedef struct ipgui_path_context
// {   
//     ipgui_point_t * points;
// }ipgui_path_t;

typedef struct {
    // ipgui_node_t link;
    unsigned int num_ops;
    unsigned int size_ops;
    unsigned int num_points;
    unsigned int size_points;

    ipgui_path_code_e *op;
    ipgui_point_t *points;
} ipgui_path_buf_t;

typedef struct {
    ipgui_path_buf_t base;

    ipgui_path_code_e op[20];
    ipgui_point_t points[2 * 20];//路径锚点
} ipgui_path_buf_fixed_t;

/*
  NOTES:
  has_curve_to => !stroke_is_rectilinear
  fill_is_rectilinear => stroke_is_rectilinear
  fill_is_empty => fill_is_rectilinear
  fill_maybe_region => fill_is_rectilinear
*/
typedef struct _ipgui_path_context {
    ipgui_point_t last_move_point;    // 上一次 move_to 操作的点
    ipgui_point_t current_point;     // 当前路径的最后一个点
    unsigned int has_current_point   : 1;  // 是否有当前点
    unsigned int needs_move_to       : 1;  // 是否需要 move_to 操作
    unsigned int has_extents         : 1;  // 是否有计算好的边界框
    unsigned int has_curve_to        : 1;  // 是否包含贝塞尔曲线
    unsigned int stroke_is_rectilinear : 1;  // 描边是否是矩形/直线的？
    unsigned int fill_is_rectilinear : 1;  // 填充是否是矩形/直线的
    unsigned int fill_maybe_region   : 1;  // 填充是否可能是区域
    unsigned int fill_is_empty       : 1;  // 填充是否为空

    ipgui_aabb_t extents;             // 路径的边界框
    ipgui_path_buf_fixed_t buf;      // 路径缓冲区，存储路径操作和点
}ipgui_path_t;
extern void ipgui_path_init(ipgui_path_t *path);
ipgui_err_t
ipgui_path_fixed_move_to (ipgui_path_t  *path,
			   ipgui_coord_t	x,
			   ipgui_coord_t	y);
               ipgui_err_t
ipgui_path_fixed_rel_line_to (ipgui_path_t *path,
			       ipgui_coord_t	   dx,
			       ipgui_coord_t	   dy);
extern void ipgui_path_move_to(ipgui_path_t *path, ipgui_point_t p);
extern void ipgui_path_line_to(ipgui_path_t *path, ipgui_point_t p);
extern void ipgui_path_close(ipgui_path_t *path);

IPGUI_HEADER_END

#endif