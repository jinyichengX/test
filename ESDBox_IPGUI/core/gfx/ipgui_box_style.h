#ifndef IPGUI_BOX_SHAPE_H
#define IPGUI_BOX_SHAPE_H

#include "ipgui_coord.h"

typedef struct {
    /* padding */
    ipgui_coord_t left_padding;
    ipgui_coord_t right_padding;
    ipgui_coord_t top_padding;
    ipgui_coord_t bottom_padding;

    /* corner radius
     * 圆角是padding box的圆角
     * 不是content box或者border box的圆角 
     */
    ipgui_coord_t left_top_radius;
    ipgui_coord_t right_top_radius;
    ipgui_coord_t left_bottom_radius;
    ipgui_coord_t right_bottom_radius;
} ipgui_box_style_t;

#endif