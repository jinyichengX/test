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
}ipgui_edge_wdf_mask_dsc_t;

extern __IPGUI_API__ ipgui_edge_wdf_param_t ipgui_edge_wdf_param_init(
    ipgui_coord_t x1,
    ipgui_coord_t y1,
    ipgui_coord_t x2,
    ipgui_coord_t y2);
    
extern __IPGUI_API__ ipgui_coord_t ipgui_edge_wdf_xspan(
    ipgui_edge_wdf_param_t * param,
    ipgui_coord_t width);

#endif