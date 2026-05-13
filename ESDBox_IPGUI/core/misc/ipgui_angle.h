#ifndef __IPGUI_ANGLE_H__
#define __IPGUI_ANGLE_H__

typedef int ipgui_angle_t;

typedef enum {
    IPGUI_ANGLE_DIR_CW = 0, /* clockwise */
    IPGUI_ANGLE_DIR_CCW = 1 /* counter clockwise */
}ipgui_angle_dir_t;

/*
    cw example:
    ^       ^(start)        
    |      /
    |     /                  
    |    /                     cartesian coordinate
    |   / \                                  
    |  /   /                  
    | / ------------------->(end)                      
 (ori)----------------------->
    |
    |
    |                          screen coordinate
    |
    |
    |
*/

#define IPGUI_ANGLE_SUBBITS     16                              /* recommend 16*/
#define IPGUI_ANGLE_PRECISION   (1 << IPGUI_ANGLE_SUBBITS)      /* 角度精度，16倍精度 */

#define IPGUI_ANGLE_2PI         (360 * IPGUI_ANGLE_PRECISION)   /* 360度 */
#define IPGUI_ANGLE_PI          (180 * IPGUI_ANGLE_PRECISION)   /* 180度 */
#define IPGUI_ANGLE_PI2         (90  * IPGUI_ANGLE_PRECISION)   /* 90度 */
#define IPGUI_ANGLE_PI4         (45  * IPGUI_ANGLE_PRECISION)   /* 45度 */

/* fixed convert to float */
static inline float ipgui_angle_float(ipgui_angle_t angle)
{
    return (float)angle / (float)IPGUI_ANGLE_PRECISION;
}

/* float convert to fixed */
static inline ipgui_angle_t ipgui_angle_fixed(float angle)
{
    return (ipgui_angle_t)(angle * IPGUI_ANGLE_PRECISION);
}

#endif