#ifndef IPGUI_WIDGET_EVENT_H
#define IPGUI_WIDGET_EVENT_H

#include "ipgui_types.h"

typedef enum {
    WIDGET_EVENT_NONE = 0,
    
    // 触摸事件
    WIDGET_EVENT_PRESS,
    WIDGET_EVENT_RELEASE,
    WIDGET_EVENT_LONG_PRESS,
    WIDGET_EVENT_CLICK,         // 短按后抬起
    WIDGET_EVENT_DRAG_BEGIN,
    WIDGET_EVENT_DRAGGING,
    WIDGET_EVENT_DRAG_END,
    
    // 手势事件
    WIDGET_EVENT_SWIPE_UP,
    WIDGET_EVENT_SWIPE_DOWN,
    WIDGET_EVENT_SWIPE_LEFT,
    WIDGET_EVENT_SWIPE_RIGHT,
    
    // 按键事件
    WIDGET_EVENT_KEY_DOWN,
    WIDGET_EVENT_KEY_UP,
    WIDGET_EVENT_KEY_REPEAT,
    
    // 焦点事件
    WIDGET_EVENT_FOCUS_IN,
    WIDGET_EVENT_FOCUS_OUT,
} widget_event_type_t;

typedef struct {
    widget_event_type_t type;
    s32_t x;                    // 控件内相对坐标
    s32_t y;
    s32_t delta_x;              // 拖动增量
    s32_t delta_y;
    u32_t key_code;             // 按键码
    u32_t timestamp;
    void *target_widget;        // 目标控件（分发时填充）
} ipgui_widget_evt_t;

#endif