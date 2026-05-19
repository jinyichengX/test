#ifndef IPGUI_EDGE_WDF_MASK_H
#define IPGUI_EDGE_WDF_MASK_H

#include "ipgui_types.h"
#include "ipgui_coord.h"
#include "ipgui_color.h"
#include "ipgui_core.h"

typedef struct {
    ipgui_coord_t a, b, c;

    ipgui_coord_t x1, y1; /* 线段中y值小的那个点 */
 
    ipgui_coord_t dy, dx; /* dy always > 0 */

    s32_t                delta_y;     /* delta_y / 65536 = abs(dy / dx) */

    u8_t                 correction_frac_index : 7;     /* 修正因子索引，因为d0记录的不是点到直线的垂直距离而是轴向距离，所以d0要乘以这个修正因子，修正因子分母256，修正因子<=256             */
    u8_t                 flatten : 1; /* 1: a flatten edge, 0: a steep edge */
}ipgui_edge_wdf_param_t;

typedef struct {
    ipgui_coord_t inte;
    s32_t         frac;    /* 0 ~ dy */
}ipgui_xstep_t, ipgui_xidx_t;

typedef struct {
    ipgui_edge_wdf_param_t * p;
    ipgui_coord_t            x_half_span;
    ipgui_xstep_t            x_step;
    ipgui_xidx_t             x_idx;
    u32_t                    half_width64; /* 4 byte padding */
}ipgui_edge_wdf_mask_dsc_t;

extern __IPGUI_API__ ipgui_edge_wdf_param_t ipgui_edge_wdf_param_init(
    ipgui_coord_t x1,
    ipgui_coord_t y1,
    ipgui_coord_t x2,
    ipgui_coord_t y2);
    
extern __IPGUI_API__ ipgui_coord_t ipgui_edge_wdf_xspan(
    ipgui_edge_wdf_param_t * param,
    ipgui_coord_t width);

extern __IPGUI_API__ void ipgui_gen_edge_mask_dsc(
    ipgui_edge_wdf_mask_dsc_t * res,
    ipgui_edge_wdf_param_t    * param,
    ipgui_coord_t               start_y,
    ipgui_coord_t               width);

#endif