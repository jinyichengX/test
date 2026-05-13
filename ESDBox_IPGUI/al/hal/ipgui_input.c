#include "ipgui_input.h"
#include "ipgui_debug.h"
#include "ipgui_widget.h"

__IPGUI_STATIC__ void ipgui_input_dev_read_timer(ipg_tmr_t * timer, void * args);
__IPGUI_STATIC__ void ipgui_widget_pid_event_handler(ipgui_widget_t * widget, ipgui_pid_event_t * pid_event);
__IPGUI_API__ int
ipgui_input_device_register(ipgui_input_drv_t * drv, \
                            ipgui_scr_t * scr, \
                            ipgui_tick_t read_priod)
{
    if (!drv || !scr) {
        return -1;
    }

    ipgui_input_t * dev = \
    ipgui_mem_alloc_def(sizeof(ipgui_input_t));
    if (!dev) {
        return -2;
    }

    dev->tmr = (ipg_tmr_t *)0;
    ipgui_timer_create(&dev->tmr, read_priod, 
        IPGUI_TIME_FOREVER, 
        ipgui_input_dev_read_timer, (void *)&dev->tmr);

    if (!dev->tmr) {
        ipgui_mem_free_def(dev);
        return -2;
    }

    dev->drv = drv;
    dev->scr_owner = scr;
    list_head_init(&dev->node);
    list_add_tail(&dev->node, &scr->input_dev);
    ipgui_memset(&dev->cooked, 0, sizeof(ipgui_input_data_cooked_t));

    ipgui_timer_start_def(dev->tmr, 0);

    return 0;
}

__IPGUI_STATIC__ int 
ipgui_pid_handler(ipgui_input_t * input_dev, ipgui_pid_t * pid_data)
{
#define pid_cooked_data input_dev->cooked.data.pid_data
    ipgui_scr_t * scr;
    ipgui_widget_t * hitted;
    ipgui_widget_event_code_t e_collec = 0;
    ipgui_pid_event_t pid_event;

    scr = input_dev->scr_owner;
    if (pid_data->state == IPGUI_PRESS_STATE_DOWN) {
        ipgui_point_t pressed_pos;
        pressed_pos.x = pid_data->x;
        pressed_pos.y = pid_data->y;
        
        /* handler */
        pid_event.first_press = 0; /* reset first press flag */
        if (!pid_cooked_data.last_state) { /* recalculate hitted */
            hitted = ipgui_widget_topest_on(scr->cur_screen, &pressed_pos);
            if (hitted == (ipgui_widget_t *)0) { /* panic */
                ipgui_dbg_error("no widget hitted at %d, %d\n", pressed_pos.x, pressed_pos.y);
                return -1;
            }
            pid_event.first_press = 1; /* first press */
        } else {
            hitted = pid_cooked_data.grabbed;
        }
        pid_event.dx = 0; /* reset offset x */
        pid_event.dy = 0; /* reset offset y */
        if (hitted == pid_cooked_data.grabbed) {
            /* if same widget, add pressing tick */
            pid_cooked_data.pressing_tick += input_dev->tmr->unPeriod;
            pid_event.dx = pressed_pos.x - pid_cooked_data.last_pressed_pos.x; /* calculate offset */
            pid_event.dy = pressed_pos.y - pid_cooked_data.last_pressed_pos.y;
        }
        pid_event.input_dev = input_dev;
        pid_event.pressed = pid_data->state;
        pid_event.global_pos.x = pid_data->x;
        pid_event.global_pos.y = pid_data->y;
        pid_event.pressing_tick = pid_cooked_data.pressing_tick;
        /* get local pos */
        pid_event.local_pos = ipgui_widget_global2_visible_local_offset(hitted, &pressed_pos);

        /* update cooked data */
        pid_cooked_data.grabbed = hitted;
        pid_cooked_data.last_pressed_pos = pressed_pos;
        ipgui_printk("pid_event: %s, local pos: %d, %d, tick: %d, offset: x = %d, y = %d\n",
                 "pressed",
                pid_event.local_pos.x, pid_event.local_pos.y,
                pid_event.pressing_tick,
                pid_event.dx, pid_event.dy);
        /* dispatch pid event */
        ipgui_widget_pid_event_handler(hitted, &pid_event);

    } else if (pid_data->state == IPGUI_PRESS_STATE_UP) {
        pid_cooked_data.grabbed = (ipgui_widget_t *)0;
        pid_cooked_data.pressing_tick = 0;
        pid_event.input_dev = input_dev;
        pid_event.pressed = 0;
        if (pid_cooked_data.last_state) pid_event.unpressed_hover = 0; /* represent released */
        else pid_event.unpressed_hover = 1; /* represent hover */ 
        pid_event.global_pos.x = pid_data->x;
        pid_event.global_pos.y = pid_data->y;
        /* dispatch pid event */
        ipgui_widget_pid_event_handler(hitted, &pid_event);
    }
    pid_cooked_data.last_state = pid_data->state;
    return 0;
}

__IPGUI_STATIC__ void 
ipgui_widget_pid_event_handler(ipgui_widget_t * widget, ipgui_pid_event_t * pid_event)
{
    if (pid_event->pressed == 1) { /* press down */
        if (!ipgui_widget_is_flag_set(widget, IPGUI_WIDGET_FLAG_FIXED)) { /* allow drag */
            widget->rect.start.x += pid_event->dx;
            widget->rect.start.y += pid_event->dy;
            widget->rect.end.x += pid_event->dx;
            widget->rect.end.y += pid_event->dy;
        }
    } else {/* released up */
        /* if local point is in the widget visible region */
        if (pid_event->pressing_tick) {
            
        }
    }


    /* emit widget event to user's registered callback */
}

__IPGUI_STATIC__ void 
ipgui_kbid_handler(ipgui_input_t * input_dev, ipgui_kbid_t * kbid_data)
{

}

__IPGUI_STATIC__ void
ipgui_input_dev_read_timer(ipg_tmr_t * timer, void * args)
{
    /* get screen */
    ipgui_scr_t * scr;
    ipgui_input_t * input_dev;
    ipgui_input_data_t input_data;

    input_dev = (ipgui_input_t *)
        ipgui_container_of(args, ipgui_input_t, tmr);
                
    ipgui_dbg_assert(input_dev != (ipgui_input_t *)0,
        "input_device is NULL");

    if (!input_dev->drv->read)
        return;

    scr = input_dev->scr_owner;
    if (!scr) return;
    if (input_dev->drv->read(input_dev->drv, &input_data)) {
        ipgui_dbg_warning("input device read failed!\n");
        return;
    }
    /* post handler */
    if (input_dev->drv->type == IPGUI_INPUT_TYPE_PID) {
        ipgui_pid_handler(input_dev, &input_data.data.pid);
    } else if (input_dev->drv->type == IPGUI_INPUT_TYPE_KBID) {
        ipgui_kbid_handler(input_dev, &input_data.data.kbid);
    } else {
        ipgui_dbg_warning("unknown input device type!\n");
        return;
    }
}

