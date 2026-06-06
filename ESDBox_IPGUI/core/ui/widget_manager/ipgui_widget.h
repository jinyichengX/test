#ifndef IPGUI_WIDGET_H
#define IPGUI_WIDGET_H

#include "ipgui_utils.h"
#include "ipgui_types.h"
#include "ipgui_prim.h"
#include "ipgui_widget_evt.h"
#include "ipgui_widget_tree.h"

typedef struct ipgui_widget
{
    /* private data */
    void                 * priv_data;

    /* the link node to widget tree */
    struct widget_link_t   link;

    /* position and size */ /* 控件在父控件内的位置和大小，与父控件区域联合裁剪绘制区 */
    ipgui_coord_t          x, y;
    ipgui_coord_t          w, h;

    /* callback functions */
    void (*render)       (struct ipgui_widget * widget);
    void (*event_handler)(struct ipgui_widget * widget, ipgui_widget_evt_t * evt);
}ipgui_widget_t;

extern __IPGUI_API__ ipgui_widget_t * ipgui_widget_create(ipgui_widget_t * parent);

#endif

