#include "ipgui_anim_liner.h"

ipgui_anim_value_t ipgui_anim_liner(struct ipgui_anim_t * anim, ipgui_tick_t t, void * data)
{
    return (ipgui_anim_value_t)t;
}
