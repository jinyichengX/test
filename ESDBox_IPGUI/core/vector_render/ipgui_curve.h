#ifndef __IPGUI_CURVE_H__
#define __IPGUI_CURVE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_utils.h"
#include "ipgui_graphic2.h"
#include "ipgui_prim.h"

typedef struct {
    ipgui_scoord_t x0, y0;
    ipgui_scoord_t x1, y1;
    ipgui_scoord_t x2, y2;
}ipgui_curve2_t;

typedef struct {
    ipgui_scoord_t x0, y0;
    ipgui_scoord_t x1, y1;
    ipgui_scoord_t x2, y2;
    ipgui_scoord_t x3, y3;
}ipgui_curve3_t;
__IPGUI_API__ void ipgui_generate_quad_circle_control(ipgui_scoord_t radius,
                                            ipgui_svector_t * v0, ipgui_svector_t * v1,
                                            ipgui_svector_t *v2, ipgui_svector_t * v3);

#ifdef __cplusplus
}
#endif

#endif