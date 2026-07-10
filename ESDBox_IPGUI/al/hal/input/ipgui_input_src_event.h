#ifndef IPGUI_INPUT_SRC_EVENT_H
#define IPGUI_INPUT_SRC_EVENT_H

#include "ipgui_coord.h"
#include "ipgui_time.h"

typedef s32_t ipgui_input_src_id_t;
typedef s32_t ipgui_scr_id_t;

typedef enum {
    IPGUI_INPUT_SRC_EVENT_NONE = 0,

    /* pointer(touch or pointer) */
    IPGUI_INPUT_SRC_EVENT_POINTER_PRESS,
    IPGUI_INPUT_SRC_EVENT_POINTER_RELEASE,

    /* key(board) */
    IPGUI_INPUT_SRC_EVENT_KEY_DOWN,
    IPGUI_INPUT_SRC_EVENT_KEY_UP,
}ipgui_input_evt_type_t;

typedef struct {
    ipgui_coord_t x;
    ipgui_coord_t y;
}ipgui_pointer_pos_t;

typedef struct {
    u32_t code;
}ipgui_key_code_t;

typedef struct {
    ipgui_input_src_id_t    input_src_id;
    ipgui_input_evt_type_t  input_src_evt;

    union {
        ipgui_pointer_pos_t pointer_pos;
        ipgui_key_code_t    key_code;
    }evt_info;

    ipgui_tick_t evt_tick;
}ipgui_input_src_evt_t;

#endif