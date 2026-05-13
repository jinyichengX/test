#ifndef IPGUI_HANDLER_H 
#define IPGUI_HANDLER_H

#include "ipgui_utils.h"
#include "ipgui_screen.h"
#include "ipgui_coord.h"
IPGUI_HEADER_BEGIN _______________MARKER_______________
typedef enum
{
    IPGUI_POST_EVENT_IDLE = 0,
    IPGUI_POST_EVENT_HOVER = 0x01,
    IPGUI_POST_EVENT_CLICKED = 0x02,
    IPGUI_POST_EVENT_DRAG = 0x04,

    /* global event */
    IPGUI_POST_EVENT_FOCUS_CANCEL = 0x08,
    IPGUI_POST_EVENT_FOCUS_LEFT = 0x09,
    IPGUI_POST_EVENT_FOCUS_RIGHT = 0x0A,
    IPGUI_POST_EVENT_FOCUS_UP = 0x0B,
    IPGUI_POST_EVENT_FOCUS_DOWN = 0x0C,
    IPGUI_POST_EVENT_FOCUS_NEXT = 0x0D,
}ipgui_post_evt_e;
extern struct input_device_id event_dispatcher_id_table;
ipgui_err_t ipgui_input_event_handler(ipgui_input_handle_t * handle, ipgui_input_event_t * evts, unsigned int count);
int ipgui_input_event_filter(ipgui_input_handle_t * handle, unsigned int type, unsigned int code, unsigned int value);
extern ipgui_input_handler_t event_dispatcher;
IPGUI_HEADER_END   _______________MARKER_______________

#endif