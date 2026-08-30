#ifndef IPGUI_IMAGE_GEOMETRY_TRANSFORM_H
#define IPGUI_IMAGE_GEOMETRY_TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_core.h"
#include "ipgui_graphic2.h"
#include "ipgui_math.h"
#include "ipgui_image.h"

typedef enum {
    IPGUI_LERP_NEAREST,     /* 最邻近插值 */
    IPGUI_LERP_BILINEAR     /* 双线性插值 */
} ipgui_lerp_method_t;

typedef struct {
    /* T = |a  b|
     *     |c  d|
     */
    ipgui_scoord_t a, b, c, d;
}ipgui_img_geo_trans_mat_t;

typedef struct {
    ipgui_point_t pivot; /* a transformation center, the relative coordinate to image */
    ipgui_img_geo_trans_mat_t mat;

    /* some special state */
    int only_rotate_n90 : 1; // ?
    int degree : 31; // ?
}ipgui_img_geo_trans_t;

extern __IPGUI_API__ void ipgui_image_trans_init(ipgui_img_geo_trans_t * trans);
extern __IPGUI_API__ void ipgui_image_trans_pivot(ipgui_img_geo_trans_t * trans, ipgui_point_t pp);
extern __IPGUI_API__ void ipgui_image_trans_scale(ipgui_img_geo_trans_t * trans, float sx, float sy);
extern __IPGUI_API__ void ipgui_image_trans_rotate_degree(ipgui_img_geo_trans_t * trans, int deg/* 0-360 is a period */);
extern __IPGUI_API__ void ipgui_image_trans_reflect_x(ipgui_img_geo_trans_t * trans);
extern __IPGUI_API__ void ipgui_image_trans_reflect_y(ipgui_img_geo_trans_t * trans);
extern __IPGUI_API__ void ipgui_image_trans_shear(ipgui_img_geo_trans_t * trans, float sx, float sy);
extern __IPGUI_API__ ipgui_img_dsc_t * ipgui_image_geo_transform(
    ipgui_img_dsc_t * image,
    ipgui_img_geo_trans_t * trans,
    ipgui_lerp_method_t lerp);

#ifdef __cplusplus
}
#endif

#endif