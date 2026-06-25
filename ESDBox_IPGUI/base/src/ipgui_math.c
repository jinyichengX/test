#include "ipgui_math.h"
#include "ipgui_prim.h"

/* sin0-90查找表计算时必须使用int进行计算 */
const unsigned short ipgui_sinLUT[] = {
    0,     572,   1144,  1715,  2286,  2856,  3425,  3993,  4560,  5126,   /* 0°~9°   */
    5690,  6252,  6813,  7371,  7927,  8481,  9032,  9580,  10126, 10668,  /* 10°~19° */
    11207, 11743, 12275, 12803, 13328, 13848, 14364, 14876, 15383, 15886,  /* 20°~29° */
    16384, 16877, 17364, 17846, 18324, 18794, 19260, 19720, 20173, 20621,  /* 30°~39° */
    21062, 21497, 21925, 22347, 22762, 23170, 23571, 23964, 24351, 24730,  /* 40°~49° */
    25101, 25465, 25821, 26169, 26509, 26841, 27165, 27481, 27789, 28088,  /* 50°~59° */
    28377, 28659, 28932, 29196, 29452, 29698, 29935, 30163, 30382, 30592,  /* 60°~69° */
    30792, 30983, 31164, 31336, 31499, 31651, 31795, 31928, 32052, 32166,  /* 70°~79° */
    32270, 32365, 32449, 32524, 32588, 32643, 32688, 32723, 32748, 32763,  /* 80°~89° */
    32768,/* 90° */
};

/* definitions the angles(times of 65536) of 2^(-n)
 * tan(45°) = 1 = 2^0,              θ = 45°
 * tan(26.565051°) = 0.5 = 2^(-1),  θ = 26.565051°
 * tan(14.036243°) = 0.25 = 2^(-2), θ = 14.036243°
 * tan(7.125016°) = 0.125 = 2^(-3), θ = 7.125016°
 * ........
*/
#define IPGUI_ANGLE_ROUND(x) ((ipgui_angle_t)((x) + 0.5f))
const ipgui_angle_t ipgui_arctan_lut[23] = {
    IPGUI_ANGLE_ROUND(45.00000   * IPGUI_ANGLE_PRECISION),      /* 45.00000°   */           
    IPGUI_ANGLE_ROUND(26.565051  * IPGUI_ANGLE_PRECISION),      /* 26.565051°  */
    IPGUI_ANGLE_ROUND(14.036243  * IPGUI_ANGLE_PRECISION),      /* 14.036243°  */
    IPGUI_ANGLE_ROUND(7.125016   * IPGUI_ANGLE_PRECISION),      /* 7.125016°   */
    IPGUI_ANGLE_ROUND(3.576334   * IPGUI_ANGLE_PRECISION),      /* 3.576334°   */
    IPGUI_ANGLE_ROUND(1.789911   * IPGUI_ANGLE_PRECISION),      /* 1.789911°   */
    IPGUI_ANGLE_ROUND(0.895174   * IPGUI_ANGLE_PRECISION),      /* 0.895174°   */
    IPGUI_ANGLE_ROUND(0.44761417 * IPGUI_ANGLE_PRECISION),      /* 0.44761417° */
    IPGUI_ANGLE_ROUND(0.2238105  * IPGUI_ANGLE_PRECISION),      /* 0.2238105°  */
    IPGUI_ANGLE_ROUND(0.11190568 * IPGUI_ANGLE_PRECISION),      /* 0.11190568° */
    IPGUI_ANGLE_ROUND(0.05595289 * IPGUI_ANGLE_PRECISION),      /* 0.05595289° */
    IPGUI_ANGLE_ROUND(0.02797645 * IPGUI_ANGLE_PRECISION),      /* 0.02797645° */
    IPGUI_ANGLE_ROUND(0.01398823 * IPGUI_ANGLE_PRECISION),      /* 0.01398823° */
    IPGUI_ANGLE_ROUND(0.00699411 * IPGUI_ANGLE_PRECISION),      /* 0.00699411° */
    IPGUI_ANGLE_ROUND(0.00349706 * IPGUI_ANGLE_PRECISION),      /* 0.00349706° */
    IPGUI_ANGLE_ROUND(0.00174853 * IPGUI_ANGLE_PRECISION),      /* 0.00174853° */
    IPGUI_ANGLE_ROUND(0.00087426 * IPGUI_ANGLE_PRECISION),      /* 0.00087426° */
    IPGUI_ANGLE_ROUND(0.00043713 * IPGUI_ANGLE_PRECISION),      /* 0.00043713° */
    IPGUI_ANGLE_ROUND(0.00021857 * IPGUI_ANGLE_PRECISION),      /* 0.00021857° */
    IPGUI_ANGLE_ROUND(0.00010928 * IPGUI_ANGLE_PRECISION),      /* 0.00010928° */
    IPGUI_ANGLE_ROUND(0.00005464 * IPGUI_ANGLE_PRECISION),      /* 0.00005464° */
    IPGUI_ANGLE_ROUND(0.00002732 * IPGUI_ANGLE_PRECISION),      /* 0.00002732° */
    IPGUI_ANGLE_ROUND(0.00001366 * IPGUI_ANGLE_PRECISION),      /* 0.00001366° */
};

__IPGUI_API__ s32_t ipgui_sin(s32_t angle)
{
    s32_t ret = 0;
    angle       = angle % 360;
    if(angle < 0) angle = 360 + angle;
    if(angle < 90)
    {
        ret = ipgui_sinLUT[angle];
    }
    else if(angle >= 90 && angle < 180)
    {
        angle = 180 - angle;
        ret   = ipgui_sinLUT[angle];
    }
    else if(angle >= 180 && angle < 270)
    {
        angle = angle - 180;
        ret   = -ipgui_sinLUT[angle];
    }
    else     /*angle >=270*/
    {
        angle = 360 - angle;
        ret   = -ipgui_sinLUT[angle];
    }
    return ret;
}

__IPGUI_API__ s32_t ipgui_cos(s32_t angle)
{
    return ipgui_sin(angle + 90);   /* cosα = sin(π/2+α) 或 cosα = sin(π/2－α) */
}

#if defined(IPGUI_USE_FPU)
#define IPGUI_FIXED2FLOAT(x) ((double)(x) / IPGUI_PIXEL_PRECI)
__IPGUI_API__ float ipgui_float_sqrt(ipgui_scoord_t a)
{
    if (!a) return 0;
    if (a < 0) return -1;
    /* 定点数转化成浮点数 */
    double x = (double)IPGUI_FIXED2FLOAT(a);

    /* 牛顿迭代法 */
    double x_iter = x;
    double last_iter;
    while (x_iter * x_iter - x > 0) {
        x_iter = (-(x_iter * x_iter - x)) / (2 * x_iter) + x_iter;
        if (x_iter == last_iter)
            break;
        last_iter = x_iter;
    }

    return (float)x_iter;
}
#endif

/* 计算最小的b且满足b * b > a */
__IPGUI_STATIC__ s32_t ipgui_guess_sqrt_iter_init(s32_t a)
{
    s32_t b;
    b = a >> 1;
    while (1)
    {
        if ((s64_t)b * (s64_t)b > a)
            b = b >> 1;
        else 
            break;
    }
    return b << 1;
}

/* r_frac_1000是小数部分，精确到小数点后3位 */
__IPGUI_API__ ipgui_err_t ipgui_int_sqrt(s32_t a, s32_t * r_int, s32_t * r_frac_1000)
{
    if (a < 0)
        return IPGUI_ERR_PARAM;

    if ((!r_int) && (!r_frac_1000))
        return IPGUI_ERR_PARAM;

    /* 下面的迭代算法对0和1不适用 */
    if ((0 == a) || (1 == a)) {
        if (r_int) * r_int = a;
        if (r_frac_1000) * r_frac_1000 = 0;
        return IPGUI_ERR_OK;
    }

    /* 牛顿迭代法 */
    s64_t x_iter;
    x_iter = ipgui_guess_sqrt_iter_init(a);/* 选合适的初始值 */
    s64_t fx = (s64_t)x_iter * (s64_t)x_iter - a;
    s64_t inte;
    s32_t fract;

    s64_t last_fx;
    for (;;) {
        /* 使用整数部分迭代 */
        inte = (-fx) / (2 * x_iter);
        x_iter = inte + x_iter;
        fx = (s64_t)x_iter * (s64_t)x_iter - a;
        if (fx == last_fx)
            break;
        last_fx = fx;
    }
    /* 测试发现，到这里已经可以保证一定的精度
     * 但是a越小，精度越低，所以还需要再迭代一下
     */
    fract = (-fx) % (2 * x_iter);

    s32_t temp_int, temp_frac;
    if (fract < 0) {
        temp_int = x_iter - 1;
        /* 1 - fract/(2 * x_iter) / 1 * 1000 % */
        temp_frac = 1000 - (-fract * 1000) / (2 * x_iter);
    } else if (fract > 0) {
        temp_int = x_iter;
        temp_frac = (fract * 1000) / (2 * x_iter);
    }
    else {
        temp_int = x_iter;
    }
    x_iter = temp_int * 1000 + temp_frac;
    fx = (s64_t)((s32_t)x_iter * (s32_t)x_iter) - (s64_t)(a * 1000000);
    inte = (-fx) / (2 * x_iter);
    x_iter = inte + x_iter;

    if (r_int) * r_int = x_iter / 1000;
    if (r_frac_1000) * r_frac_1000 = x_iter % 1000;

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_int_sqrt_optimized(
    s32_t a, s32_t * r_int, s32_t * r_frac_1000)
{
    ipgui_err_t ret;
    if (a < 50) {    
        s32_t temp;
        ret = ipgui_int_sqrt(a * 64, r_int, r_frac_1000);
        if(ret != IPGUI_ERR_OK) 
            return ret;

        temp = (*r_int) * 1000 + (*r_frac_1000);
        temp >>= 3;
        *r_int = temp / 1000;
        *r_frac_1000 = temp % 1000;
        return IPGUI_ERR_OK;
    }
    ret = ipgui_int_sqrt(a, r_int, r_frac_1000);
    return ret;
}

/* 求log(a)的值，转化为2^x = a--->即求f(x) = 2^x - a = 0 的解 */
__IPGUI_API__ ipgui_err_t ipgui_log2(s32_t a, s32_t * r_i, s32_t * r_f)
{
    s32_t abs_a = a < 0 ? (-a) : a;
    
}