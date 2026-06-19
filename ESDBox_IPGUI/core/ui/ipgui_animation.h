#ifndef IPGUI_ANIMATION_H
#define IPGUI_ANIMATION_H

#include "ipgui_time.h"

typedef enum {
    IPGUI_ANIM_LOOP_TYPE_NONE = 0,
    IPGUI_ANIM_LOOP_TYPE_ONCE = 1,
    IPGUI_ANIM_LOOP_TYPE_FORWARD = 2, /* 正向循环 */
}ipgui_anim_loop_type_t;

typedef struct {
    ipgui_tick_t duration; /* 一帧动画持续时间 */
    ipgui_anim_loop_type_t loop_type; /* 循环类型 */
    u32_t loop_count; /* 循环次数 */
    ipgui_tick_t loop_delay; /* 循环延迟时间 */ 
}ipgui_anim_dsc_t;

#endif