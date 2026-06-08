#ifndef IPGUI_RECT_SLICE_H
#define IPGUI_RECT_SLICE_H

#include "ipgui_prim.h"

typedef struct {
    ipgui_rect_t * rect;
    ipgui_coord_t remain_w;
    ipgui_coord_t remain_h;

    ipgui_coord_t slice_len;

}ipgui_rect_slice_ctx;

extern __IPGUI_API__ void ipgui_rect_slice_ctx_init(
    ipgui_rect_slice_ctx * ctx,
    ipgui_rect_t         * rect,
    ipgui_coord_t          slice_len);

extern __IPGUI_API__ s32_t ipgui_get_rect_slice(
    ipgui_rect_slice_ctx * ctx,
    ipgui_rect_t         * res);

#endif