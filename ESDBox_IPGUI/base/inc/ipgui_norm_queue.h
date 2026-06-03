#ifndef IPGUI_NORM_QUEUE_H
#define IPGUI_NORM_QUEUE_H

#include "ipgui_types.h"

typedef struct
{
  u16_t head;        /* index to read */
  u16_t tail;        /* index to write */  
  u16_t length;      /* valid data num */

  u8_t * pool;

  u16_t item_cnt;
  u16_t item_size;
}ipgui_norm_queue_t;

extern ipgui_err_t ipgui_norm_queue_init(ipgui_norm_queue_t * q, void * pool, u16_t item_cnt, u16_t item_size);
extern ipgui_err_t ipgui_norm_queue_post(ipgui_norm_queue_t * q, void * item, u16_t size);
extern ipgui_err_t ipgui_norm_queue_fetch(ipgui_norm_queue_t * q, void * item);

#endif