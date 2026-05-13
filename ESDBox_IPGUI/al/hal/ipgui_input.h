#ifndef IPGUI_INPUT_H
#define IPGUI_INPUT_H

#include "ipgui_coord.h"
#include "ipgui_list.h"
#include "ipgui_timer.h"
#include "ipgui_utils.h"
#include "ipgui_prim.h"
#include "ipgui_memory.h"
#include "ipgui_screen.h"

IPGUI_HEADER_BEGIN _______________MARKER_______________

typedef struct ipgui_scr_ctx ipgui_scr_t;

typedef enum {
    IPGUI_INPUT_TYPE_PID = 0,   /* pointer input device */
    IPGUI_INPUT_TYPE_KBID,       /* keyboard input device */
}ipgui_input_type_t;

typedef enum {
    IPGUI_PRESS_STATE_UP = 0,
    IPGUI_PRESS_STATE_DOWN,
}ipgui_press_state_t;

typedef struct {
    ipgui_coord_t x;
    ipgui_coord_t y;
    ipgui_press_state_t state;
}ipgui_pid_t;               /* pointer input data */

typedef  struct {
    unsigned int key_code;
    ipgui_press_state_t state;
}ipgui_kbid_t;              /* keyboard input data */

typedef struct {
    ipgui_input_type_t type;        /* input device type */
    union {
        ipgui_pid_t pid;            /* pointer input data */
        ipgui_kbid_t kbid;          /* keyboard input data */
    } data;                          /* input data */
}ipgui_input_data_t;

typedef struct {
    union {
        struct {
            ipgui_press_state_t last_state; /* 按下1或释放0 */
            ipgui_point_t last_pressed_pos;
            ipgui_widget_t * grabbed;   /* 当前抓取的控件，直到鼠标释放/触摸手指抬起清零 */
            ipgui_tick_t pressing_tick; /* 同一控件内连续按下时间 */
        }pid_data;
        struct {
            unsigned int key_code;      /* 键盘按键码 */
            unsigned char is_pressed : 1; /* 是否按下 */
        }kbid_data;
    }data;
}ipgui_input_data_cooked_t;

typedef struct ipgui_input_drv_ctx ipgui_input_drv_t;
typedef struct ipgui_input_drv_ctx {
    void * priv_data;               /* private data */
    ipgui_input_type_t type;        /* input device type */
    int (*read)(struct ipgui_input_drv_t * dev, ipgui_input_data_t * data); /* read input data */
}ipgui_input_drv_t;

typedef struct {
    struct list_head node;          /* input device node */ /* 没什么用 */
    ipgui_scr_t * scr_owner;        /* screen owner */  /* 指向屏幕，而不是在屏幕结构体中指向输入设备，是因为read之后需要立即索引到屏幕 */
    ipgui_input_drv_t * drv;        /* input device driver */
    ipg_tmr_t * tmr;
    ipgui_input_data_cooked_t cooked; /* cooked input data */
}ipgui_input_t;

typedef struct {
    unsigned char pressed : 1; /* 按下1或释放0 */
    unsigned char first_press : 1; /* 首次按下 */
    unsigned char unpressed_hover : 1; /* 按下松开瞬间/未按下时的悬停 */
    ipgui_tick_t pressing_tick; /* 连续按下时间 */
    ipgui_input_t * input_dev; /* 引起事件的输入设备 */
    ipgui_point_t local_pos; /* 作用于的控件的局部坐标（相对于控件内部的的0,0） */
    ipgui_point_t global_pos; /* 屏幕全局坐标 */
    ipgui_coord_t dx; /* 相对于上次位置的x偏移 */
    ipgui_coord_t dy; /* 相对于上次位置的y偏移 */
}ipgui_pid_event_t;

typedef struct {

}ipgui_kbid_event_t;

__IPGUI_API__ int
ipgui_input_device_register(ipgui_input_drv_t * drv, \
                            ipgui_scr_t * scr, \
                            ipgui_tick_t read_priod);

IPGUI_HEADER_END _______________MARKER_______________

#endif