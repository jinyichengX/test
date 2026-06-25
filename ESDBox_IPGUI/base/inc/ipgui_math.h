#ifndef IPGUI_MATH_H
#define IPGUI_MATH_H

#include "ipgui_utils.h"
#include "ipgui_angle.h"

extern const ipgui_angle_t ipgui_arctan_lut[23];
__IPGUI_API__ ipgui_err_t ipgui_int_sqrt(s32_t a, s32_t * r_int, s32_t * r_frac_1000);
__IPGUI_API__ ipgui_err_t ipgui_int_sqrt_optimized(s32_t a, s32_t * r_int, s32_t * r_frac_1000);
__IPGUI_API__ s32_t ipgui_sin(s32_t angle);
__IPGUI_API__ s32_t ipgui_cos(s32_t angle);

#endif