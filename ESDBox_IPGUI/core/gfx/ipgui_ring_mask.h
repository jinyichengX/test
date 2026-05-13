#ifndef IPGUI_RING_MASK_H
#define IPGUI_RING_MASK_H

#include "ipgui_coord.h"
#include "ipgui_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ipgui_coord_t r;/* radius */
    u32_t r2;
    u32_t rmin2;
    u32_t rmax2;
    u32_t inv_out;
    u32_t inv_in;
}corner_param_t;

typedef struct {
    struct list_head   node;
    ipgui_coord_t      r;
    u16_t            * mask_index_at_y;
    ipgui_coord_t    * mask_start_x_at_y; 
    u16_t              dig_mask_index;
    ipgui_coord_t      dig_mask_start_xy;
    u8_t             * mask;
    
    /* 用于简单LRU淘汰 */
    u32_t              last_used_tick;           /* 最近使用时间戳 */
    u16_t              refcnt;                  /* 引用计数，未被使用可淘汰 */
}corner_mask_cache_item_t;

int ipgui_fetch_ring_mask(ipgui_coord_t x, ipgui_coord_t y, 
    ipgui_coord_t ir, ipgui_coord_t er,
    s8_t step,/* step = 1/-1 */
    u8_t * mask, u16_t len);
    
#ifdef __cplusplus
}
#endif

#endif