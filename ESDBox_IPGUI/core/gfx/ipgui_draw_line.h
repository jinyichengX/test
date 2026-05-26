#ifndef ipgui_draw_line_generic_H
#define ipgui_draw_line_generic_H

#include "ipgui_core.h"
#include "ipgui_coord.h"
#include "ipgui_color.h"
#include "ipgui_prim.h"
#include "ipgui_blend.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IPGUI_LINE_CAP_BUTT = 0,
    IPGUI_LINE_CAP_ROUND,
}ipgui_line_cap_t;

typedef enum {
    /* 跟随线条（渐变方向：起始点到结束点） */
    IPGUI_LINE_GRADIENT_FOLLIOW = 0, 

    /* 线内水平渐变 */
    IPGUI_LINE_GRADIENT_HOR,    

    /* 线内垂直渐变 */
    IPGUI_LINE_GRADIENT_VER, 
}ipgui_line_gradient_dir_t;

typedef struct {
    ipgui_coord_t        width;
    ipgui_line_cap_t     cap; /* 默认垂直/水平线为平头线帽，斜线为圆头线帽 */

    ipgui_paint_t        paint;

    u8_t                 opacity;
    ipgui_blend_mode_t   blend_mode;
}ipgui_line_style_t;

extern __IPGUI_API__ void ipgui_draw_line_generic(       
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_line_t       * line, 
    ipgui_line_style_t * style);

extern __IPGUI_API__ void ipgui_draw_line_classic(
    ipgui_surf_t       * surf, 
    ipgui_aabb_t       * clip,
    ipgui_line_t       * line, 
    ipgui_line_style_t * style);

#ifdef __cplusplus
}
#endif

#endif