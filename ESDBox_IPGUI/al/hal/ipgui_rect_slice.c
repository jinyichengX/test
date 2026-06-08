#include "ipgui_rect_slice.h"
#include "ipgui_memory.h"

__IPGUI_API__ void ipgui_rect_slice_ctx_init(
    ipgui_rect_slice_ctx * ctx,
    ipgui_rect_t         * rect,
    ipgui_coord_t          slice_len)
{
    ipgui_memset(ctx, 0, sizeof(ipgui_rect_slice_ctx));
    ctx->rect         = rect;
    ctx->remain_w     = rect->end.x - rect->start.x + 1;
    ctx->remain_h     = rect->end.y - rect->start.y + 1;
    ctx->slice_len    = slice_len;

    ctx->full_w       = ctx->remain_w;
    ctx->full_h       = ctx->remain_h;
    ctx->orig_start_x = rect->start.x;
    ctx->orig_end_x   = rect->end.x;
    ctx->orig_bottom  = rect->end.y + 1;
}

/* 返回1，仍有切片剩余；返回0，切片已完成 */
__IPGUI_API__ s32_t ipgui_get_rect_slice(
    ipgui_rect_slice_ctx * ctx,
    ipgui_rect_t         * res)
{
    ipgui_coord_t strip_w;
    ipgui_coord_t rows;

    if (ctx->remain_w <= 0) {
        return 0;
    }

    if (ctx->remain_h <= 0) {
        ctx->remain_w -= IPGUI_MIN(ctx->remain_w, ctx->slice_len);
        ctx->remain_h  = ctx->full_h;
        if (ctx->remain_w <= 0) {
            return 0;
        }
    }

    strip_w = IPGUI_MIN(ctx->remain_w, ctx->slice_len);
    rows = IPGUI_MIN(ctx->remain_h, ctx->slice_len / strip_w);

    res->start.x = ctx->orig_start_x + (ctx->full_w - ctx->remain_w);
    res->start.y = ctx->orig_bottom  - ctx->remain_h;
    res->end.x   = res->start.x + strip_w - 1;
    res->end.y   = res->start.y + rows    - 1;

    ctx->remain_h -= rows;
    return 1;
}