#ifndef IPGUI_ANIM_LINER_H
#define IPGUI_ANIM_LINER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_animation.h"

/* 线性动画: output = t */
ipgui_anim_value_t ipgui_anim_liner(struct ipgui_anim_t * anim, ipgui_tick_t t, void * data);

#ifdef __cplusplus
}
#endif

#endif
