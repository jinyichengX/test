#include "ipgui_vector.h"

#define IPGUI_VEC_SCALED_MAGICNUM_FLOAT     0.607253f                                                                                                                   /* the magic number is used to the pesudo-rotated vector */
#define IPGUI_VEC_SCALED_MAGICNUM_INT       (s32_t)(IPGUI_VEC_SCALED_MAGICNUM_FLOAT * (float)(1 << IPGUI_PIXEL_BITS))                                                     /* integer part */
#define IPGUI_VEC_SCALED_MAGICNUM_FRACT     (s32_t)(((IPGUI_VEC_SCALED_MAGICNUM_FLOAT * (float)(1 << IPGUI_PIXEL_BITS)) * 100)  - (IPGUI_VEC_SCALED_MAGICNUM_INT * 100))  /* fract part, percent */
#define IPGUI_VEC_SCALED_MAGICNUM           ((IPGUI_VEC_SCALED_MAGICNUM_INT * 100)  + IPGUI_VEC_SCALED_MAGICNUM_FRACT)
#define IPGUI_VEC_ROTATE_ITER_MAX           IPGUI_ARRAY_LEN(ipgui_arctan_lut)                                                                                           /* the value must be same with arctan lookup table size */                     /* the value must be same with arctan lookup table size */

#define IPGUI_DOWNSCALE_VECTOR(v, out)/* v:point of vec, out:point of scaled vec*/  \
do {\
    ipgui_scoord_t delta, tmp;\
    ipgui_scoord_t coeff = 100 *  (1 << IPGUI_PIXEL_BITS);\
    tmp = IPGUI_VEC_SCALED_MAGICNUM;\
    tmp *= (v)->x;\
    delta = tmp / coeff;\
    if (tmp % coeff > (coeff >> 1)) delta ++;\
    (out)->x = delta;\
    tmp = IPGUI_VEC_SCALED_MAGICNUM;\
    tmp *= (v)->y;\
    delta = tmp / coeff;\
    if (tmp % coeff > (coeff >> 1)) delta ++;\
    (out)->y = delta;\
}while(0)

#define IPGUI_DOWNSACLE_COORD(c) /* c:value of scaled coord */ \
do {\
    ipgui_scoord_t delta, tmp;\
    ipgui_scoord_t coeff = 100 *  (1 << IPGUI_PIXEL_BITS);\
    tmp = IPGUI_VEC_SCALED_MAGICNUM;\
    tmp *= (c);\
    delta = tmp / coeff;\
    if (tmp % coeff > (coeff >> 1)) delta ++;\
    (c) = delta;\
}while(0)

__IPGUI_API__ ipgui_scoord_t ipgui_vector_mod(ipgui_svector_t * v)
{
    if (v->x == 0) {
        return IPGUI_ABS(v->y);
    } else if (v->y == 0) {
        return IPGUI_ABS(v->x);
    }

    ipgui_scoord_t x = IPGUI_ABS(v->x);
    ipgui_scoord_t y = IPGUI_ABS(v->y);
    ipgui_int_sqrt(x * x + y * y, &x, &y);
    if (y >= 500)
        x = x + 1;
    return x;
}

/* 伪旋转，参考系是笛卡尔坐标系，非屏幕坐标系
 * 如果angle大于0，则逆时针旋转，否则顺时针旋转
 * （返回的向量x,y各是freetype计算的结果的1.414倍
 * 所以向量长度也是freetype计算的结果的1.414倍
 * 因为多旋转了一个45°）
 */
__IPGUI_STATIC__ void ipgui_vector_pesudo_rotate(ipgui_svector_t * v, 
        ipgui_angle_t angle, ipgui_svector_t * scaled_out)
{
    ipgui_svector_t temp_v;
    ipgui_scoord_t for_exc;

    if (!v || !scaled_out) return;

    temp_v.x = v->x;
    temp_v.y = v->y;

    angle = angle % IPGUI_ANGLE_2PI;
    if (angle < 0) {
        angle = IPGUI_ANGLE_2PI + angle;
    }
    while (angle > IPGUI_ANGLE_PI2) {
        angle -= IPGUI_ANGLE_PI2;
        for_exc = temp_v.x;
        temp_v.x = -temp_v.y;
        temp_v.y = for_exc;
    }

    ipgui_scoord_t offx, offy;
    s32_t compensate = 0, compensate_temp;
    for (s32_t i = 0; i < IPGUI_VEC_ROTATE_ITER_MAX; i++) {
        if (angle > 0) {
            if (temp_v.y < 0) compensate_temp = -compensate;
            else compensate_temp = compensate;
            offx = (temp_v.y + compensate_temp) >> i;

            if (temp_v.x < 0) compensate_temp = -compensate;
            else compensate_temp = compensate;
            offy = (temp_v.x + compensate_temp) >> i;

            angle -= ipgui_arctan_lut[i];
        } else {
            if (temp_v.y < 0) compensate_temp = -compensate;
            else compensate_temp = compensate;
            offx = -((temp_v.y + compensate_temp) >> i);

            if (temp_v.x < 0) compensate_temp = -compensate;
            else compensate_temp = compensate;
            offy = -((temp_v.x + compensate_temp) >> i);

            angle += ipgui_arctan_lut[i];
        }
        temp_v.x -= offx;
        temp_v.y += offy;
        compensate = (1 << i);
    }
    scaled_out->x = temp_v.x;
    scaled_out->y = temp_v.y;
}

/* pesudo plorarization operation */
__IPGUI_API__ void ipgui_vector_pesudo_polarization(
        ipgui_svector_t * v, ipgui_scoord_t * angle, ipgui_scoord_t * mod)
{
    ipgui_angle_t angle_tmp = 0;
    ipgui_svector_t temp_v;
    ipgui_scoord_t for_exc;
    s8_t sign;

    if (!v || !angle || !mod) return;

    temp_v.x = v->x;
    temp_v.y = v->y;

    /* offset angle */
    if (v->y > 0) {
        angle_tmp += IPGUI_ANGLE_PI2;
        for_exc = temp_v.x;
        temp_v.x = temp_v.y;
        temp_v.y = -for_exc;
        sign = 1;
    } else {
        angle_tmp += (-IPGUI_ANGLE_PI2);
        for_exc = temp_v.x;
        temp_v.x = -temp_v.y;
        temp_v.y = for_exc;
        sign = 0;
    }

    ipgui_scoord_t offx, offy;
    s32_t compensate = 0, compensate_temp;
    for (s32_t i = 0; i < IPGUI_VEC_ROTATE_ITER_MAX; i++) {
        if (temp_v.y < 0) {
            if (temp_v.y < 0) compensate_temp = -compensate;
            else compensate_temp = compensate;
            offx = (temp_v.y + compensate_temp) >> i;

            if (temp_v.x < 0) compensate_temp = -compensate;
            else compensate_temp = compensate;
            offy = (temp_v.x + compensate_temp)>> i;

            angle_tmp -= ipgui_arctan_lut[i];
        } else {
            if (temp_v.y < 0) compensate_temp = -compensate;
            else compensate_temp = compensate;
            offx = -((temp_v.y + compensate_temp) >> i);

            if (temp_v.x < 0) compensate_temp = -compensate;
            else compensate_temp = compensate;
            offy = -((temp_v.x + compensate_temp) >> i);

            angle_tmp += ipgui_arctan_lut[i];
        }
        temp_v.x -= offx;
        temp_v.y += offy;
        compensate = (1 << i);
    }
    * mod = temp_v.x;
    * angle = angle_tmp;
}

/* rotate operation, return the rotated vector*/
__IPGUI_API__ void ipgui_vector_rotate(ipgui_svector_t * v, 
        ipgui_angle_t angle, ipgui_svector_t * out)
{
    ipgui_vector_pesudo_rotate(v, angle, out);
    IPGUI_DOWNSCALE_VECTOR(out, out);
}

/* 屏幕坐标系中旋转，人眼望向屏幕，逆时针旋转是正角度 */
__IPGUI_API__ void ipgui_vector_rotate_screen(ipgui_svector_t * v, 
        ipgui_angle_t angle, ipgui_svector_t * out)
{
    ipgui_vector_pesudo_rotate(v, -angle, out);
    IPGUI_DOWNSCALE_VECTOR(out, out);
}

/* plorarization operation, return the angle(-180°~180°) and mod of the vector*/
__IPGUI_API__ void ipgui_vector_polarization(ipgui_svector_t * v, 
        ipgui_angle_t * angle, ipgui_scoord_t * mod)
{
    ipgui_vector_pesudo_polarization(v, angle, mod);
    IPGUI_DOWNSACLE_COORD(* mod);
}

/* 屏幕坐标系中求向量角和模，人眼望向屏幕，逆时针旋转是正角度 */
__IPGUI_API__ void ipgui_vector_polarization_screen(ipgui_svector_t * v, 
        ipgui_angle_t * angle, ipgui_scoord_t * mod)
{
    ipgui_svector_t temp_v;
    temp_v.x = v->x;
    temp_v.y = -v->y;
    ipgui_vector_pesudo_polarization(&temp_v, angle, mod);
    IPGUI_DOWNSACLE_COORD(* mod);
}

/* 求两个向量之间的夹角，从from旋转到to需要的角度[-180° ~ 180°] 
 * 参考系是笛卡尔坐标系
 * 逆时针旋转是正角度，顺时针旋转是负角度
 */
__IPGUI_API__ ipgui_angle_t ipgui_vector_angle_diff(ipgui_svector_t * from,
        ipgui_svector_t * to)
{
    ipgui_angle_t a1, a2, diff;
    ipgui_scoord_t mod;//no use

    ipgui_vector_pesudo_polarization(from, &a1, &mod);
    ipgui_vector_pesudo_polarization(to, &a2, &mod);
    if (a1 > 0) {
        diff = a2 - a1;
        if (diff < (-IPGUI_ANGLE_PI))
            diff += IPGUI_ANGLE_2PI;
    } else {
        diff = a2 - a1;
        if (diff > IPGUI_ANGLE_PI)
            diff -= IPGUI_ANGLE_2PI;
    }
    return diff;
}

/* 屏幕坐标系中求向量夹角，人眼望向屏幕，逆时针旋转是正角度 */
__IPGUI_API__ ipgui_angle_t ipgui_vector_angle_diff_screen(ipgui_svector_t * from,
        ipgui_svector_t * to)
{
    return -ipgui_vector_angle_diff(from, to);
}

/* 求向量长度，与ipgui_vector_mod是一样的 */
__IPGUI_API__ ipgui_scoord_t ipgui_vector_len(ipgui_svector_t * v)
{
    ipgui_angle_t angle;
    ipgui_scoord_t mod;
    ipgui_vector_polarization(v, &angle, &mod);
    return mod;
}