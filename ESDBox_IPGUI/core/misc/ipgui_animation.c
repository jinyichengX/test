/**
 * @file    ipgui_animation.c
 * @brief   动画模块实现 — 嵌入式GUI系统通用动画引擎
 *
 * ## 实现概述
 * 提供完整的UI动画支持，包括淡入淡出、位移、缩放、旋转、颜色渐变等效果。
 * 采用纯整数运算，支持定点数插值，适用于无FPU的嵌入式平台。
 *
 * ## 核心设计
 *   - 静态内存池：避免动态分配，支持中断上下文安全调用
 *   - 链表管理：活跃动画和空闲动画分离，高效调度
 *   - 位图ID分配：O(1)时间复杂度的ID管理
 *   - 定点数插值：避免浮点运算，支持多种数据类型
 */

#include "ipgui_animation.h"
#include "ipgui_timer.h"
#include "ipgui_memory.h"

/*============================================================================
 * 宏定义
 *===========================================================================*/

/** 调试开关 */
#define IPGUI_ANIM_DEBUG             0

/** 断言开关 */
#define IPGUI_ANIM_ASSERT            1

#if IPGUI_ANIM_ASSERT
#define IPGUI_ANIM_ASSERT_PTR(ptr)   do { if(!(ptr)) return IPGUI_ERR_PARAM; } while(0)
#define IPGUI_ANIM_ASSERT_HANDLE(h)  do { if(!(ipgui_anim_is_valid_handle(h))) return IPGUI_ERR_PARAM; } while(0)
#else
#define IPGUI_ANIM_ASSERT_PTR(ptr)   ((void)0)
#define IPGUI_ANIM_ASSERT_HANDLE(h)  ((void)0)
#endif

#if IPGUI_ANIM_DEBUG
#define IPGUI_ANIM_TRACE(...)        do { /* printf("[ANIM] "__VA_ARGS__); */ } while(0)
#else
#define IPGUI_ANIM_TRACE(...)        ((void)0)
#endif

/** 定点数位数 */
#define IPGUI_ANIM_FP_BITS           16
#define IPGUI_ANIM_FP_ONE            (1 << IPGUI_ANIM_FP_BITS)
#define IPGUI_ANIM_FP_MASK           ((1 << IPGUI_ANIM_FP_BITS) - 1)

/** 定点数乘法（使用s64中间结果避免溢出） */
#define IPGUI_ANIM_FP_MUL(a, b)      ((s32_t)(((s64_t)(a) * (s64_t)(b)) >> IPGUI_ANIM_FP_BITS))

/*============================================================================
 * 缓动函数内部实现
 *
 * 所有函数返回定点数值，使用s32_t类型
 * 输入参数：t=当前时间, b=起始值, c=变化量, d=总时长
 *
 * 数学约定：
 *   - ease_in:  加速开始，减速结束
 *   - ease_out: 减速开始，加速结束
 *   - ease_in_out: 前半段ease_in，后半段ease_out
 *
 * 关键修正（2026-06）：使用s64中间类型防止溢出；
 *   ease_out 统一使用 complement 公式确保数学正确性；
 *   ease_in_out 使用s64直接计算正确公式。
 *===========================================================================*/

/* ---- 前向声明（用于 complement 公式） ---- */
__IPGUI_STATIC__ s32_t ease_in_quad(s32_t t, s32_t b, s32_t c, s32_t d);
__IPGUI_STATIC__ s32_t ease_in_cubic(s32_t t, s32_t b, s32_t c, s32_t d);
__IPGUI_STATIC__ s32_t ease_in_quart(s32_t t, s32_t b, s32_t c, s32_t d);
__IPGUI_STATIC__ s32_t ease_in_sine(s32_t t, s32_t b, s32_t c, s32_t d);
__IPGUI_STATIC__ s32_t ease_in_expo(s32_t t, s32_t b, s32_t c, s32_t d);
__IPGUI_STATIC__ s32_t ease_in_back(s32_t t, s32_t b, s32_t c, s32_t d);

/**
 * 线性插值
 */
__IPGUI_STATIC__ s32_t ease_linear(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    return b + (s32_t)((s64_t)c * t / d);
}

/**
 * 二次方缓入: f(t) = (t/d)^2
 */
__IPGUI_STATIC__ s32_t ease_in_quad(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    s64_t t64 = t;
    s64_t d64 = d;
    return b + (s32_t)((s64_t)c * t64 * t64 / (d64 * d64));
}

/**
 * 二次方缓出: f(t) = 1 - (1 - t/d)^2 = complement of ease_in_quad
 */
__IPGUI_STATIC__ s32_t ease_out_quad(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    return b + c - ease_in_quad(d - t, 0, c, d);
}

/**
 * 二次方缓入缓出
 */
__IPGUI_STATIC__ s32_t ease_in_out_quad(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    s64_t t64 = t;
    s64_t d64 = d;
    if (t * 2 < d) {
        /* 前半段: c/2 * 2*(t/(d/2))^2 = c * (2t/d)^2 */
        s64_t num = (s64_t)c * t64 * t64 * 2;
        return b + (s32_t)(num / (d64 * d64));
    } else {
        /* 后半段: c/2 + c/2 * (1 - 2*(1 - t/d)^2) */
        s64_t dt = d64 - t64;
        s64_t num = (s64_t)c * (d64 * d64 - 2 * dt * dt);
        return (s32_t)(b + num / (d64 * d64));
    }
}

/**
 * 三次方缓入: f(t) = (t/d)^3
 */
__IPGUI_STATIC__ s32_t ease_in_cubic(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    s64_t t64 = t;
    s64_t d64 = d;
    return b + (s32_t)((s64_t)c * t64 * t64 * t64 / (d64 * d64 * d64));
}

/**
 * 三次方缓出: f(t) = 1 - (1 - t/d)^3
 */
__IPGUI_STATIC__ s32_t ease_out_cubic(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    return b + c - ease_in_cubic(d - t, 0, c, d);
}

/**
 * 三次方缓入缓出
 */
__IPGUI_STATIC__ s32_t ease_in_out_cubic(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    s64_t t64 = t;
    s64_t d64 = d;
    if (t * 2 < d) {
        /* 前半段: c * 4*(t/d)^3 */
        s64_t num = (s64_t)c * 4 * t64 * t64 * t64;
        return b + (s32_t)(num / (d64 * d64 * d64));
    } else {
        /* 后半段: c - c * 4*(1 - t/d)^3 */
        s64_t dt = d64 - t64;
        s64_t num = (s64_t)c * (d64 * d64 * d64 - 4 * dt * dt * dt);
        return (s32_t)(b + num / (d64 * d64 * d64));
    }
}

/**
 * 四次方缓入: f(t) = (t/d)^4
 */
__IPGUI_STATIC__ s32_t ease_in_quart(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    s64_t t64 = t;
    s64_t d64 = d;
    s64_t t2 = t64 * t64;
    return b + (s32_t)((s64_t)c * t2 * t2 / (d64 * d64 * d64 * d64));
}

/**
 * 四次方缓出: f(t) = 1 - (1 - t/d)^4
 */
__IPGUI_STATIC__ s32_t ease_out_quart(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    return b + c - ease_in_quart(d - t, 0, c, d);
}

/**
 * 四次方缓入缓出
 */
__IPGUI_STATIC__ s32_t ease_in_out_quart(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    s64_t t64 = t;
    s64_t d64 = d;
    s64_t t2 = t64 * t64;
    if (t * 2 < d) {
        /* 前半段: c * 8*(t/d)^4 */
        s64_t num = (s64_t)c * 8 * t2 * t2;
        return b + (s32_t)(num / (d64 * d64 * d64 * d64));
    } else {
        /* 后半段 */
        s64_t dt = d64 - t64;
        s64_t dt2 = dt * dt;
        s64_t num = (s64_t)c * (d64 * d64 * d64 * d64 - 8 * dt2 * dt2);
        return (s32_t)(b + num / (d64 * d64 * d64 * d64));
    }
}

/**
 * 正弦缓入: f(t) = 1 - cos(PI/2 * t/d)
 * 使用麦克劳林级数近似: cos(x) ≈ 1 - x^2/2，所以 1-cos(x) ≈ x^2/2
 * 但更精确的近似: sin(PI/2 * x) ≈ 1.57*x - 0.647*x^3 + ...
 *
 * 采用定标整数实现: 令 ratio = 10000 * t / d (定点数0.0001单位)
 * 则 x = ratio * PI/20000 ≈ ratio * 15708/10000000
 * 1 - cos(PI/2 * t/d) ≈ (PI/2 * t/d)^2 / 2 = PI^2 * t^2 / (8 * d^2)
 * PI^2/8 ≈ 1.2337
 *
 * 简化实现: 使用归一化后的二次逼近，适用于嵌入式环境
 */
__IPGUI_STATIC__ s32_t ease_in_sine(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    /*
     * 使用三阶近似: sin(PI/2 * x) ≈ x * (1 + (1-x) * x * 0.233)
     * 其中 x = t/d. 转换为整数运算。
     * phase = (t/d) * (1 + (1 - t/d) * (t/d) * 0.233)
     * = (t/d) + 0.233 * (1 - t/d) * (t/d)^2
     * 0.233 ≈ 15275 / 65536 (FP16近似)
     */
    s64_t d64 = d;
    s64_t t64 = t;
    /* term1 = (t/d)^2 * (d - t)/d * 15275 / 65536 */
    /* = t^2 * (d - t) * 15275 / (d^3 * 65536) */
    s64_t num = (s64_t)c * t64 * t64 * (d64 - t64) * 15275;
    s64_t den = d64 * d64 * d64 * 65536;
    s64_t phase_extra = num / den;
    /* base = c * t / d */
    s64_t base = (s64_t)c * t64 / d64;
    return (s32_t)(b + base + phase_extra);
}

/**
 * 正弦缓出: f(t) = sin(PI/2 * t/d)
 */
__IPGUI_STATIC__ s32_t ease_out_sine(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    return b + c - ease_in_sine(d - t, 0, c, d);
}

/**
 * 正弦缓入缓出
 */
__IPGUI_STATIC__ s32_t ease_in_out_sine(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    if (t * 2 < d) {
        return b + ease_in_sine(t * 2, 0, c / 2, d) * 2;
    } else {
        s32_t phase = ease_out_sine(t * 2 - d, 0, c / 2, d);
        return b + c / 2 + phase;
    }
}

/**
 * 指数缓入: f(t) ≈ 2^(10*(t/d - 1))
 *
 * 使用整数近似: 当 t=0 时 = 0, 当 t=d 时 = 1
 * 实现为逐段线性+二次组合逼近
 */
__IPGUI_STATIC__ s32_t ease_in_expo(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    if (t == 0) return b;
    /* expo: 2^(10*(t/d - 1)) 使用归一化后再二次缩放 */
    s64_t d64 = d;
    s64_t t64 = t;
    /*
     * 近似: 令 x = t/d, 2^(10*(x-1)) ≈ (x*x + x) / 2 当 x 接近 1
     * 但更好的方式: 使用分段线性
     * x = t/d, result ≈ x^10 (简化但风格类似)
     * 实际采用: x^2 * x^2 * x^2 * x^2 * x^2 = x^10
     * 连续乘5次平方项，但用s64防止溢出
     */
    s64_t phase = t64 * t64 / d64;        /* x^2 * d */
    phase = phase * t64 / d64;              /* x^3 * d */
    phase = phase * t64 / d64;              /* x^4 * d */
    phase = phase * t64 / d64;              /* x^5 * d */
    phase = phase * phase / d64;            /* x^10 * d */
    return b + (s32_t)((s64_t)c * phase / d64);
}

/**
 * 指数缓出: f(t) = 1 - 2^(-10*t/d)
 */
__IPGUI_STATIC__ s32_t ease_out_expo(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    if (t == d) return b + c;
    return b + c - ease_in_expo(d - t, 0, c, d);
}

/**
 * 指数缓入缓出
 */
__IPGUI_STATIC__ s32_t ease_in_out_expo(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    if (t == 0) return b;
    if (t == d) return b + c;
    if (t * 2 < d) {
        return b + ease_in_expo(t * 2, 0, c / 2, d) * 2;
    } else {
        s32_t phase = ease_out_expo(t * 2 - d, 0, c / 2, d);
        return b + c / 2 + phase;
    }
}

/**
 * 回退缓入 (带overshoot效果)
 * f(t) = x^2 * ((s+1)*x - s)  where x = t/d, s = 1.70158
 *
 * s = 1.70158 的 FP16.16 表示为 111515 (0x1B39B)
 * 近似: s * 65536 ≈ 111523, 使用 111523
 */
#define IPGUI_ANIM_BACK_S   111523  /* 1.70158 in FP16.16 */

__IPGUI_STATIC__ s32_t ease_in_back(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    s64_t d64 = d;
    s64_t t64 = t;
    /* phase = (t/d)^2 * ((s+1)*(t/d) - s) */
    s64_t s1 = IPGUI_ANIM_BACK_S + (1 << IPGUI_ANIM_FP_BITS);  /* s + 1 */
    s64_t term = s1 * t64 / d64 - (s64_t)IPGUI_ANIM_BACK_S;
    s64_t phase = t64 * t64 * term / (d64 * d64);
    return b + (s32_t)((s64_t)c * phase / d64);
}

#undef IPGUI_ANIM_BACK_S

/**
 * 回退缓出: complement of ease_in_back
 */
__IPGUI_STATIC__ s32_t ease_out_back(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    return b + c - ease_in_back(d - t, 0, c, d);
}

/**
 * 回退缓入缓出
 * 使用 s = 1.70158 * 1.525 ≈ 2.59491
 */
#define IPGUI_ANIM_BACK_S_INOUT  170080  /* 2.59491 in FP16.16 */

__IPGUI_STATIC__ s32_t ease_in_out_back(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    s64_t d64 = d;
    s64_t t64 = t;
    if (t * 2 < d) {
        /* 前半段: ease_in_back with s = 2.59491, scaled to [0, c/2] */
        s64_t s_val = IPGUI_ANIM_BACK_S_INOUT;
        s64_t s1 = s_val + (1 << IPGUI_ANIM_FP_BITS);
        s64_t x = t64 * 2;  /* scale t to [0, d] for ease_in */
        s64_t term = s1 * x / d64 - s_val;
        s64_t phase = x * x * term / (d64 * d64);
        return b + (s32_t)((s64_t)c * phase / (2 * d64));
    } else {
        /* 后半段: c/2 + ease_out_back scaled */
        s64_t x = (t64 * 2 - d64);  /* scale t to [0, d] */
        s64_t phase = ease_out_back((s32_t)x, 0, c / 2, d);
        return b + c / 2 + phase;
    }
}

#undef IPGUI_ANIM_BACK_S_INOUT

/**
 * 弹跳缓出: 使用分段二次函数模拟弹性衰减
 * 将动画分为4段，每段模拟一次弹跳衰减
 */
__IPGUI_STATIC__ s32_t ease_out_bounce(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    t = IPGUI_MIN(t, d);
    s64_t d64 = d;
    s64_t t64 = t;

    /*
     * 分段系数: 1/2.75, 2/2.75, 2.5/2.75, 2.75/2.75
     * 使用整数乘除: k1=1*d/2.75 ≈ d*16384/45056, let's do proper divisions
     *
     * 简化版: 使用标准bounce公式的整数适配
     *   bounce(t) = 7.5625*t^2 when t in [0, 1/2.75]
     *   bounce(t) = 7.5625*(t-1.5/2.75)^2 + 0.75 when t in [1/2.75, 2/2.75]
     *   etc.
     * 7.5625 = 121/16, 2.75 = 11/4
     *
     * 转换为整数域:
     *   k = 11*d/4, 所以 d/k = 4/11
     *   第一个分段: t ∈ [0, d*4/11], 即 t*11 < d*4
     */
    s64_t k1 = d64 * 4 / 11;          /* d * 4/11 */
    s64_t k2 = d64 * 6 / 11;          /* d * 6/11 */
    s64_t k3 = d64 * 9 / 11;          /* d * 9/11 */
    s64_t k4 = d64 * 10 / 11;         /* d * 10/11 */

    /* 7.5625 * 65536 = 495616 (FP16.16) */
    s64_t coeff = 495616;

    if (t64 < k1) {
        /* t ∈ [0, d*4/11]: 7.5625 * (t * 11/4 / d)^2 */
        s64_t x = t64 * 11 * IPGUI_ANIM_FP_ONE / (4 * d64);
        s64_t phase = IPGUI_ANIM_FP_MUL(coeff, x) * x;
        return (s32_t)(b + (s64_t)c * phase / IPGUI_ANIM_FP_ONE);
    } else if (t64 < k2) {
        /* t ∈ [d*4/11, d*6/11]: 7.5625 * ((t - d*1.5/2.75) * 11/4 / d)^2 + 0.75 */
        t64 -= k1;
        s64_t x = t64 * 11 * IPGUI_ANIM_FP_ONE / (d64 * 2);
        s64_t phase = IPGUI_ANIM_FP_MUL(coeff, x) * x;
        return (s32_t)(b + (s64_t)c * (phase + 3 * IPGUI_ANIM_FP_ONE / 4) / IPGUI_ANIM_FP_ONE);
    } else if (t64 < k3) {
        /* t ∈ [d*6/11, d*9/11]: 7.5625 * ((t - d*2.5/2.75) * 11/4 / d)^2 + 0.9375 */
        t64 -= k2;
        s64_t x = t64 * 11 * IPGUI_ANIM_FP_ONE / (d64 * 3);
        s64_t phase = IPGUI_ANIM_FP_MUL(coeff, x) * x;
        return (s32_t)(b + (s64_t)c * (phase + 15 * IPGUI_ANIM_FP_ONE / 16) / IPGUI_ANIM_FP_ONE);
    } else {
        /* t ∈ [d*9/11, d]: 7.5625 * ((t - d*2.625/2.75) * 11/4 / d)^2 + 0.984375 */
        t64 -= k4;
        d64 = d64 - k4;
        if (d64 <= 0) return b + c;
        s64_t x = t64 * IPGUI_ANIM_FP_ONE / d64;
        s64_t phase = IPGUI_ANIM_FP_MUL(coeff, x) * x;
        return (s32_t)(b + (s64_t)c * (phase + 63 * IPGUI_ANIM_FP_ONE / 64) / IPGUI_ANIM_FP_ONE);
    }
}

/**
 * 弹跳缓入
 */
__IPGUI_STATIC__ s32_t ease_in_bounce(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    return b + c - ease_out_bounce(d - t, 0, c, d);
}

/**
 * 弹跳缓入缓出
 */
__IPGUI_STATIC__ s32_t ease_in_out_bounce(s32_t t, s32_t b, s32_t c, s32_t d)
{
    if (d == 0) return b;
    if (t * 2 < d) {
        return b + (c - ease_out_bounce(d - t * 2, 0, c, d)) / 2;
    } else {
        return b + (c + ease_out_bounce(t * 2 - d, 0, c, d)) / 2;
    }
}

/*============================================================================
 * 缓动函数表
 *===========================================================================*/

/** 缓动函数查找表 */
__IPGUI_STATIC__ const ipgui_anim_ease_func_t s_ease_func_table[IPGUI_ANIM_EASE_MAX] = {
    [IPGUI_ANIM_EASE_LINEAR]         = ease_linear,
    [IPGUI_ANIM_EASE_IN_QUAD]        = ease_in_quad,
    [IPGUI_ANIM_EASE_OUT_QUAD]       = ease_out_quad,
    [IPGUI_ANIM_EASE_IN_OUT_QUAD]    = ease_in_out_quad,
    [IPGUI_ANIM_EASE_IN_CUBIC]       = ease_in_cubic,
    [IPGUI_ANIM_EASE_OUT_CUBIC]      = ease_out_cubic,
    [IPGUI_ANIM_EASE_IN_OUT_CUBIC]   = ease_in_out_cubic,
    [IPGUI_ANIM_EASE_IN_QUART]       = ease_in_quart,
    [IPGUI_ANIM_EASE_OUT_QUART]      = ease_out_quart,
    [IPGUI_ANIM_EASE_IN_OUT_QUART]   = ease_in_out_quart,
    [IPGUI_ANIM_EASE_IN_SINE]        = ease_in_sine,
    [IPGUI_ANIM_EASE_OUT_SINE]       = ease_out_sine,
    [IPGUI_ANIM_EASE_IN_OUT_SINE]    = ease_in_out_sine,
    [IPGUI_ANIM_EASE_IN_EXPO]        = ease_in_expo,
    [IPGUI_ANIM_EASE_OUT_EXPO]       = ease_out_expo,
    [IPGUI_ANIM_EASE_IN_OUT_EXPO]    = ease_in_out_expo,
    [IPGUI_ANIM_EASE_IN_BACK]        = ease_in_back,
    [IPGUI_ANIM_EASE_OUT_BACK]       = ease_out_back,
    [IPGUI_ANIM_EASE_IN_OUT_BACK]    = ease_in_out_back,
    [IPGUI_ANIM_EASE_IN_BOUNCE]      = ease_in_bounce,
    [IPGUI_ANIM_EASE_OUT_BOUNCE]     = ease_out_bounce,
    [IPGUI_ANIM_EASE_IN_OUT_BOUNCE]  = ease_in_out_bounce,
};

/*============================================================================
 * 动画系统全局变量
 *===========================================================================*/

/** 动画系统全局句柄 */
static ipgui_anim_sys_t s_anim_sys = {
    .active_list       = NULL,
    .free_list         = NULL,
    .initialized       = 0,
    .last_update_time  = 0,
    .running_anim_count = 0,
    .global_frame_cb   = NULL,
    .global_user_data  = NULL,
};

/*============================================================================
 * 内部函数实现
 *===========================================================================*/

/**
 * @brief 初始化空闲链表
 */
__IPGUI_STATIC__ void init_free_list(void)
{
    s_anim_sys.free_list = NULL;

    /* 将所有动画节点加入空闲链表 */
    for (int i = 0; i < IPGUI_ANIM_MAX_COUNT; i++) {
        s_anim_sys.anim_pool[i].next = s_anim_sys.free_list;
        s_anim_sys.free_list = &s_anim_sys.anim_pool[i];
    }

    /* 初始化ID位图 */
    ipgui_memset(s_anim_sys.id_bitmap, 0, sizeof(s_anim_sys.id_bitmap));
}

/**
 * @brief 从空闲链表获取一个动画节点
 * @return 动画节点指针，失败返回NULL
 */
__IPGUI_STATIC__ ipgui_anim_t * alloc_anim_node(void)
{
    ipgui_anim_t * node = s_anim_sys.free_list;
    if (node) {
        s_anim_sys.free_list = node->next;
        node->next = NULL;
    }
    return node;
}

/**
 * @brief 释放动画节点回空闲链表
 * @param node 要释放的节点
 */
__IPGUI_STATIC__ void free_anim_node(ipgui_anim_t * node)
{
    if (!node) return;
    node->next = s_anim_sys.free_list;
    s_anim_sys.free_list = node;
}

/**
 * @brief 从活跃链表中移除动画
 * @param anim 要移除的动画
 * @note 仅从链表移除，不释放节点和ID
 */
__IPGUI_STATIC__ void remove_from_active_list(ipgui_anim_t * anim)
{
    if (!anim) return;

    if (s_anim_sys.active_list == anim) {
        s_anim_sys.active_list = anim->next;
    } else {
        ipgui_anim_t * prev = s_anim_sys.active_list;
        while (prev && prev->next != anim) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = anim->next;
        }
    }
    anim->next = NULL;
    s_anim_sys.running_anim_count--;
}

/**
 * @brief 添加动画到活跃链表（按优先级排序，高优先级在前）
 * @param anim 要添加的动画
 */
__IPGUI_STATIC__ void add_to_active_list(ipgui_anim_t * anim)
{
    if (!anim) return;

    /* 按优先级降序插入链表 */
    anim->next = NULL;
    if (!s_anim_sys.active_list || s_anim_sys.active_list->priority < anim->priority) {
        anim->next = s_anim_sys.active_list;
        s_anim_sys.active_list = anim;
    } else {
        ipgui_anim_t * curr = s_anim_sys.active_list;
        while (curr->next && curr->next->priority >= anim->priority) {
            curr = curr->next;
        }
        anim->next = curr->next;
        curr->next = anim;
    }
    s_anim_sys.running_anim_count++;
}

/**
 * @brief 分配新的动画ID
 * @return 分配的ID (1-based)，失败返回0
 */
__IPGUI_STATIC__ u32_t alloc_anim_id(void)
{
    for (int i = 0; i < (IPGUI_ANIM_MAX_COUNT + 31) >> 5; i++) {
        if (s_anim_sys.id_bitmap[i] != 0xFFFFFFFF) {
            for (int j = 0; j < 32; j++) {
                if (!(s_anim_sys.id_bitmap[i] & (1U << j))) {
                    s_anim_sys.id_bitmap[i] |= (1U << j);
                    return (u32_t)(i * 32 + j + 1);
                }
            }
        }
    }
    return 0;
}

/**
 * @brief 释放动画ID
 * @param id 要释放的ID (1-based)
 */
__IPGUI_STATIC__ void free_anim_id(u32_t id)
{
    if (id == 0 || id > IPGUI_ANIM_MAX_COUNT) return;
    id--;
    s_anim_sys.id_bitmap[id >> 5] &= ~(1U << (id & 31));
}

/**
 * @brief 根据句柄查找动画节点
 * @param handle 动画句柄
 * @return 动画节点指针，失败返回NULL
 */
__IPGUI_STATIC__ ipgui_anim_t * find_anim_by_handle(ipgui_anim_handle_t handle)
{
    if (handle == IPGUI_ANIM_INVALID_HANDLE) return NULL;

    u32_t id = handle;
    if (id == 0 || id > IPGUI_ANIM_MAX_COUNT) return NULL;

    /* 检查ID位图 */
    id--;
    if (!(s_anim_sys.id_bitmap[id >> 5] & (1U << (id & 31)))) {
        return NULL;
    }

    /* ID从1开始，索引从0开始 */
    return &s_anim_sys.anim_pool[id];
}

/**
 * @brief 更新动画状态并调用回调
 * @param anim     动画节点
 * @param new_state 新状态
 */
__IPGUI_STATIC__ void update_anim_state(ipgui_anim_t * anim,
                                          ipgui_anim_state_t new_state)
{
    if (!anim || anim->state == new_state) return;

    ipgui_anim_state_t old_state = anim->state;
    anim->state = new_state;

    /* 调用状态改变回调 */
    if (anim->state_cb) {
        anim->state_cb((ipgui_anim_handle_t)anim->id, old_state, new_state, anim->user_data);
    }
}

/**
 * @brief 计算并更新动画当前值
 * @param anim 动画节点
 */
__IPGUI_STATIC__ void calculate_anim_value(ipgui_anim_t * anim)
{
    if (!anim || !anim->ease_func) return;

    s32_t t = (s32_t)anim->elapsed;
    s32_t d = (s32_t)anim->duration;
    s32_t from, to, result;

    /* 根据动画类型获取正确的起始值和目标值 */
    switch (anim->type) {
        case IPGUI_ANIM_TYPE_FADE:
        case IPGUI_ANIM_TYPE_COLOR_A:
            from = (s32_t)anim->from.u8_;
            to   = (s32_t)anim->to.u8_;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.u8_ = (u8_t)IPGUI_MIN(IPGUI_MAX(result, 0), 255);
            break;

        case IPGUI_ANIM_TYPE_TRANSLATE_X:
        case IPGUI_ANIM_TYPE_WIDTH:
            from = (s32_t)anim->from.coord_;
            to   = (s32_t)anim->to.coord_;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.coord_ = (ipgui_coord_t)result;
            break;

        case IPGUI_ANIM_TYPE_TRANSLATE_Y:
        case IPGUI_ANIM_TYPE_HEIGHT:
            from = (s32_t)anim->from.coord_;
            to   = (s32_t)anim->to.coord_;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.coord_ = (ipgui_coord_t)result;
            break;

        case IPGUI_ANIM_TYPE_TRANSLATE:
            from = (s32_t)anim->from.point_.x;
            to   = (s32_t)anim->to.point_.x;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.point_.x = (ipgui_coord_t)result;

            from = (s32_t)anim->from.point_.y;
            to   = (s32_t)anim->to.point_.y;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.point_.y = (ipgui_coord_t)result;
            break;

        case IPGUI_ANIM_TYPE_SCALE_X:
        case IPGUI_ANIM_TYPE_SCALE_Y:
        case IPGUI_ANIM_TYPE_SCALE:
        case IPGUI_ANIM_TYPE_ROTATE:
            from = anim->from.scoord_;
            to   = anim->to.scoord_;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.scoord_ = result;
            break;

        case IPGUI_ANIM_TYPE_SIZE:
            from = (s32_t)anim->from.size_.w;
            to   = (s32_t)anim->to.size_.w;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.size_.w = (ipgui_coord_t)result;

            from = (s32_t)anim->from.size_.h;
            to   = (s32_t)anim->to.size_.h;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.size_.h = (ipgui_coord_t)result;
            break;

        case IPGUI_ANIM_TYPE_COLOR_RGB:
        case IPGUI_ANIM_TYPE_COLOR:
            from = (s32_t)anim->from.color_.r;
            to   = (s32_t)anim->to.color_.r;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.color_.r = (u8_t)IPGUI_MIN(IPGUI_MAX(result, 0), 255);

            from = (s32_t)anim->from.color_.g;
            to   = (s32_t)anim->to.color_.g;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.color_.g = (u8_t)IPGUI_MIN(IPGUI_MAX(result, 0), 255);

            from = (s32_t)anim->from.color_.b;
            to   = (s32_t)anim->to.color_.b;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.color_.b = (u8_t)IPGUI_MIN(IPGUI_MAX(result, 0), 255);

            if (anim->type == IPGUI_ANIM_TYPE_COLOR) {
                from = (s32_t)anim->from.color_.a;
                to   = (s32_t)anim->to.color_.a;
                result = anim->ease_func(t, from, to - from, d);
                anim->current.color_.a = (u8_t)IPGUI_MIN(IPGUI_MAX(result, 0), 255);
            }
            break;

        default:
            /* 默认按s32处理：用于GROUP、USER及未来扩展类型 */
            from   = anim->from.s32_;
            to     = anim->to.s32_;
            result = anim->ease_func(t, from, to - from, d);
            anim->current.s32_ = result;
            break;
    }

    /* 计算进度 */
    if (d > 0) {
        anim->progress = (float)anim->elapsed / (float)anim->duration;
        if (anim->progress > 1.0f) anim->progress = 1.0f;
        if (anim->progress < 0.0f) anim->progress = 0.0f;
    }

    /* 调用更新回调 */
    if (anim->update_cb) {
        anim->update_cb((ipgui_anim_handle_t)anim->id, &anim->current, anim->user_data);
    }

    /* 调用帧回调 */
    if (anim->frame_cb) {
        anim->frame_cb((ipgui_anim_handle_t)anim->id, anim->progress, anim->user_data);
    }
}

/**
 * @brief 处理动画完成逻辑（循环或销毁）
 * @param anim 动画节点
 */
__IPGUI_STATIC__ void handle_anim_completion(ipgui_anim_t * anim)
{
    if (!anim) return;

    /* 处理循环 */
    if (anim->loop_mode != IPGUI_ANIM_LOOP_NONE) {
        /* loop_count == 0 表示无限循环，否则需要检查是否已达到指定次数 */
        if (anim->loop_count == 0 || anim->current_loop < anim->loop_count) {
            /* 需要继续循环 */
            anim->current_loop++;

            if (anim->loop_mode == IPGUI_ANIM_LOOP_PING_PONG) {
                /* 乒乓模式：反转方向和值 */
                anim->direction = -anim->direction;
                ipgui_anim_value_t tmp = anim->from;
                anim->from = anim->to;
                anim->to = tmp;
            }

            /* 重置时间并从当前时刻继续 */
            anim->elapsed = 0;
            anim->start_time = ipgui_tick_get();

            return;
        }
    }

    /* 动画完成 */
    update_anim_state(anim, IPGUI_ANIM_STATE_COMPLETED);

    /* 调用完成回调 */
    if (anim->complete_cb) {
        anim->complete_cb((ipgui_anim_handle_t)anim->id, anim->user_data);
    }

    /* 保存ID后释放 */
    u32_t id = anim->id;

    /* 从活跃列表移除 */
    remove_from_active_list(anim);

    /* 释放ID */
    free_anim_id(id);

    /* 释放节点回空闲链表 */
    free_anim_node(anim);
}

/*============================================================================
 * API 函数实现
 *===========================================================================*/

/* ---- 系统级函数 ---- */

__IPGUI_API__ ipgui_err_t ipgui_anim_sys_init(const ipgui_anim_sys_config_t * config)
{
    if (s_anim_sys.initialized) {
        return IPGUI_ERR_OK; /* 已经初始化 */
    }

    /* 使用默认配置（config参数保留供未来扩展） */
    (void)config;

    /* 初始化空闲链表 */
    init_free_list();

    /* 初始化系统状态 */
    s_anim_sys.active_list       = NULL;
    s_anim_sys.running_anim_count = 0;
    s_anim_sys.last_update_time  = ipgui_tick_get();
    s_anim_sys.initialized       = 1;

    IPGUI_ANIM_TRACE("Animation system initialized, pool size: %d\n", IPGUI_ANIM_MAX_COUNT);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ void ipgui_anim_sys_deinit(void)
{
    if (!s_anim_sys.initialized) return;

    /* 停止所有动画 */
    ipgui_anim_stop_all();

    /* 重新初始化空闲链表（恢复所有节点） */
    init_free_list();

    s_anim_sys.active_list        = NULL;
    s_anim_sys.running_anim_count = 0;
    s_anim_sys.last_update_time   = 0;
    s_anim_sys.initialized        = 0;
    s_anim_sys.global_frame_cb    = NULL;
    s_anim_sys.global_user_data   = NULL;

    IPGUI_ANIM_TRACE("Animation system deinitialized\n");
}

__IPGUI_API__ ipgui_anim_sys_t * ipgui_anim_sys_get(void)
{
    return &s_anim_sys;
}

__IPGUI_API__ u32_t ipgui_anim_update_all(u32_t current_time)
{
    if (!s_anim_sys.initialized) {
        ipgui_anim_sys_init(NULL);
    }

    /* 首次调用初始化时间基准 */
    if (s_anim_sys.last_update_time == 0) {
        s_anim_sys.last_update_time = current_time;
    }

    /* 计算时间增量（防止时间回退） */
    s32_t dt = (s32_t)(current_time - s_anim_sys.last_update_time);
    if (dt < 0) dt = 0;

    s_anim_sys.last_update_time = current_time;

    /* 遍历所有活跃动画 */
    ipgui_anim_t * anim = s_anim_sys.active_list;
    ipgui_anim_t * next;

    while (anim) {
        next = anim->next;  /* 保存next指针，因为回调可能修改链表 */

        switch (anim->state) {
            case IPGUI_ANIM_STATE_IDLE:
                break;

            case IPGUI_ANIM_STATE_DELAY:
                /* 检查延迟是否结束 */
                if (dt >= (s32_t)anim->delay) {
                    anim->elapsed = 0;
                    anim->start_time = current_time;
                    update_anim_state(anim, IPGUI_ANIM_STATE_RUNNING);
                } else {
                    /* 继续等待延迟 */
                }
                break;

            case IPGUI_ANIM_STATE_RUNNING:
                {
                    /* 按速度倍率更新时间 */
                    s32_t scaled_dt = (s32_t)((float)dt * anim->speed);
                    if (scaled_dt < 0) scaled_dt = 0;
                    anim->elapsed += (u32_t)scaled_dt;

                    /* 检查是否完成 */
                    if (anim->elapsed >= anim->duration) {
                        anim->elapsed = anim->duration;
                        calculate_anim_value(anim);
                        handle_anim_completion(anim);
                    } else {
                        calculate_anim_value(anim);
                    }
                }
                break;

            case IPGUI_ANIM_STATE_PAUSED:
                break;

            case IPGUI_ANIM_STATE_COMPLETED:
            case IPGUI_ANIM_STATE_STOPPED:
                /* 这些状态不应出现在活跃链表中 */
                break;
        }

        anim = next;
    }

    /* 调用全局帧回调 */
    if (s_anim_sys.global_frame_cb) {
        s_anim_sys.global_frame_cb(IPGUI_ANIM_INVALID_HANDLE,
                                    (float)dt / 1000.0f,
                                    s_anim_sys.global_user_data);
    }

    return s_anim_sys.running_anim_count;
}

__IPGUI_API__ void ipgui_anim_pause_all(void)
{
    ipgui_anim_t * anim = s_anim_sys.active_list;
    while (anim) {
        if (anim->state == IPGUI_ANIM_STATE_RUNNING ||
            anim->state == IPGUI_ANIM_STATE_DELAY) {
            update_anim_state(anim, IPGUI_ANIM_STATE_PAUSED);
        }
        anim = anim->next;
    }
}

__IPGUI_API__ void ipgui_anim_resume_all(void)
{
    ipgui_anim_t * anim = s_anim_sys.active_list;
    while (anim) {
        if (anim->state == IPGUI_ANIM_STATE_PAUSED) {
            update_anim_state(anim, IPGUI_ANIM_STATE_RUNNING);
        }
        anim = anim->next;
    }
}

__IPGUI_API__ void ipgui_anim_stop_all(void)
{
    ipgui_anim_t * anim = s_anim_sys.active_list;
    ipgui_anim_t * next;

    while (anim) {
        next = anim->next;

        update_anim_state(anim, IPGUI_ANIM_STATE_STOPPED);
        u32_t id = anim->id;
        remove_from_active_list(anim);
        free_anim_id(id);
        free_anim_node(anim);

        anim = next;
    }
}

__IPGUI_API__ u32_t ipgui_anim_get_active_count(void)
{
    return s_anim_sys.running_anim_count;
}

/* ---- 单动画管理函数 ---- */

__IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create(const ipgui_anim_dsc_t * dsc)
{
    IPGUI_ANIM_ASSERT_PTR(dsc);

    if (!s_anim_sys.initialized) {
        ipgui_anim_sys_init(NULL);
    }

    /* 检查空闲节点 */
    ipgui_anim_t * anim = alloc_anim_node();
    if (!anim) {
        IPGUI_ANIM_TRACE("Failed to create animation: no free nodes\n");
        return IPGUI_ANIM_INVALID_HANDLE;
    }

    /* 分配ID */
    u32_t id = alloc_anim_id();
    if (id == 0) {
        free_anim_node(anim);
        IPGUI_ANIM_TRACE("Failed to create animation: no free IDs\n");
        return IPGUI_ANIM_INVALID_HANDLE;
    }

    /* 初始化动画节点 */
    ipgui_memset(anim, 0, sizeof(ipgui_anim_t));

    anim->id          = id;
    anim->type        = dsc->type;
    anim->state       = IPGUI_ANIM_STATE_IDLE;
    anim->from        = dsc->from;
    anim->to          = dsc->to;
    anim->duration    = IPGUI_MAX(dsc->duration, IPGUI_ANIM_MIN_DURATION);
    anim->delay       = dsc->delay;
    anim->ease        = dsc->ease;
    anim->ease_func   = s_ease_func_table[dsc->ease];
    anim->loop_mode   = dsc->loop_mode;
    anim->loop_count  = dsc->loop_count;
    anim->current_loop = 0;
    anim->direction   = 1;
    anim->speed       = 1.0f;
    anim->update_cb   = dsc->update_cb;
    anim->complete_cb = dsc->complete_cb;
    anim->state_cb    = dsc->state_cb;
    anim->frame_cb    = dsc->frame_cb;
    anim->user_data   = dsc->user_data;
    anim->priority    = dsc->priority;
    anim->elapsed     = 0;
    anim->progress    = 0.0f;
    anim->next        = NULL;

    IPGUI_ANIM_TRACE("Animation created: id=%u, type=0x%x, duration=%ums\n",
                      id, dsc->type, anim->duration);

    return (ipgui_anim_handle_t)id;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_destroy(ipgui_anim_handle_t handle)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);

    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return IPGUI_ERR_PARAM;
    }

    /* 从活跃列表移除 */
    remove_from_active_list(anim);

    /* 释放ID */
    free_anim_id(handle);

    /* 释放节点 */
    free_anim_node(anim);

    IPGUI_ANIM_TRACE("Animation destroyed: id=%u\n", handle);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_start(ipgui_anim_handle_t handle)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);

    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return IPGUI_ERR_PARAM;
    }

    if (anim->state == IPGUI_ANIM_STATE_RUNNING ||
        anim->state == IPGUI_ANIM_STATE_DELAY) {
        return IPGUI_ERR_OK; /* 已经在运行或等待延迟 */
    }

    /* 重置时间和循环计数 */
    anim->elapsed = 0;
    anim->current_loop = 0;
    anim->direction = 1;
    anim->start_time = ipgui_tick_get();

    /* 如果有延迟，先进入延迟状态 */
    if (anim->delay > 0) {
        update_anim_state(anim, IPGUI_ANIM_STATE_DELAY);
    } else {
        update_anim_state(anim, IPGUI_ANIM_STATE_RUNNING);
    }

    /* 添加到活跃列表 */
    add_to_active_list(anim);

    IPGUI_ANIM_TRACE("Animation started: id=%u\n", handle);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_pause(ipgui_anim_handle_t handle)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);

    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return IPGUI_ERR_PARAM;
    }

    if (anim->state != IPGUI_ANIM_STATE_RUNNING &&
        anim->state != IPGUI_ANIM_STATE_DELAY) {
        return IPGUI_ERR_LOGIC;
    }

    update_anim_state(anim, IPGUI_ANIM_STATE_PAUSED);

    IPGUI_ANIM_TRACE("Animation paused: id=%u\n", handle);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_stop(ipgui_anim_handle_t handle)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);

    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return IPGUI_ERR_PARAM;
    }

    update_anim_state(anim, IPGUI_ANIM_STATE_STOPPED);

    /* 从活跃列表移除 */
    remove_from_active_list(anim);

    /* 释放ID */
    free_anim_id(handle);

    /* 释放节点 */
    free_anim_node(anim);

    IPGUI_ANIM_TRACE("Animation stopped: id=%u\n", handle);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_restart(ipgui_anim_handle_t handle)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);

    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return IPGUI_ERR_PARAM;
    }

    /*
     * 重启策略：保持同一节点和同一ID不变。
     * 仅从活跃列表移除后重置状态，再重新加入。
     * 避免释放后重新分配导致的节点/ID错乱问题。
     */

    /* 从活跃列表移除但不释放资源和ID */
    remove_from_active_list(anim);

    /* 重置时间和循环计数 */
    anim->elapsed       = 0;
    anim->current_loop  = 0;
    anim->start_time    = ipgui_tick_get();
    anim->direction     = 1;
    anim->progress      = 0.0f;

    /* 如果有延迟，先进入延迟状态 */
    if (anim->delay > 0) {
        update_anim_state(anim, IPGUI_ANIM_STATE_DELAY);
    } else {
        update_anim_state(anim, IPGUI_ANIM_STATE_RUNNING);
    }

    /* 重新添加到活跃列表 */
    add_to_active_list(anim);

    IPGUI_ANIM_TRACE("Animation restarted: id=%u\n", handle);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_reverse(ipgui_anim_handle_t handle)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);

    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return IPGUI_ERR_PARAM;
    }

    /* 交换起始值和目标值 */
    ipgui_anim_value_t tmp = anim->from;
    anim->from = anim->to;
    anim->to = tmp;

    /* 反转方向 */
    anim->direction = -anim->direction;

    /* 重置时间 */
    anim->elapsed = 0;
    anim->start_time = ipgui_tick_get();

    IPGUI_ANIM_TRACE("Animation reversed: id=%u\n", handle);

    return IPGUI_ERR_OK;
}

/* ---- 动画属性查询/设置 ---- */

__IPGUI_API__ ipgui_anim_state_t ipgui_anim_get_state(ipgui_anim_handle_t handle)
{
    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return IPGUI_ANIM_STATE_IDLE;
    }

    return anim->state;
}

__IPGUI_API__ float ipgui_anim_get_progress(ipgui_anim_handle_t handle)
{
    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return 0.0f;
    }

    return anim->progress;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_set_speed(ipgui_anim_handle_t handle, float speed)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);

    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return IPGUI_ERR_PARAM;
    }

    if (speed < 0.0f) {
        speed = 0.0f;
    }

    anim->speed = speed;

    IPGUI_ANIM_TRACE("Animation speed set: id=%u, speed=%.2f\n", handle, speed);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ float ipgui_anim_get_speed(ipgui_anim_handle_t handle)
{
    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return 1.0f;
    }

    return anim->speed;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_set_to(ipgui_anim_handle_t handle, ipgui_anim_value_t to)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);

    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return IPGUI_ERR_PARAM;
    }

    anim->to = to;

    IPGUI_ANIM_TRACE("Animation target set: id=%u\n", handle);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_get_current_value(ipgui_anim_handle_t handle,
                                                         ipgui_anim_value_t * value)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);
    IPGUI_ANIM_ASSERT_PTR(value);

    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) {
        return IPGUI_ERR_PARAM;
    }

    *value = anim->current;

    return IPGUI_ERR_OK;
}

/* ---- 回调函数管理 ---- */

__IPGUI_API__ ipgui_err_t ipgui_anim_set_update_cb(ipgui_anim_handle_t handle,
                                                    ipgui_anim_update_cb_t cb)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);
    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) return IPGUI_ERR_PARAM;
    anim->update_cb = cb;
    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_set_complete_cb(ipgui_anim_handle_t handle,
                                                      ipgui_anim_complete_cb_t cb)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);
    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) return IPGUI_ERR_PARAM;
    anim->complete_cb = cb;
    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_set_state_change_cb(ipgui_anim_handle_t handle,
                                                         ipgui_anim_state_change_cb_t cb)
{
    IPGUI_ANIM_ASSERT_HANDLE(handle);
    ipgui_anim_t * anim = find_anim_by_handle(handle);
    if (!anim) return IPGUI_ERR_PARAM;
    anim->state_cb = cb;
    return IPGUI_ERR_OK;
}

/* ---- 工具函数 ---- */

__IPGUI_API__ s32_t ipgui_anim_interpolate(s32_t from, s32_t to,
                                            ipgui_anim_ease_t ease,
                                            s32_t t, s32_t d)
{
    if (ease >= IPGUI_ANIM_EASE_MAX) {
        ease = IPGUI_ANIM_EASE_LINEAR;
    }

    ipgui_anim_ease_func_t func = s_ease_func_table[ease];
    if (!func) {
        return ease_linear(t, from, to - from, d);
    }

    return func(t, from, to - from, d);
}

__IPGUI_API__ ipgui_anim_ease_func_t ipgui_anim_get_ease_func(ipgui_anim_ease_t ease)
{
    if (ease >= IPGUI_ANIM_EASE_MAX) {
        return ease_linear;
    }
    return s_ease_func_table[ease];
}

__IPGUI_API__ int ipgui_anim_is_valid_handle(ipgui_anim_handle_t handle)
{
    return (find_anim_by_handle(handle) != NULL) ? 1 : 0;
}

/* ---- 便捷创建函数 ---- */

__IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create_fade_in(
    u32_t duration,
    ipgui_anim_update_cb_t cb,
    ipgui_anim_complete_cb_t complete,
    void * user_data)
{
    ipgui_anim_dsc_t dsc = {
        .type        = IPGUI_ANIM_TYPE_FADE,
        .from.u8_    = 0,
        .to.u8_      = 255,
        .duration    = duration,
        .delay       = 0,
        .ease        = IPGUI_ANIM_EASE_OUT_QUAD,
        .loop_mode   = IPGUI_ANIM_LOOP_NONE,
        .loop_count  = 0,
        .update_cb   = cb,
        .complete_cb = complete,
        .user_data   = user_data,
    };

    return ipgui_anim_create(&dsc);
}

__IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create_fade_out(
    u32_t duration,
    ipgui_anim_update_cb_t cb,
    ipgui_anim_complete_cb_t complete,
    void * user_data)
{
    ipgui_anim_dsc_t dsc = {
        .type        = IPGUI_ANIM_TYPE_FADE,
        .from.u8_    = 255,
        .to.u8_      = 0,
        .duration    = duration,
        .delay       = 0,
        .ease        = IPGUI_ANIM_EASE_IN_QUAD,
        .loop_mode   = IPGUI_ANIM_LOOP_NONE,
        .loop_count  = 0,
        .update_cb   = cb,
        .complete_cb = complete,
        .user_data   = user_data,
    };

    return ipgui_anim_create(&dsc);
}

__IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create_translate(
    ipgui_coord_t from_x, ipgui_coord_t from_y,
    ipgui_coord_t to_x, ipgui_coord_t to_y,
    u32_t duration,
    ipgui_anim_update_cb_t cb,
    ipgui_anim_complete_cb_t complete,
    void * user_data)
{
    ipgui_anim_dsc_t dsc = {
        .type           = IPGUI_ANIM_TYPE_TRANSLATE,
        .from.point_.x  = from_x,
        .from.point_.y  = from_y,
        .to.point_.x    = to_x,
        .to.point_.y    = to_y,
        .duration       = duration,
        .delay          = 0,
        .ease           = IPGUI_ANIM_EASE_OUT_CUBIC,
        .loop_mode      = IPGUI_ANIM_LOOP_NONE,
        .loop_count     = 0,
        .update_cb      = cb,
        .complete_cb    = complete,
        .user_data      = user_data,
    };

    return ipgui_anim_create(&dsc);
}

__IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create_scale(
    s32_t from_scale, s32_t to_scale,
    u32_t duration,
    ipgui_anim_update_cb_t cb,
    ipgui_anim_complete_cb_t complete,
    void * user_data)
{
    ipgui_anim_dsc_t dsc = {
        .type        = IPGUI_ANIM_TYPE_SCALE,
        .from.scoord_ = from_scale,
        .to.scoord_   = to_scale,
        .duration    = duration,
        .delay       = 0,
        .ease        = IPGUI_ANIM_EASE_OUT_BACK,
        .loop_mode   = IPGUI_ANIM_LOOP_NONE,
        .loop_count  = 0,
        .update_cb   = cb,
        .complete_cb = complete,
        .user_data   = user_data,
    };

    return ipgui_anim_create(&dsc);
}

__IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create_rotate(
    s32_t from_angle, s32_t to_angle,
    u32_t duration,
    ipgui_anim_update_cb_t cb,
    ipgui_anim_complete_cb_t complete,
    void * user_data)
{
    ipgui_anim_dsc_t dsc = {
        .type        = IPGUI_ANIM_TYPE_ROTATE,
        .from.scoord_ = from_angle,
        .to.scoord_   = to_angle,
        .duration    = duration,
        .delay       = 0,
        .ease        = IPGUI_ANIM_EASE_OUT_CUBIC,
        .loop_mode   = IPGUI_ANIM_LOOP_NONE,
        .loop_count  = 0,
        .update_cb   = cb,
        .complete_cb = complete,
        .user_data   = user_data,
    };

    return ipgui_anim_create(&dsc);
}
