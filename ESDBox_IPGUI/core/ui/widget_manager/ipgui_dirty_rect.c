#include "ipgui_dirty_rect.h"
#include "ipgui_utils.h"
#include "ipgui_debug.h"

/* normalize rect x1 x2 y1 y2 */
__IPGUI_STATIC__ void dirty_rect_normalize(ipgui_dirty_rect_t * r)
{
    ipgui_coord_t temp;
    if (r->x1 > r->x2) {
        temp = r->x1;
        r->x1 = r->x2;
        r->x2 = temp;
    }
    if (r->y1 > r->y2) {
        temp = r->y1;
        r->y1 = r->y2;
        r->y2 = temp;
    }
}

/* get dirty rect area */
__IPGUI_STATIC__ int dirty_rect_area(ipgui_dirty_rect_t * r)
{
    ipgui_dirty_rect_t temp = * r;
    dirty_rect_normalize(&temp);
    return (int)(temp.x2 - temp.x1 + 1) *
           (temp.y2 - temp.y1 + 1);
}

/* merge two dirty rect */
__IPGUI_STATIC__ ipgui_dirty_rect_t dirty_rect_merge(
    ipgui_dirty_rect_t * a,
    ipgui_dirty_rect_t * b)
{
    ipgui_dirty_rect_t m;
    ipgui_dirty_rect_t temp_a = * a;
    ipgui_dirty_rect_t temp_b = * b;

    dirty_rect_normalize(&temp_a);
    dirty_rect_normalize(&temp_b);
    m.x1 = IPGUI_MIN(temp_a.x1, temp_b.x1);
    m.y1 = IPGUI_MIN(temp_a.y1, temp_b.y1);
    m.x2 = IPGUI_MAX(temp_a.x2, temp_b.x2);
    m.y2 = IPGUI_MAX(temp_a.y2, temp_b.y2);
    return m;
}

/* b是否被a完全包含 */
__IPGUI_STATIC__ int dirty_rect_contains(
    ipgui_dirty_rect_t * a,
    ipgui_dirty_rect_t * b)
{
    return (a->x1 <= b->x1 && a->y1 <= b->y1 &&
            a->x2 >= b->x2 && a->y2 >= b->y2);
}

/* the cost of merging two rectangles
 * if result < 0, the area of merged rect is less than the area of a and b
 * if result > 0, the area of merged rect is more than the area of a and b
 * if result = 0, the area of merged rect is equal to the area of a and b
 */
__IPGUI_STATIC__ int dirty_rect_merge_cost(
    ipgui_dirty_rect_t * a,
    ipgui_dirty_rect_t * b)
{
    ipgui_dirty_rect_t u = dirty_rect_merge(a, b);
    return dirty_rect_area(&u) - dirty_rect_area(a) - dirty_rect_area(b);
}

/* remove dirty rect(the idx) from array */
__IPGUI_STATIC__ void arr_remove(
    ipgui_dirty_rect_t * arr, 
    int * num, int idx)
{
    (* num) --;
    if (idx != * num) {
        arr[idx] = arr[* num];
    }
}

/* 步骤1：在数组中和找代价最小（合并后面积最小）的两两合并对
 * 比较次数为C(n,2)即n(n-1)/2次
 * 步骤2：在数组中和找代价最小（合并后面积最小）的和新矩形的合并对
 * 比较次数为n次
 * 步骤3：找出最小的合并代价的那对进行合并
 */
__IPGUI_STATIC__ void arr_merge_best_pair(
    ipgui_dirty_rect_t * arr, int * num, ipgui_dirty_rect_t * new_dr)
{
    int best_i = 0, best_j = 1;
    int best_cost = 0x7fffffff;
    int cost;

    for (int i = 0; i < * num; i ++) {
        for (int j = i + 1; j < * num; j ++) {
            cost = dirty_rect_merge_cost(&arr[i], &arr[j]);
            if (cost < best_cost) {
                best_cost = cost;
                best_i = i;
                best_j = j;
            }
        }
    }

    int old_best_cost = best_cost;
    int old_best_i = best_i;
    for (int i = 0; i < * num; i ++) {
        cost = dirty_rect_merge_cost(&arr[i], new_dr);
        if (cost < best_cost) {
            best_cost = cost;
            best_i = i;
        }
    }
    if (best_cost >= old_best_cost) {
        /* merge arr[best_j] into arr[best_i] 
         * and then remove arr[best_j]
         */
        arr[old_best_i] = dirty_rect_merge(&arr[old_best_i], &arr[best_j]);
        arr_remove(arr, num, best_j);
        arr[(* num) ++] = * new_dr;
    } else {
        /* merge new_dr into arr[best_i] */
        arr[best_i] = dirty_rect_merge(&arr[best_i], new_dr);
    }
    if (best_cost > 0) {
        // ipgui_dbg_warning("warning(dirty_rect): it is a bad deal!\r\n");
    }
}

/* add new dirty rect to pool */
__IPGUI_STATIC__ void pool_add(
    ipgui_dirty_rect_mgr_t * mgr,
    ipgui_dirty_rect_t dr)
{
    /* check if dr is contained by any rect in pool 
     * if so, discard the dr and return
     */
    int i = 0;
    for (; i < mgr->pool_num; i ++) {
        if (dirty_rect_contains(&mgr->pool[i], &dr)) {
            return;
        }
    }

    /* check if dr contains any rect in pool
     * if so, remove them 
     */
    for (i = mgr->pool_num - 1; i >= 0; i --) {
        if (dirty_rect_contains(&dr, &mgr->pool[i])) {
            arr_remove(mgr->pool, &mgr->pool_num, i);
        }
    }

    /* if have slot, add dr into pool directly */
    if (mgr->pool_num < IPGUI_DIRTY_RECT_POOL) {
        mgr->pool[mgr->pool_num ++] = dr;
    } else {
        arr_merge_best_pair(mgr->pool, &mgr->pool_num, &dr);
    }
}
/* APIs */

/* init dirty rect manager */
__IPGUI_API__ void ipgui_dirty_rect_mgr_init(ipgui_dirty_rect_mgr_t * mgr)
{
    mgr->pool_num = 0;
}

/* reset dirty rect manager（重绘后调用） */
__IPGUI_API__ void ipgui_dirty_rect_mgr_reset(ipgui_dirty_rect_mgr_t * mgr)
{
    ipgui_dirty_rect_mgr_init(mgr);
}

/* add a new dirty rect to pool */
__IPGUI_API__ void ipgui_dirty_rect_add(ipgui_dirty_rect_mgr_t * mgr,
                          ipgui_dirty_rect_t * dr)
{
    ipgui_dirty_rect_t nr = * dr;
    dirty_rect_normalize(&nr);
    pool_add(mgr, nr);
}

/* add a new dirty rect to pool */
__IPGUI_API__ void ipgui_dirty_rect_add_xywh(ipgui_dirty_rect_mgr_t * mgr,
                                ipgui_coord_t x, ipgui_coord_t y,
                                ipgui_coord_t w, ipgui_coord_t h)
{
    if (w <= 0 || h <= 0) return;
    ipgui_dirty_rect_t r = { x, y,
                              (ipgui_coord_t)(x + w - 1),
                              (ipgui_coord_t)(y + h - 1) };
    ipgui_dirty_rect_add(mgr, &r);
}

/* 阶段2：flush — 全局最优合并，生成用于重绘的脏矩形（重绘前调用）
 */
__IPGUI_API__ void ipgui_dirty_rect_flush(ipgui_dirty_rect_mgr_t * mgr)
{
    if (mgr->pool_num == 0) {
        return;
    }
    ipgui_dirty_rect_t * pool = mgr->pool;

    /* we have found a better merge */
    int improved = 1;
    int best_i, best_j;
    int best_cost;
    while (improved && mgr->pool_num > 1) {
        improved = 0;

        best_i = best_j = -1;
        best_cost = (int)IPGUI_MERGE_COST_THRESHOLD;

        for (int i = 0; i < mgr->pool_num; i ++) {
            for (int j = i + 1; j < mgr->pool_num; j ++) {
                int cost = dirty_rect_merge_cost(&pool[i], &pool[j]);
                if (cost <= (int)IPGUI_MERGE_COST_THRESHOLD &&
                    cost <= best_cost) {
                    best_cost = cost;
                    best_i = i;
                    best_j = j;
                }
            }
        }

        /* we can merge pool[best_i] and pool[best_j] */
        if (best_i >= 0) {
            pool[best_i] = dirty_rect_merge(&pool[best_i], &pool[best_j]);
            arr_remove(pool, &mgr->pool_num, best_j);
            improved = 1;
        }
    }
}

__IPGUI_API__ ipgui_dirty_rect_t * ipgui_dirty_rect_get(
    ipgui_dirty_rect_mgr_t * mgr, int index)
{
    if (index < 0 || index >= mgr->pool_num) return (ipgui_dirty_rect_t *)0;
    return &mgr->pool[index];
}

__IPGUI_API__ int ipgui_dirty_rect_is_dirty(ipgui_dirty_rect_mgr_t * mgr,
                            ipgui_dirty_rect_t * dr)
{
    for (int i = 0; i < mgr->pool_num; i++) {
        if (!(mgr->pool[i].x2 < dr->x1 ||
              dr->x2 < mgr->pool[i].x1 ||
              mgr->pool[i].y2 < dr->y1 ||
              dr->y2 < mgr->pool[i].y1)) {
            return 0;
        }
    }
    return 1;
}
