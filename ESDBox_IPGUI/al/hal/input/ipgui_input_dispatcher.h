#ifndef IPGUI_INPUT_DISPATCHER_H
#define IPGUI_INPUT_DISPATCHER_H

#include "ipgui_input_src.h"
#include "ipgui_screen.h"

typedef struct {
    ipgui_input_src_t * input_srcs;
    ipgui_scr_t       * screens;
}ipgui_input_dispatcher_t;

#endif