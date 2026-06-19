#ifndef IPGUI_WIDGET_EVENT_H
#define IPGUI_WIDGET_EVENT_H

#include "ipgui_types.h"
#include "ipgui_coord.h"

typedef struct ipgui_widget ipgui_widget_t;

typedef enum {
    IPGUI_WIDGET_EVENT_PRESSED,
    IPGUI_WIDGET_EVENT_RELEASED,
    IPGUI_WIDGET_EVENT_HOVER,
}ipgui_widget_event_type_t;

/* 按压事件（鼠标或单点触摸）（包括按下瞬间和持续按压） */
typedef struct {
    ipgui_coord_t x, y; /* 按压时（不是按压瞬间）的坐标 */
    ipgui_coord_t first_press_x, first_press_y;
    ipgui_coord_t last_press_x, last_press_y;
}ipgui_widget_pressed_evt_t;

/* 释放事件（鼠标或单点触摸）（仅在释放瞬间触发） */
typedef struct {
    ipgui_coord_t x, y; /* 释放瞬间的坐标 */
    ipgui_coord_t first_press_x, first_press_y;
}ipgui_widget_released_evt_t;

/* 悬停事件（仅对鼠标有效，见ipgui_default_event_converter） */
typedef struct {
    ipgui_coord_t x, y; /* 悬停时的坐标 */
}ipgui_widget_hover_evt_t;

typedef struct {
    ipgui_widget_t * target;
    ipgui_widget_event_type_t type;
    union {
        ipgui_widget_pressed_evt_t pressed_evt;
        ipgui_widget_released_evt_t released_evt;
        ipgui_widget_hover_evt_t hover_evt;
    }evt;
} ipgui_widget_evt_t;

#endif