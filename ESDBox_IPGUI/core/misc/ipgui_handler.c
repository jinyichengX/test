/*
 * MIT License
 *
 * Copyright (c) 2025 JinYiCheng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ipgui_handler.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"
#include "ipgui_prim.h"
#include "ipgui_widget.h"


extern ipgui_widget_t * wid1; 
extern ipgui_widget_t * wid2; 
struct input_device_id event_dispatcher_id_table = {
    .driver_info = 1,
};

__IPGUI_STATIC__ void ipgui_input_event_post_handler(ipgui_post_event_t * event)
{
    ipgui_post_evt_e evt;
    if( event->key_code_pressed )
    {
        switch (event->key_code)
        {
            case IPGUI_EVT_KEY_LEFT:
                evt = IPGUI_POST_EVENT_FOCUS_LEFT;
                break;
            case IPGUI_EVT_KEY_RIGHT:
                evt = IPGUI_POST_EVENT_FOCUS_RIGHT;
                break;
            case IPGUI_EVT_KEY_UP:
                evt = IPGUI_POST_EVENT_FOCUS_UP;
                break;
            case IPGUI_EVT_KEY_DOWN:
                evt = IPGUI_POST_EVENT_FOCUS_DOWN;
                break;
            case IPGUI_EVT_KEY_TAB:
                evt = IPGUI_POST_EVENT_FOCUS_NEXT;
                break;
            default:
                break;
        }
    }
}

ipgui_err_t ipgui_input_event_handler(ipgui_input_handle_t * handle, ipgui_input_event_t * evts, unsigned int count)
{

}

int ipgui_input_event_filter(ipgui_input_handle_t * handle, unsigned int type, unsigned int code, unsigned int value)
{
    return 0;
}