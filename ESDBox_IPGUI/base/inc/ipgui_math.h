#ifndef __IPGUI_MATH_H
#define __IPGUI_MATH_H

#include "ipgui_utils.h"
#include "ipgui_types.h"
#include "ipgui_angle.h"
extern const ipgui_angle_t ipgui_arctan_lut[23];
__IPGUI_API__ ipgui_err_t ipgui_int_sqrt(int a, int * r_int, int * r_frac_1000);
__IPGUI_API__ ipgui_err_t ipgui_int_sqrt_optimized(int a, int * r_int, int * r_frac_1000);
__IPGUI_API__ int ipgui_sin(int angle);
__IPGUI_API__ int ipgui_cos(int angle);
#endif