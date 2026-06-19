#ifndef IPGUI_INPUT_SRC_H
#define IPGUI_INPUT_SRC_H

#include "ipgui_types.h"
#include "ipgui_widget_evt.h"
#include "ipgui_input_src_event.h"

typedef ipgui_err_t (* convert_event_cb_t)(void * priv_data, 
                          ipgui_input_src_evt_t * raw_evt,
                          ipgui_widget_evt_t * widget_evt);

typedef struct {
    void * priv_data;
    
    /* 读取原始事件 return IPGUI_ERR_READ_INPUT_SRC_EVT_OK/ERR */
    ipgui_err_t (*input_src_event_read_cb)(void * priv_data, ipgui_input_src_evt_t * raw_evt);
}ipgui_input_src_t;

#endif