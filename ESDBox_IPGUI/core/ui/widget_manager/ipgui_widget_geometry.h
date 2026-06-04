#ifndef IPGUI_WIDGET_GEOMETRY_H
#define IPGUI_WIDGET_GEOMETRY_H

#include "ipgui_prim.h"
#include "ipgui_widget.h"
#include "ipgui_utils.h"
#include "ipgui_widget_tree.h"

/* if need gesture to change widget's size, need this struct */
typedef struct {
    float scale_x, scale_y; /* XY方向缩放比例 */
    ipgui_coord_t zoom_cx, zoom_cy; /* 缩放中心点（相对于控件左上角） */
}widget_scale_t;

typedef struct {
    /* 位置和大小约束（控件内容） */
    ipgui_coord_t x, y; /* the start position of it's parent aabb area */
    ipgui_coord_t w, h; /* the widget's width and height */

    /* 布局约束，在控件内容上的扩展空间 */
    /* 内边距是边框和内容的距离，外边距是控件之间的距离约束 */
    ipgui_coord_t padding; /* 内边距宽度 */
    ipgui_coord_t border;   /* 边框宽度 */
    ipgui_coord_t radius;   /* 边框圆角半径 */
    ipgui_coord_t margin; /* 外边距宽度 */

}widget_geometry_t;

#endif