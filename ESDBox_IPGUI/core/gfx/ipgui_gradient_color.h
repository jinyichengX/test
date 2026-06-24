#ifndef IPGUI_GRADIENT_COLOR_H
#define IPGUI_GRADIENT_COLOR_H

#include "ipgui_color.h"
#include "ipgui_prim.h"
#include "ipgui_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

#if IPGUI_GRADIENT_STOP_MAX < 2
#error "IPGUI_GRADIENT_STOP_MAX must be at least 2"
#endif

typedef enum {
    IPGUI_GRADIENT_TYPE_LINEAR = 0,
    IPGUI_GRADIENT_TYPE_RADIAL = 1,
    IPGUI_GRADIENT_TYPE_CONIC  = 2,
    IPGUI_GRADIENT_TYPE_MAX,
} ipgui_gradient_type_t;

typedef struct {
    ipgui_color_t color;
    u8_t pos; /* offset */ /* 停止点在渐变起始位置到结束位置的占比0-255，例：0表示在(x_start, y_start)，255表示在(x_end, y_end) */ 
}ipgui_gradient_color_stop_t;

/* liner gradient struct */
typedef struct {
    /* 这四个参数决定了渐变在aabb中的方向（比例） */
    /* 从(0,0)到(255,255)相当于从包围盒左上角到右下角(0,0)-->(1,1),
     * 再举例：从(0,0)到(0,255)相当于包围盒垂直渐变(0,0)-->(0,1) 
     * 从(255,0)到(255,255)相当于包围盒垂直渐变(1,0)-->(1,1)与(0,0)到(0,255渐变效果一样 
     */
    s32_t x_start;  /* 比例模式下 起始比例x(* 255.0f) */ /* 直接模式下起始x */
    s32_t y_start;  /* 比例模式下 起始比例y(* 255.0f) */ /* 直接模式下起始y */
    s32_t x_end;    /* 比例模式下 结束比例x(* 255.0f) */ /* 直接模式下结束x */
    s32_t y_end;    /* 比例模式下 结束比例y(* 255.0f) */ /* 直接模式下结束y */

    /* 下面的参数根据渐变所应用的包围盒（比例模式）实时更新 */
    ipgui_aabb_t aabb;    /* 需要渐变的包围盒 */
    ipgui_coord_t x_start_abs, y_start_abs;  /* 直接模式和比例模式共用 */ /* 需要渐变的包围盒的起始渐变点(x_start/255 * aabb_w + aabb.start.x) */
    ipgui_coord_t x_end_abs, y_end_abs; /* 这里的abs不是绝对值，是绝对坐标 */
    /* 渐变起点->终点的向量和模，直接模式和比例模式共用 */
    ipgui_vector_t gradient_vector;
    s32_t gradient_vec_mod_pow; /* 渐变向量（绝对值）的模的平方 */

    ipgui_gradient_color_stop_t stops[IPGUI_GRADIENT_STOP_MAX];
    u32_t stop_nr : 16;  /* must from 2-IPGUI_GRADIENT_STOP_MAX */
}ipgui_liner_gradient_color_t;

/* radial gradient（径向渐变） struct */
typedef struct {
    ipgui_coord_t center_x; /* 径向渐变中心x坐标 */
    ipgui_coord_t center_y; /* 径向渐变中心y坐标 */
    ipgui_coord_t radius;   /* 渐变半径 */
    ipgui_coord_t radius_pow;   /* 渐变半径的平方 */

    ipgui_gradient_color_stop_t stops[IPGUI_GRADIENT_STOP_MAX];
    u32_t stop_nr : 16;  /* must from 2-IPGUI_GRADIENT_STOP_MAX */
}ipgui_radial_gradient_color_t;

typedef struct {
    ipgui_coord_t center_x;  /* 锥形渐变中心x坐标 */
    ipgui_coord_t center_y;  /* 锥形渐变中心y坐标 */
    u32_t angle_start : 16;       /* 渐变起始角度（初始化时被归一化到0-360），angle_start是角度制 不是弧度制 */
    //s32_t end_angle; 不需要结束角度，从起始角度逆时针（笛卡尔坐标系）渐变360度，对于屏幕坐标系就是顺时针渐变
    u32_t stop_nr : 16;      /* must from 2-IPGUI_GRADIENT_STOP_MAX */
    ipgui_gradient_color_stop_t stops[IPGUI_GRADIENT_STOP_MAX];
} ipgui_conic_gradient_color_t;

/* 如果是水平线性渐变，返回1，否则0 */
__IPGUI_STATIC__ __IPGUI_INLINE__ s32_t ipgui_if_liner_gradient_hor(ipgui_liner_gradient_color_t * gradient)
{
    return gradient->y_start_abs == gradient->y_end_abs;
}

/* 如果是垂直线性渐变，返回1，否则0 */
__IPGUI_STATIC__ __IPGUI_INLINE__ s32_t ipgui_if_liner_gradient_ver(ipgui_liner_gradient_color_t * gradient)
{
    return gradient->x_start_abs == gradient->x_end_abs;
}

/* 线性渐变 */
extern __IPGUI_API__ void ipgui_liner_gradient_init(ipgui_liner_gradient_color_t * gradient, float x_start, float y_start, float x_end, float y_end);

extern __IPGUI_API__ void ipgui_liner_gradient_init_direct(ipgui_liner_gradient_color_t * gradient, ipgui_coord_t x_start, ipgui_coord_t y_start, /* 渐变起点，绝对坐标 */ipgui_coord_t x_end, ipgui_coord_t y_end); /* 渐变终点，绝对坐标 */

extern __IPGUI_API__ s32_t ipgui_liner_gradient_add_stop(ipgui_liner_gradient_color_t * gradient, ipgui_gradient_color_stop_t * stop);

extern __IPGUI_API__ s32_t ipgui_liner_gradient_remove_stop(ipgui_liner_gradient_color_t * gradient, u8_t pos);

extern __IPGUI_API__ void ipgui_liner_gradient_apply_to_aabb(ipgui_liner_gradient_color_t * gradient, ipgui_aabb_t * aabb);

extern __IPGUI_API__ void ipgui_liner_gradient_color_get(ipgui_liner_gradient_color_t * gradient, u8_t pos, ipgui_color_t * res);

extern __IPGUI_API__ u8_t ipgui_get_liner_gradient_pos_at_xy(ipgui_liner_gradient_color_t * gradient, ipgui_coord_t x, ipgui_coord_t y);

/* 径向渐变 */
extern __IPGUI_API__ void ipgui_radial_gradient_init(ipgui_radial_gradient_color_t * gradient, ipgui_coord_t center_x, ipgui_coord_t center_y, ipgui_coord_t radius);

extern __IPGUI_API__ s32_t ipgui_radial_gradient_add_stop(ipgui_radial_gradient_color_t * gradient, ipgui_gradient_color_stop_t * stop);

extern __IPGUI_API__ s32_t ipgui_radial_gradient_remove_stop(ipgui_radial_gradient_color_t * gradient, u8_t pos);

extern __IPGUI_API__ void ipgui_radial_gradient_color_get(ipgui_radial_gradient_color_t * gradient, u8_t pos, ipgui_color_t * res);

extern __IPGUI_API__ u8_t ipgui_get_radial_gradient_pos_at_xy(ipgui_radial_gradient_color_t * gradient, ipgui_coord_t x, ipgui_coord_t y);

/* 锥形（角度）渐变 */
extern __IPGUI_API__ void ipgui_conic_gradient_init(ipgui_conic_gradient_color_t * gradient, ipgui_coord_t center_x, ipgui_coord_t center_y, s32_t angle_start);

extern __IPGUI_API__ s32_t ipgui_conic_gradient_add_stop(ipgui_conic_gradient_color_t * gradient, ipgui_gradient_color_stop_t * stop);

extern __IPGUI_API__ s32_t ipgui_conic_gradient_remove_stop(ipgui_conic_gradient_color_t * gradient, u8_t pos);

extern __IPGUI_API__ void ipgui_conic_gradient_color_get(ipgui_conic_gradient_color_t * gradient, u8_t pos, ipgui_color_t * res);

extern __IPGUI_API__ u8_t ipgui_get_conic_gradient_pos_at_xy(ipgui_conic_gradient_color_t * gradient, ipgui_coord_t x, ipgui_coord_t y);

#if IPGUI_GRADIENT_LUT_EN == 1
extern __IPGUI_API__ void ipgui_gradient_lut_init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif