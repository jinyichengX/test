#ifndef IPGUI_IMAGE_RAS_H
#define IPGUI_IMAGE_RAS_H

#include "ipgui_image.h"
#include "ipgui_utils.h"
#include "ipgui_core.h"
#include "ipgui_color.h"
#include "ipgui_prim.h"

typedef enum {
    /* original show mode(crop automatically) */
    IPGUI_IMG_SHOW_ORI_CENTER,
    IPGUI_IMG_SHOW_ORI_TOP,
    IPGUI_IMG_SHOW_ORI_BOTTOM,
    IPGUI_IMG_SHOW_ORI_LEFT,
    IPGUI_IMG_SHOW_ORI_RIGHT,
    IPGUI_IMG_SHOW_ORI_TOP_LEFT,
    IPGUI_IMG_SHOW_ORI_TOP_RIGHT,
    IPGUI_IMG_SHOW_ORI_BOTTOM_LEFT,
    IPGUI_IMG_SHOW_ORI_BOTTOM_RIGHT,

    IPGUI_IMG_SHOW_ORI_MAX_FLAG,

    /* fit show mode(keep scale and show all in dest region) */
    IPGUI_IMG_SHOW_FIT_CENTER,
    IPGUI_IMG_SHOW_FIT_TOP_LEFT,
    IPGUI_IMG_SHOW_FIT_TOP_RIGHT,
    IPGUI_IMG_SHOW_FIT_BOTTOM_LEFT,
    IPGUI_IMG_SHOW_FIT_BOTTOM_RIGHT,

    IPGUI_IMG_SHOW_FIT_MAX_FLAG,

    /* fill show mode */
    IPGUI_IMG_SHOW_FILL,/* do not keep x/y, and fill the dest region */
}ipgui_img_show_mode_t;

typedef enum {
    IPGUI_IMAGE_LERP_NEAREST,   /* 最近邻，性能优先 */
    IPGUI_IMAGE_LERP_BILINEAR,  /* 双线性，质量优先 */
} ipgui_image_lerp_t;

typedef struct {
    /* 显示模式 */
    ipgui_img_show_mode_t   show_mode;
    ipgui_image_lerp_t      lerp;
    // ipgui_composite_mode_t  composite;

    /* 全局透明度 */
    unsigned char           opacity;
} ipgui_image_show_attr_t;

#endif