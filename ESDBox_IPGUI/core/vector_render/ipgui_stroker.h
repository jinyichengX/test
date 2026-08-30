#ifndef IPGUI_STROKER_H
#define IPGUI_STROKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_color.h"
#include "ipgui_prim.h"
#include "ipgui_math.h"
#include "ipgui_line_clip.h"
#include "ipgui_vector.h"
#include "ipgui_screen.h"

/*   
      line join bevel                 line join miter                 line join round  
                                            .                             . .
                                           . .                           . . .
         . . . . .                        . . .                        . . . . .
        . . . . . .                      . . . .                      . . . . . .
       . . . . . . .                    . . . . .                    . . .   . . .
      . . . . . . . .                  . . . . . .                  . . .     . . .
     . . . .   . . . .                . . .   . . .                . . .       . . .
    . . . .     . . . .              . . .     . . .              . . .         . . .
   . . . .       . . . .            . . .       . . .            . . .           . . .
  . . . .         . . . .          . . .         . . .          . . .             . . .
*/

/* 线段之间的连接样式 */
typedef enum
{
    IPGUI_LINE_JOIN_ROUND,              /* 圆角连接 */
    IPGUI_LINE_JOIN_BEVEL,              /* 斜/平角连接 */
    IPGUI_LINE_JOIN_MITER,              /* 尖角连接 */
    IPGUI_LINE_JOIN_MAX
}ipgui_line_join_t;

/* 线段头/尾部（又称线帽）样式 */
// typedef enum
// {
//     IPGUI_LINE_CAP_BUTT,                /* 平顶线帽 */
//     IPGUI_LINE_CAP_ROUND,               /* 圆顶线帽 */
//     IPGUI_LINE_CAP_SQUARE,              /* 方形线帽头顶尖尖的 */
//     IPGUI_LINE_CAP_MAX
// }ipgui_line_cap_t;

typedef struct
{
    ipgui_scoord_t solid_len;            /* line dash solid length */
    ipgui_scoord_t gap_len;              /* line dash gap length */
}ipgui_line_dash_data_t;

typedef struct
{
    ipgui_line_dash_data_t * dashes;    /* line dash data */
    unsigned int dash_num;              /* line dash data number */
    ipgui_scoord_t offset;               /* line dash offset */
}ipgui_line_dash_t;

typedef struct
{
    ipgui_color_t color;                /* line color */
    unsigned char dash_on : 1;          /* line dash on/off */
    unsigned char reserved : 7;         /* reserved */
    ipgui_line_join_t join;             /* line join type */
    ipgui_scoord_t miter_limit;          /* line miter limit, this param only active when join type is miter */
    // ipgui_line_cap_t cap;               /* line cap */
    ipgui_scoord_t width;                /* line width */
    ipgui_line_dash_t * dash;           /* line dash, this param only active when dash_on is 1 */
    unsigned char opacity;              /* line opacity */
}ipgui_stroker_dsc_t;

__IPGUI_API__ ipgui_err_t ipgui_stroker_path_line_to(ipgui_stroker_dsc_t * stroker, ipgui_spoint_t from, ipgui_spoint_t to);

#ifdef __cplusplus
}
#endif

#endif 