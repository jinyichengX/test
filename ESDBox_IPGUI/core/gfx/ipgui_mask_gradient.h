#ifndef IPGUI_MASK_GRADIENT_H
#define IPGUI_MASK_GRADIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_prim.h"
#include "ipgui_conf.h"

#if IPGUI_GRADIENT_STOP_MAX < 2
#error "IPGUI_GRADIENT_STOP_MAX must be at least 2"
#endif

typedef enum {
    IPGUI_LINER_MASK_GRADIENT,
    IPGUI_RADIAL_MASK_GRADIENT,
}ipgui_mask_gradient_type_t;

/* 遮罩停止点 - 只包含位置和透明度值(0-255) */
typedef struct {
    unsigned char pos;      /* 停止点在渐变中的位置(0-255) */
    unsigned char value;    /* 遮罩值(0-255)，0=完全透明，255=完全不透明 */
} ipgui_mask_gradient_stop_t;

/* 线性遮罩渐变结构体 */
typedef struct {
    /* 比例模式参数(0-255) */
    int x_start;  /* 起始比例x(* 255.0f) */
    int y_start;  /* 起始比例y(* 255.0f) */
    int x_end;    /* 结束比例x(* 255.0f) */
    int y_end;    /* 结束比例y(* 255.0f) */
    
    unsigned int opacity : 16;      /* 整体不透明度影响(0-255) */
    unsigned int stop_nr : 16;      /* 停止点数量(2-IPGUI_GRADIENT_STOP_MAX) */
    ipgui_mask_gradient_stop_t stops[IPGUI_GRADIENT_STOP_MAX];
    
    /* 应用AABB后计算的绝对坐标 */
    ipgui_aabb_t aabb;               /* 应用的包围盒 */
    ipgui_coord_t x_start_abs, y_start_abs;  /* 绝对坐标起点 */
    ipgui_coord_t x_end_abs, y_end_abs;      /* 绝对坐标终点 */
    ipgui_vector_t gradient_vector;           /* 渐变向量 */
    int gradient_vec_mod_pow;                 /* 渐变向量模的平方 */
} ipgui_liner_mask_gradient_t;

/* 径向遮罩渐变结构体 */
typedef struct {
    ipgui_coord_t center_x;        /* 中心点x坐标 */
    ipgui_coord_t center_y;        /* 中心点y坐标 */
    ipgui_coord_t radius;          /* 半径 */
    ipgui_coord_t radius_pow;      /* 半径平方 */
    
    unsigned int opacity : 16;      /* 整体不透明度影响(0-255) */
    unsigned int stop_nr : 16;      /* 停止点数量(2-IPGUI_GRADIENT_STOP_MAX) */
    ipgui_mask_gradient_stop_t stops[IPGUI_GRADIENT_STOP_MAX];
} ipgui_radial_mask_gradient_t;

/* ==================== 线性遮罩渐变API ==================== */

/* 比例模式初始化：参数范围0.0-1.0 */
extern __IPGUI_API__ void ipgui_liner_mask_gradient_init(
    ipgui_liner_mask_gradient_t * gradient,
    float x_start, float y_start,
    float x_end, float y_end);

/* 直接坐标模式初始化 */
extern __IPGUI_API__ void ipgui_liner_mask_gradient_init_direct(
    ipgui_liner_mask_gradient_t * gradient,
    ipgui_coord_t x_start, ipgui_coord_t y_start,
    ipgui_coord_t x_end, ipgui_coord_t y_end);

/* 添加遮罩停止点 */
extern __IPGUI_API__ int ipgui_liner_mask_gradient_add_stop(
    ipgui_liner_mask_gradient_t * gradient,
    ipgui_mask_gradient_stop_t * stop);

/* 移除遮罩停止点 */
extern __IPGUI_API__ int ipgui_liner_mask_gradient_remove_stop(
    ipgui_liner_mask_gradient_t * gradient,
    unsigned char pos);

/* 设置整体不透明度影响 */
extern __IPGUI_API__ void ipgui_liner_mask_gradient_set_opacity(
    ipgui_liner_mask_gradient_t * gradient,
    unsigned char opacity);

/* 应用渐变到指定AABB（比例模式用） */
extern __IPGUI_API__ void ipgui_liner_mask_gradient_apply_to_aabb(
    ipgui_liner_mask_gradient_t * gradient,
    ipgui_aabb_t * aabb);

/* 获取指定位置的遮罩值 */
extern __IPGUI_API__ unsigned char ipgui_liner_mask_gradient_value_get(
    ipgui_liner_mask_gradient_t * gradient,
    unsigned char pos);

/* 获取指定坐标在渐变中的位置(0-255) */
extern __IPGUI_API__ unsigned char ipgui_get_liner_mask_gradient_pos_at_xy(
    ipgui_liner_mask_gradient_t * gradient,
    ipgui_coord_t x, ipgui_coord_t y);

/* 便捷函数：直接根据坐标获取遮罩值 */
extern __IPGUI_API__ unsigned char ipgui_liner_mask_gradient_value_at_xy(
    ipgui_liner_mask_gradient_t * gradient,
    ipgui_coord_t x, ipgui_coord_t y);

/* ==================== 径向遮罩渐变API ==================== */

/* 径向遮罩初始化 */
extern __IPGUI_API__ void ipgui_radial_mask_gradient_init(
    ipgui_radial_mask_gradient_t * gradient,
    ipgui_coord_t center_x, ipgui_coord_t center_y,
    ipgui_coord_t radius);

/* 添加遮罩停止点 */
extern __IPGUI_API__ int ipgui_radial_mask_gradient_add_stop(
    ipgui_radial_mask_gradient_t * gradient,
    ipgui_mask_gradient_stop_t * stop);

/* 移除遮罩停止点 */
extern __IPGUI_API__ int ipgui_radial_mask_gradient_remove_stop(
    ipgui_radial_mask_gradient_t * gradient,
    unsigned char pos);

/* 设置整体不透明度影响 */
extern __IPGUI_API__ void ipgui_radial_mask_gradient_set_opacity(
    ipgui_radial_mask_gradient_t * gradient,
    unsigned char opacity);

/* 获取指定位置的遮罩值 */
extern __IPGUI_API__ unsigned char ipgui_radial_mask_gradient_value_get(
    ipgui_radial_mask_gradient_t * gradient,
    unsigned char pos);

/* 获取指定坐标在渐变中的位置(0-255) */
extern __IPGUI_API__ unsigned char ipgui_get_radial_mask_gradient_pos_at_xy(
    ipgui_radial_mask_gradient_t * gradient,
    ipgui_coord_t x, ipgui_coord_t y);

/* 便捷函数：直接根据坐标获取遮罩值 */
extern __IPGUI_API__ unsigned char ipgui_radial_mask_gradient_value_at_xy(
    ipgui_radial_mask_gradient_t * gradient,
    ipgui_coord_t x, ipgui_coord_t y);

/* ==================== 公用工具 ==================== */

#if IPGUI_MASK_GRADIENT_LUT_EN == 1
/* 初始化查找表（如果需要） */
extern __IPGUI_API__ void ipgui_mask_gradient_lut_init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* IPGUI_MASK_GRADIENT_H */