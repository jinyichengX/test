#ifndef IPGUI_INPUT_SRC_H
#define IPGUI_INPUT_SRC_H

#include "ipgui_types.h"

typedef struct ipgui_input_drv_ctx {
    void * priv_data;               /* private data */
    ipgui_input_type_t type;        /* input device type */
    int (*read)(struct ipgui_input_drv_t * dev, ipgui_input_data_t * data); /* read input data */
}ipgui_input_drv_t;

typedef struct {
    void * data;
    u8_t id;
    u8_t enabled;
    void (*init)(void);
    void (*read)(input_event_t * ev);
    bool (*filter)(input_event_t * ev); // 设备级事件过滤
}ipgui_input_src_t;

#endif