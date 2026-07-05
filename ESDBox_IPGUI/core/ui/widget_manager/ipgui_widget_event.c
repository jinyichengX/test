#include "ipgui_widget.h"

/*事件回调函数列表

事件掩码 (支持哪些事件)

用户数据指针*/

// /* register widget events */
// __IPGUI_API__ int ipgui_widget_register_event(ipgui_widget_t * widget, unsigned int codes,
//                                             ipgui_widget_event_cb_t cb, void * args)
// {
//     ipgui_widget_event_handler_t handler;
//     handler.codes = codes;
//     handler.event_cb = cb;
//     handler.args = args;
//     if(-1 == ipgui_darray_element_append(&widget->user_event_cb, &handler, 1))
//         return -1;
//     else return 0;
// }

// /* handle widget events */
// __IPGUI_API__ int ipgui_widget_event_handler(ipgui_widget_t * widget, unsigned int codes)
// {
//     unsigned int idx;
//     ipgui_widget_event_handler_t * iter;
//     for (idx = 0; idx < widget->user_event_cb.elem_size; idx ++) {
//         iter = (ipgui_widget_event_handler_t *)\
//         ipgui_darray_index(&widget->user_event_cb, idx);
//         if ((!iter) || (!iter->event_cb)) continue;
//         unsigned int filt_code;
//         if (filt_code = (iter->codes & codes)) {
//             iter->event_cb(filt_code, iter->args);
//         } else continue;
//     }
//     return 0;
// }

__IPGUI_API__ void ipgui_widget_scroll_handler(ipgui_widget_t * widget, ipgui_widget_evt_t * evt)
{
    if (widget->flags & IPGUI_WIDGET_FLAG_SCROLLABLE) {
        // widget->scroll_x += evt->scroll_x;
        // widget->scroll_y += evt->scroll_y;
    }
}