#ifndef IPGUI_DIRTY_RECT_H
#define IPGUI_DIRTY_RECT_H

#include "ipgui_prim.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IPGUI_DIRTY_RECT_POOL
#define IPGUI_DIRTY_RECT_POOL  8
#endif

/* -------------------------------------------------------------------------
 * 合并代价阈值
 *
 * 两个矩形合并后，若"新增的多余像素数"超过此阈值，则不合并（保持分离）。
 * 设为 0 表示只要有额外像素就不合并（最精确）；
 * 设为 INT32_MAX 表示总是合并（最激进）。
 *
 * 建议取值：屏幕总像素的 1%~5%，例如 240x320 屏可设 768（1%）。
 * ---------------------------------------------------------------------- */
#ifndef IPGUI_MERGE_COST_THRESHOLD
#define IPGUI_MERGE_COST_THRESHOLD  0
#endif

typedef struct {
    ipgui_coord_t x1, y1;
    ipgui_coord_t x2, y2;
} ipgui_dirty_rect_t;

/* dirty rect manager */
typedef struct {
    ipgui_dirty_rect_t pool[IPGUI_DIRTY_RECT_POOL];
    s32_t                pool_num;
} ipgui_dirty_rect_mgr_t;

__IPGUI_API__ void ipgui_dirty_rect_mgr_init(ipgui_dirty_rect_mgr_t * mgr);
__IPGUI_API__ void ipgui_dirty_rect_mgr_reset(ipgui_dirty_rect_mgr_t * mgr);
__IPGUI_API__ void ipgui_dirty_rect_add(ipgui_dirty_rect_mgr_t * mgr, ipgui_dirty_rect_t * dr);
__IPGUI_API__ void ipgui_dirty_rect_add_xywh(ipgui_dirty_rect_mgr_t * mgr, ipgui_coord_t x, ipgui_coord_t y, ipgui_coord_t w, ipgui_coord_t h);
__IPGUI_API__ void ipgui_dirty_rect_flush(ipgui_dirty_rect_mgr_t * mgr);
__IPGUI_API__ ipgui_dirty_rect_t * ipgui_dirty_rect_get(ipgui_dirty_rect_mgr_t * mgr, s32_t index);
__IPGUI_API__ s32_t ipgui_dirty_rect_is_dirty(ipgui_dirty_rect_mgr_t * mgr, ipgui_dirty_rect_t * dr);

#ifdef __cplusplus
}
#endif
#endif