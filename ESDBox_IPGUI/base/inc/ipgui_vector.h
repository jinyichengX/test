#ifndef __IPGUI_VECTOR_H__
#define __IPGUI_VECTOR_H__

#include "ipgui_utils.h"
#include "ipgui_graphic2.h"
#include "ipgui_math.h"
#include "ipgui_prim.h"

extern __IPGUI_API__ ipgui_scoord_t ipgui_vector_mod                (ipgui_svector_t * v);

extern __IPGUI_API__ ipgui_scoord_t ipgui_vector_len                (ipgui_svector_t * v);

extern __IPGUI_API__ void           ipgui_vector_rotate             (ipgui_svector_t * v,    ipgui_angle_t     angle, ipgui_svector_t * out);

extern __IPGUI_API__ void           ipgui_vector_rotate_screen      (ipgui_svector_t * v,    ipgui_angle_t     angle, ipgui_svector_t * out);

extern __IPGUI_API__ void           ipgui_vector_polarization       (ipgui_svector_t * v,    ipgui_angle_t *   angle, ipgui_scoord_t *  mod);

extern __IPGUI_API__ void           ipgui_vector_polarization_screen(ipgui_svector_t * v,    ipgui_angle_t *   angle, ipgui_scoord_t *  mod);

extern __IPGUI_API__ ipgui_angle_t  ipgui_vector_angle_diff         (ipgui_svector_t * from, ipgui_svector_t * to);

extern __IPGUI_API__ ipgui_angle_t  ipgui_vector_angle_diff_screen  (ipgui_svector_t * from, ipgui_svector_t * to);

#endif