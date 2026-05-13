#ifndef IPGUI_TRANSFORM_H
#define IPGUI_TRANSFORM_H

#include "ipgui_utils.h"
#include "ipgui_coord.h"

IPGUI_HEADER_BEGIN

typedef struct ipgui_matrix_context
{
    float a;    //index : [1][1]
    float b;    //index : [1][2]
    float c;    //index : [2][1]
    float d;    //index : [2][2]
    float tx;   //index : [1][3]
    float ty;   //index : [2][3]
}ipgui_matrix_t;

typedef enum 
{
    IPGUI_TRANSFORM_TRANSLATE,      /* 平移变换 */
    IPGUI_TRANSFORM_SCALE,          /* 缩放变换 */
    IPGUI_TRANSFORM_ROTATE,         /* 旋转变换 */
    IPGUI_TRANSFORM_SHEAR,          /* 错切变换 */
    IPGUI_TRANSFORM_REFLECTION,     /* 反射变换 */
    IPGUI_TRANSFORM_COMPOSE,        /* 复合变换 */
}ipgui_trans_e;

extern __IPGUI_API__ void ipgui_2d_scale_generate(ipgui_matrix_t * res, float sx, float sy);

IPGUI_HEADER_END

#endif