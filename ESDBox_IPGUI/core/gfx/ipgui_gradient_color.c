#include "ipgui_gradient_color.h"
#include "ipgui_memory.h"

/* 线性渐变调用流程（测试完成）
 * mode1：比例模式
 * 1. ipgui_liner_gradient_init
 * 2. ipgui_liner_gradient_add_stop
 * 3. ipgui_liner_gradient_apply_to_aabb
 * 4. ipgui_get_liner_gradient_pos_at_xy
 * 5. ipgui_liner_gradient_color_get
 * 4和5可以合为一步ipgui_liner_gradient_color_get(grad_struct, ipgui_get_liner_gradient_pos_at_xy(x, y), &color);
 * mode2：直接坐标模式
 * 1. ipgui_liner_gradient_init_direct
 * 2. ipgui_liner_gradient_add_stop
 * 3. ipgui_get_liner_gradient_pos_at_xy
 * 4. ipgui_liner_gradient_color_get
 * 3和4可以合为一步ipgui_liner_gradient_color_get(grad_struct, ipgui_get_liner_gradient_pos_at_xy(x, y), &color);
 */

/* 径向渐变调用流程（测试完成）
 * 直接坐标模式（只有直接坐标模式）
 * 1. ipgui_radial_gradient_init
 * 2. ipgui_radial_gradient_add_stop
 * 3. ipgui_get_radial_gradient_pos_at_xy
 * 4. ipgui_radial_gradient_color_get
 * 3和4可以合为一步ipgui_radial_gradient_color_get(grad_struct, ipgui_get_radial_gradient_pos_at_xy(x, y), &color);
 */

 /* 锥形渐变调用流程（测试完成）
 * 直接坐标模式（只有直接坐标模式）
 * 1. ipgui_conic_gradient_init
 * 2. ipgui_conic_gradient_add_stop
 * 3. ipgui_get_conic_gradient_pos_at_xy
 * 4. ipgui_conic_gradient_color_get
 * 3和4可以合为一步ipgui_conic_gradient_color_get(grad_struct, ipgui_get_conic_gradient_pos_at_xy(x, y), &color);
 */

#if USE_INV_TABLE == 1
extern u16_t g_inv_tbl[256];
#endif

#define IPGUI_GRADIENT_SCALE_FACTOR 255.0f
#define IPGUI_GRADIENT_SCALE(v) (s32_t)((v) * IPGUI_GRADIENT_SCALE_FACTOR)

#if IPGUI_GRADIENT_LUT_EN == 1
static u8_t grad_lerp_lut[256][256] = {0};
__IPGUI_API__ __IPGUI_INIT__ void ipgui_gradient_lut_init(void)
{
    for (s32_t dist = 0; dist < 256; dist ++) {
        for (s32_t value = 0; value < 256; value ++) {
            grad_lerp_lut[dist][value] = (dist * value + 127) / 255;
        }
    }
}
#endif

__IPGUI_API__ void ipgui_liner_gradient_init(
    ipgui_liner_gradient_color_t * gradient, 
    float                          x_start, 
    float                          y_start,/* normalized(0.0 - 1.0) */
    float                          x_end,
    float                          y_end) /* normalized(0.0 - 1.0) */
{
    gradient->stop_nr = 0;
    gradient->x_start = IPGUI_GRADIENT_SCALE(x_start);
    gradient->y_start = IPGUI_GRADIENT_SCALE(y_start);
    gradient->x_end   = IPGUI_GRADIENT_SCALE(x_end);
    gradient->y_end   = IPGUI_GRADIENT_SCALE(y_end);
}

__IPGUI_API__ void ipgui_liner_gradient_init_direct(
    ipgui_liner_gradient_color_t * gradient, 
    ipgui_coord_t                  x_start,
    ipgui_coord_t                  y_start, /* 渐变起点，绝对坐标 */
    ipgui_coord_t                  x_end,
    ipgui_coord_t                  y_end) /* 渐变终点，绝对坐标 */
{
    gradient->stop_nr              = 0;
    gradient->x_start              = x_start;
    gradient->y_start              = y_start;
    gradient->x_end                = x_end;
    gradient->y_end                = y_end;
    gradient->x_start_abs          = x_start;
    gradient->y_start_abs          = y_start;
    gradient->x_end_abs            = x_end;
    gradient->y_end_abs            = y_end;
    gradient->gradient_vector.x    = x_end - x_start;
    gradient->gradient_vector.y    = y_end - y_start;
    gradient->gradient_vec_mod_pow = gradient->gradient_vector.x * gradient->gradient_vector.x;
    gradient->gradient_vec_mod_pow += gradient->gradient_vector.y * gradient->gradient_vector.y;
    if (gradient->gradient_vec_mod_pow == 0) gradient->gradient_vec_mod_pow = 1;/* 防止0做除数 */
}

#if 0
#include "ipgui_debug.h"
__IPGUI_API__ void ipgui_print_gradient_stops(ipgui_liner_gradient_color_t * gradient)
{
   s32_t stop_nr = gradient->stop_nr;
    ipgui_gradient_color_stop_t * iter;
   s32_t idx = 0;
    for (; idx < stop_nr; idx ++) {
        iter = &gradient->stops[idx];
        ipgui_dbg_info("current pos is %d\r\n", iter->pos);
    }
}
#endif

__IPGUI_API__ s32_t ipgui_liner_gradient_add_stop(
    ipgui_liner_gradient_color_t * gradient,
    ipgui_gradient_color_stop_t  * stop)
{
    if (gradient->stop_nr >= IPGUI_GRADIENT_STOP_MAX)
        return -1;

    s32_t stop_nr = gradient->stop_nr;
    ipgui_gradient_color_stop_t * iter;
    s32_t idx     = 0;
    
    for (; idx < stop_nr; idx ++) {
        iter = &gradient->stops[idx];
        if (iter->pos > stop->pos) {
            for (s32_t i = stop_nr; i > idx; i --) {
                gradient->stops[i].color = gradient->stops[i - 1].color;
                gradient->stops[i].pos   = gradient->stops[i - 1].pos;
            }
            break;
        }
    }
    gradient->stops[idx].color = stop->color;
    gradient->stops[idx].pos   = stop->pos;
    gradient->stop_nr++;

    return 0;
}

__IPGUI_API__ s32_t ipgui_liner_gradient_remove_stop(
    ipgui_liner_gradient_color_t * gradient,
    u8_t                           pos)
{
    s32_t stop_nr = gradient->stop_nr;
    ipgui_gradient_color_stop_t * iter;
    s32_t idx;

    for (idx = 0; idx < stop_nr; idx ++) {
        iter = &gradient->stops[idx];
        if (iter->pos == pos) {
            /* find it, remove */
            for (s32_t i = idx + 1; i < stop_nr; i ++) {
                gradient->stops[i - 1].pos   = gradient->stops[i].pos;
                gradient->stops[i - 1].color = gradient->stops[i].color;
            }
            gradient->stop_nr --;
            return 0;
        }
    }

    return -1;
}

__IPGUI_API__ void ipgui_liner_gradient_apply_to_aabb(
    ipgui_liner_gradient_color_t * gradient,
    ipgui_aabb_t                 * aabb)
{
    gradient->aabb.start.x = aabb->start.x;
    gradient->aabb.start.y = aabb->start.y;
    gradient->aabb.end.x   = aabb->end.x;
    gradient->aabb.end.y   = aabb->end.y;

    ipgui_coord_t w, h;
    w = aabb->end.x - aabb->start.x + 1;
    h = aabb->end.y - aabb->start.y + 1;
    /* calculate absolute coordinate of gradient start point and end point */
    gradient->x_start_abs = aabb->start.x + (gradient->x_start * w + 127) / 255;
    gradient->y_start_abs = aabb->start.y + (gradient->y_start * h + 127) / 255;
    gradient->x_end_abs   = aabb->start.x + (gradient->x_end * w + 127) / 255;
    gradient->y_end_abs   = aabb->start.y + (gradient->y_end * h + 127) / 255;

    /* calculate gradient absolute vector, not absolute value!!! */
    gradient->gradient_vector.x = gradient->x_end_abs - gradient->x_start_abs;
    gradient->gradient_vector.y = gradient->y_end_abs - gradient->y_start_abs;

    /* calculate gradient vector mod power */
    gradient->gradient_vec_mod_pow  = gradient->gradient_vector.x * gradient->gradient_vector.x;
    gradient->gradient_vec_mod_pow += gradient->gradient_vector.y * gradient->gradient_vector.y;
    if (gradient->gradient_vec_mod_pow == 0) gradient->gradient_vec_mod_pow = 1;/* 防止0做除数 */
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void liner_first_stop_color(ipgui_liner_gradient_color_t * gradient, ipgui_color_t * res)
{
    ipgui_memcpy((void *)res, (void *)&gradient->stops[0].color, sizeof(ipgui_color_t));
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void liner_last_stop_color(ipgui_liner_gradient_color_t * gradient, ipgui_color_t * res)
{
    ipgui_memcpy((void *)res, (void *)&gradient->stops[gradient->stop_nr - 1].color, sizeof(ipgui_color_t));
}

__IPGUI_STATIC__ __IPGUI_INLINE__ u32_t interpolate_pixel(
    u32_t x, u8_t a/* 1 - t */, 
    u32_t y, u8_t b /* t */)
{
	u32_t t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
	t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
	t &= 0xff00ff;
	x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
	x = (x + ((x >> 8) & 0xff00ff) + 0x800080);
	x &= 0xff00ff00;
	x |= t;
	return x;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_interpolate_color(
    ipgui_color_t * c1, u8_t dist, 
    ipgui_color_t * c2, u8_t idist,
    ipgui_color_t * res)
{
#if IPGUI_GRADIENT_LUT_EN == 1
    u8_t a, r, g, b;
    a = grad_lerp_lut[dist][c2->a] + grad_lerp_lut[idist][c1->a];
    r = grad_lerp_lut[dist][c2->r] + grad_lerp_lut[idist][c1->r];
    g = grad_lerp_lut[dist][c2->g] + grad_lerp_lut[idist][c1->g];
    b = grad_lerp_lut[dist][c2->b] + grad_lerp_lut[idist][c1->b];
    res->a = a;
    res->r = r;
    res->g = g;
    res->b = b;
#else
#if 0
    u8_t a, r, g, b;
    a = (s32_t)(c2->a * dist + 127) / 255 + (s32_t)(c1->a * idist + 127) / 255;
    r = (s32_t)(c2->r * dist + 127) / 255 + (s32_t)(c1->r * idist + 127) / 255;
    g = (s32_t)(c2->g * dist + 127) / 255 + (s32_t)(c1->g * idist + 127) / 255;
    b = (s32_t)(c2->b * dist + 127) / 255 + (s32_t)(c1->b * idist + 127) / 255;
    res->a = a;
    res->r = r;
    res->g = g;
    res->b = b;
#else
    res->v = interpolate_pixel(c2->v, dist, c1->v, idist);
#endif
#endif
}

__IPGUI_API__ void ipgui_liner_gradient_color_get(
    ipgui_liner_gradient_color_t * gradient, 
    u8_t                           pos,
    ipgui_color_t                * res)
{
    if (gradient->stop_nr < 2)
        return;

    s32_t idx;
    ipgui_gradient_color_stop_t * iter, * last;

    if (pos <= gradient->stops[0].pos) {
        liner_first_stop_color(gradient, res);
        return;
    }

    for (idx = 1; idx < gradient->stop_nr; idx ++) {
        iter = &gradient->stops[idx];

        if (iter->pos >= pos) {
            if (iter->pos == pos) {
                * res = iter->color;
                return;
            } else {
                /* interpolate color */
                u8_t dist, idist, delta;
                last = &gradient->stops[idx - 1];
                delta = iter->pos - last->pos;
#if USE_INV_TABLE == 0
                dist = (pos - last->pos) * 255 / delta;
#else
                dist = ((pos - last->pos) * g_inv_tbl[delta]) >> 8;
#endif
                idist = 255 - dist;
                ipgui_interpolate_color(&last->color, dist, &iter->color, idist, res);
                return;
            }
        }
    }
    liner_last_stop_color(gradient, res);
}

__IPGUI_API__ u8_t ipgui_get_liner_gradient_pos_at_xy(
    ipgui_liner_gradient_color_t * gradient, 
    ipgui_coord_t                  x,
    ipgui_coord_t                  y)
{
    /* map to gradient vector(the vecor is: v = (x_start,y_start)---->(x_end,y_end)) */
    u8_t pos;
    s32_t proj_int, proj_frac;
    s32_t v_dot; /* 向量点乘，复习一遍点乘公式a.b = |a|*|b|*cos(theta) 推出a.b/|b| = |a|*cos(theta)等于在b上的投影长度（有符号） /
    /* 设坐标点(x,y)为点p，求p到(x_start_abs, y_start_abs)在v上的投影 */
    ipgui_vector_t start2p;
    start2p.x = x - gradient->x_start_abs;
    start2p.y = y - gradient->y_start_abs;

    v_dot     = start2p.x * gradient->gradient_vector.x;
    v_dot    += start2p.y * gradient->gradient_vector.y;
    proj_int  = v_dot     / gradient->gradient_vec_mod_pow;
    proj_frac = v_dot     % gradient->gradient_vec_mod_pow;

    if ((proj_int < 0) || (proj_frac < 0)) { /* out of gradient vector start */
        pos = 0;
    } else if (proj_int >= 1) { /* out of gradient vector end */
        pos = 255;
    } else { /* middle, most case */
        pos = proj_frac * 255 / gradient->gradient_vec_mod_pow;
    }

    return pos;
}

__IPGUI_API__ void ipgui_radial_gradient_init(
    ipgui_radial_gradient_color_t * gradient,
    ipgui_coord_t                   center_x,
    ipgui_coord_t                   center_y,
    ipgui_coord_t                   radius)
{
    gradient->center_x   = center_x;
    gradient->center_y   = center_y;
    gradient->radius     = radius;
    gradient->radius_pow = radius * radius;
    gradient->stop_nr    = 0;
}

__IPGUI_API__ s32_t ipgui_radial_gradient_add_stop(
    ipgui_radial_gradient_color_t * gradient,
    ipgui_gradient_color_stop_t   * stop)
{
    if (gradient->stop_nr >= IPGUI_GRADIENT_STOP_MAX)
        return -1;

    s32_t stop_nr = gradient->stop_nr;
    ipgui_gradient_color_stop_t * iter;
    s32_t idx = 0;
    
    for (; idx < stop_nr; idx ++) {
        iter = &gradient->stops[idx];
        if (iter->pos > stop->pos) {
            for (s32_t i = stop_nr; i > idx; i --) {
                gradient->stops[i].color = gradient->stops[i - 1].color;
                gradient->stops[i].pos   = gradient->stops[i - 1].pos;
            }
            break; 
        }
    }
    gradient->stops[idx].color = stop->color;
    gradient->stops[idx].pos   = stop->pos;
    gradient->stop_nr++;

    return 0;
}

__IPGUI_API__ s32_t ipgui_radial_gradient_remove_stop(
    ipgui_radial_gradient_color_t * gradient,
    u8_t                            pos)
{
    s32_t stop_nr = gradient->stop_nr;
    ipgui_gradient_color_stop_t * iter;
    s32_t idx;

    for (idx = 0; idx < stop_nr; idx ++) {
        iter = &gradient->stops[idx];
        if (iter->pos == pos) {
            /* find it, remove */
            for (s32_t i = idx + 1; i < stop_nr; i ++) {
                gradient->stops[i - 1].pos   = gradient->stops[i].pos;
                gradient->stops[i - 1].color = gradient->stops[i].color;
            }
            gradient->stop_nr --;
            return 0;
        }
    }

    return -1;
}

__IPGUI_API__ void ipgui_radial_gradient_color_get(
    ipgui_radial_gradient_color_t * gradient,
    u8_t                            pos,
    ipgui_color_t                 * res)
{
    if (gradient->stop_nr < 2)
        return;

    s32_t idx;
    ipgui_gradient_color_stop_t * iter, * last;

    if (pos <= gradient->stops[0].pos) {
        * res = gradient->stops[0].color;
        return;
    }

    for (idx = 1; idx < gradient->stop_nr; idx ++) {
        iter = &gradient->stops[idx];

        if (iter->pos >= pos) {
            if (iter->pos == pos) {
                * res = iter->color;
                return;
            } else {
                /* interpolate color */
                u8_t dist, idist, delta;
                last = &gradient->stops[idx - 1];
                delta = iter->pos - last->pos;
#if USE_INV_TABLE == 0
                dist = (pos - last->pos) * 255 / delta;
#else
                dist = ((pos - last->pos) * g_inv_tbl[delta]) >> 8;
#endif
                idist = 255 - dist;
                ipgui_interpolate_color(&last->color, dist, &iter->color, idist, res);
                return;
            }
        }
    }
    
    * res = gradient->stops[gradient->stop_nr - 1].color;
}

/* 逻辑存疑 */
__IPGUI_API__ u8_t ipgui_get_radial_gradient_pos_at_xy(
    ipgui_radial_gradient_color_t * gradient,
    ipgui_coord_t                   x,
    ipgui_coord_t                   y)
{
    s32_t dx = x - gradient->center_x;
    s32_t dy = y - gradient->center_y;
    s32_t dp = dx * dx + dy * dy;
    s32_t r  = gradient->radius;
    s32_t rp = gradient->radius_pow;
    
    if (dp >= rp) return 255;
    
    /* 关键：使用预缩放避免大数运算 */
    /* 计算：pos² = (dp * 255²) / rp */
    /* 使用分步计算避免溢出 */
    
    /* 先计算 dp * 255 / r */
    u32_t temp = (u32_t)dp * 255U / r;
    
    /* 再计算 pos² = temp * 255 / r */
    u32_t pos_sq = temp * 255U / r;
    
    /* 查找平方根 */
    u8_t pos = 0;
    u32_t test;
    
    while (pos < 255) {
        test = (pos + 1) * (pos + 1);
        if (test > pos_sq) break;
        pos ++;
    }
    
    return pos;
}

/* angle_start是角度制 不是弧度制 */
__IPGUI_API__ void ipgui_conic_gradient_init(
    ipgui_conic_gradient_color_t * gradient,
    ipgui_coord_t                  center_x,
    ipgui_coord_t                  center_y,
    s32_t                          angle_start)
{
    gradient->center_x    = center_x;
    gradient->center_y    = center_y;
    gradient->stop_nr     = 0;

    /* 归一化到0-360 */
    angle_start           = angle_start % 360;
    if (angle_start < 0) angle_start += 360;
    gradient->angle_start = angle_start;
}


__IPGUI_API__ s32_t ipgui_conic_gradient_add_stop(
    ipgui_conic_gradient_color_t * gradient,
    ipgui_gradient_color_stop_t * stop)
{
    if (gradient->stop_nr >= IPGUI_GRADIENT_STOP_MAX)
        return -1;

    s32_t stop_nr = gradient->stop_nr;
    ipgui_gradient_color_stop_t * iter;
    s32_t idx = 0;
    
    for (; idx < stop_nr; idx ++) {
        iter = &gradient->stops[idx];
        if (iter->pos > stop->pos) {
            for (s32_t i = stop_nr; i > idx; i --) {
                gradient->stops[i].color = gradient->stops[i - 1].color;
                gradient->stops[i].pos   = gradient->stops[i - 1].pos;
            }
            break; 
        }
    }
    gradient->stops[idx].color = stop->color;
    gradient->stops[idx].pos   = stop->pos;
    gradient->stop_nr++;

    return 0;
}

__IPGUI_API__ s32_t ipgui_conic_gradient_remove_stop(
    ipgui_conic_gradient_color_t * gradient,
    u8_t                           pos)
{
    s32_t stop_nr = gradient->stop_nr;
    ipgui_gradient_color_stop_t * iter;
    s32_t idx;

    for (idx = 0; idx < stop_nr; idx ++) {
        iter = &gradient->stops[idx];
        if (iter->pos == pos) {
            /* find it, remove */
            for (s32_t i = idx + 1; i < stop_nr; i ++) {
                gradient->stops[i - 1].pos   = gradient->stops[i].pos;
                gradient->stops[i - 1].color = gradient->stops[i].color;
            }
            gradient->stop_nr --;
            return 0;
        }
    }

    return -1;
}

__IPGUI_API__ void ipgui_conic_gradient_color_get(
    ipgui_conic_gradient_color_t * gradient,
    u8_t                           pos,
    ipgui_color_t                * res)
{
    if (gradient->stop_nr < 2)
        return;

    s32_t idx;
    ipgui_gradient_color_stop_t * iter, * last;

    if (pos <= gradient->stops[0].pos) {
        * res = gradient->stops[0].color;
        return;
    }

    for (idx = 1; idx < gradient->stop_nr; idx ++) {
        iter = &gradient->stops[idx];

        if (iter->pos >= pos) {
            if (iter->pos == pos) {
                * res = iter->color;
                return;
            } else {
                /* interpolate color */
                u8_t dist, idist, delta;
                last = &gradient->stops[idx - 1];
                delta = iter->pos - last->pos;
#if USE_INV_TABLE == 0
                dist = (pos - last->pos) * 255 / delta;
#else
                dist = ((pos - last->pos) * g_inv_tbl[delta]) >> 8;
#endif
                idist = 255 - dist;
                ipgui_interpolate_color(&last->color, dist, &iter->color, idist, res);
                return;
            }
        }
    }
    
    * res = gradient->stops[gradient->stop_nr - 1].color;
}

/* slope_cache[i] / 1024 约等于tan(i) */
static const u16_t slope_cache[90] = {
       0,                                                                /* 0°     */
      18,    36,    54,    72,    90,   108,   126,   144,   162,   181, /* 1-10°  */
     199,   218,   236,   255,   274,   294,   313,   333,   353,   373, /* 11-20° */
     393,   414,   435,   456,   477,   499,   522,   544,   568,   591, /* 21-30° */
     615,   640,   665,   691,   717,   744,   772,   800,   829,   859, /* 31-40° */
     890,   922,   955,   989,  1024,  1060,  1098,  1137,  1178,  1220, /* 41-50° */
    1265,  1311,  1359,  1409,  1462,  1518,  1577,  1639,  1704,  1774, /* 51-60° */
    1847,  1926,  2010,  2100,  2196,  2300,  2412,  2534,  2668,  2813, /* 61-70° */
    2974,  3152,  3349,  3571,  3822,  4107,  4435,  4818,  5268,  5807, /* 71-80° */
    6465,  7286,  8340,  9743, 11704, 14644, 19539, 29324, 58665,        /* 81-89° */
};

__IPGUI_API__ u8_t ipgui_get_conic_gradient_pos_at_xy(
    ipgui_conic_gradient_color_t * gradient,
    ipgui_coord_t                  x,
    ipgui_coord_t                  y)
{
    ipgui_vector_t v;
    v.x = x - gradient->center_x;
    v.y = y - gradient->center_y;

    if (v.x == 0 && v.y == 0) return 0;

    s32_t angle;/* 笛卡尔坐标系，相对X轴正方向，逆时针为正，范围0-360 */

    if (v.x == 0) {
        angle = (v.y > 0) ? 90 : 270;
    } else if (v.y == 0) {
        angle = (v.x > 0) ? 0  : 180;
    } else {
        s32_t ax    = (v.x > 0) ? v.x : -v.x;
        s32_t ay    = (v.y > 0) ? v.y : -v.y;
        s32_t idx   = 0;
        s32_t lo    = 0, hi = 89, mid;

        while (lo <= hi) {
            mid = (lo + hi) >> 1;
            if ((s32_t)slope_cache[mid] * ax <= (s32_t)ay * 1024) {
                idx = mid;
                lo    = mid + 1;
            } else {
                hi    = mid - 1;
            }
        }

        if      (v.x > 0 && v.y > 0) angle = idx;
        else if (v.x < 0 && v.y > 0) angle = 180 - idx;
        else if (v.x < 0 && v.y < 0) angle = 180 + idx;
        else                         angle = 360 - idx;
    }

    return ((angle - gradient->angle_start) * 255) / 360;
}