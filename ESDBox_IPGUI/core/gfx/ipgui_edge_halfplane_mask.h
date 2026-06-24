#ifndef __ipgui_edge_halfplane_mask_H__
#define __ipgui_edge_halfplane_mask_H__

#include "ipgui_color.h"
#include "ipgui_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef s32_t ipgui_edge_coord_t; // 64位定点数 (val << 6)

typedef enum {
    EDGE_HALFPLANE_DIR_LEFT,
    EDGE_HALFPLANE_DIR_RIGHT,
} edge_halfplane_dir_t;

typedef struct {
    ipgui_edge_coord_t   x1;
    ipgui_edge_coord_t   y1;

    /* slope of the edge */
    ipgui_edge_coord_t   dx; 
    ipgui_edge_coord_t   dy;

    /* use delta_x and delta_y when it is a flatten edge */
    s32_t                delta_x;     /* must be 65536             *//* 好像没用 */
    s32_t                delta_y;     /* delta_y / 65536 = abs(dy / dx) */

    u8_t                 correction_frac_index : 7;     /* 修正因子索引，因为d0记录的不是点到直线的垂直距离而是轴向距离，所以d0要乘以这个修正因子，修正因子分母256，修正因子<=256             */
    u8_t                 flatten : 1; /* 1: a flatten edge, 0: a steep edge */
}ipgui_edge_param_t;

typedef struct {
    edge_halfplane_dir_t dir;
    ipgui_coord_t        y;         /* the y coordinate of the scan line that the descriptor acts on */
    ipgui_coord_t        x_start;   /* the first masked point's x coordination       */
    ipgui_edge_coord_t   frac_x;    /* the first masked point's x distance to the edge */
    ipgui_edge_param_t * p;
}ipgui_edge_halfplane_mask_dsc_t;

extern __IPGUI_API__ ipgui_edge_coord_t edge_x_at_y(
    ipgui_edge_param_t * p, 
    ipgui_coord_t        y);

extern __IPGUI_API__ ipgui_edge_param_t ipgui_edge_param_init(
    ipgui_edge_coord_t   x1,
    ipgui_edge_coord_t   y1,
    ipgui_edge_coord_t   x2,
    ipgui_edge_coord_t   y2);

extern __IPGUI_API__ void ipgui_gen_edge_halfplane_mask_dsc(
    ipgui_edge_halfplane_mask_dsc_t * res,
    edge_halfplane_dir_t              dir,
    ipgui_edge_param_t              * p, 
    ipgui_coord_t                     y);

extern __IPGUI_API__ u8_t ipgui_edge_halfplane_mask(
    ipgui_edge_halfplane_mask_dsc_t * dsc, 
    ipgui_edge_coord_t                x);

extern __IPGUI_API__ ipgui_edge_coord_t align_down_64(ipgui_edge_coord_t c);
extern __IPGUI_API__ ipgui_edge_coord_t align_up_64  (ipgui_edge_coord_t c);

#ifdef __cplusplus
}
#endif

#endif