#ifndef IPGUI_RECT_SLICE_H
#define IPGUI_RECT_SLICE_H

#include "ipgui_prim.h"

typedef struct {
    ipgui_rect_t * rect;
    
    ipgui_coord_t remain_w;
    ipgui_coord_t remain_h;

    ipgui_coord_t slice_len;

    ipgui_coord_t full_w;        /* width of rect */
    ipgui_coord_t full_h;        /* height of rect */

    ipgui_coord_t orig_start_x;  /* rect->start.x */
    ipgui_coord_t orig_end_x;    /* rect->end.x   */
    ipgui_coord_t orig_bottom;   /* rect->end.y + 1，current top y = orig_bottom - remain_h */
}ipgui_rect_slice_ctx;

extern __IPGUI_API__ void ipgui_rect_slice_ctx_init(
    ipgui_rect_slice_ctx * ctx,
    ipgui_rect_t         * rect,
    ipgui_coord_t          slice_len);

extern __IPGUI_API__ s32_t ipgui_get_rect_slice(
    ipgui_rect_slice_ctx * ctx,
    ipgui_rect_t         * res);

#endif