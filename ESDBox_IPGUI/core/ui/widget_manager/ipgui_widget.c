#include "ipgui_widget.h"
#include "ipgui_memory.h"
#include "ipgui_defs.h"

__IPGUI_API__ ipgui_widget_t * ipgui_widget_create(ipgui_widget_t * parent)
{
    if (!parent) {
        // if( scr != ipgui_widget_get_screen(parent))
        //     return (ipgui_widget_t *)0;
    }
    
    ipgui_widget_t * widget = (ipgui_widget_t *)ipgui_mem_alloc_def(sizeof(ipgui_widget_t));

    if (widget)
    {

    }
    
    return widget;
}



