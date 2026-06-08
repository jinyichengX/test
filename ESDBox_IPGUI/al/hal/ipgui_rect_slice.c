#include "ipgui_rect_slice.h"
#include "ipgui_memory.h"

__IPGUI_API__ void ipgui_rect_slice_ctx_init(
    ipgui_rect_slice_ctx * ctx,
    ipgui_rect_t         * rect,
    ipgui_coord_t          slice_len)
{
    ipgui_memset(ctx, 0, sizeof(ipgui_rect_slice_ctx));
    ctx->rect      = rect;
    ctx->remain_w  = rect->end.x - rect->start.x + 1;
    ctx->remain_h  = rect->end.y - rect->start.y + 1;
    ctx->slice_len = slice_len;
}

/* 返回1，仍有切片剩余；返回0，切片已完成 */
__IPGUI_API__ s32_t ipgui_get_rect_slice(
    ipgui_rect_slice_ctx * ctx,
    ipgui_rect_t         * res)
{
    ipgui_coord_t max_row;
    max_row = ctx->slice_len / ctx->remain_w;
    if (max_row == 0) max_row = 1;


}