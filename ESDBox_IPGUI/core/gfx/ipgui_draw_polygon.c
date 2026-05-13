/* reconsitution from ipgui_polygon_raster.c
 * and let it support surf-based rendering
 */
#include "ipgui_draw_polygon.h"
#include "ipgui_debug.h"
#include "ipgui_memory.h"

#ifndef IPGUI_ABS
#define IPGUI_ABS(x) ((x) < 0 ? -(x) : (x))
#endif

__IPGUI_STATIC__ void ipgui_polygon_line_ras(
                ipgui_polygon_ras_t * ras,
                ipgui_coord_t x_left, ipgui_coord_t x_right,
                ipgui_coord_t y, ipgui_surf_t * surf,
                ipgui_polygon_style_t * attr);

#define EDGE_EMBED_NUM 10
#define EDGE_MAX_SLOPE 5
/* 每条边在单条扫描线上最多允许贡献 EDGE_MAX_SLOPE 个 cell，
 * 整条扫描线最多按 EDGE_EMBED_NUM 条边估算
 */
#define CELL_EMBED_NUM (EDGE_EMBED_NUM * EDGE_MAX_SLOPE)

ipgui_polygon_ras_t g_ras;

/* 向下（小y）对齐转化为整数坐标 */
#define IPGUI_SCANY_MIN(y) ((y) >> IPGUI_PIXEL_BITS)

/* 向上（大y）对齐转化为整数坐标再减1 */
#define IPGUI_SCANY_MAX(y) ((((y) + ((1 << IPGUI_PIXEL_BITS) - 1)) >> IPGUI_PIXEL_BITS) - 1)

__IPGUI_STATIC__ ipgui_err_t
ipgui_polygon_raster_request_mem(ipgui_polygon_ras_t * ras)
{
    /* request pool for cells and edges */
    void * edge, * cell;
    edge = ipgui_mem_alloc_def(
        EDGE_EMBED_NUM * sizeof(ipgui_edge_t));
    if (!edge) goto _err;

    cell = ipgui_mem_alloc_def(
        CELL_EMBED_NUM * sizeof(ipgui_cell_t));
    if (!cell) goto _err;

    ipgui_membox_init(&ras->edge_membox, edge,
        sizeof(ipgui_edge_t), EDGE_EMBED_NUM);
    ipgui_membox_init(&ras->cell_membox, cell,
        sizeof(ipgui_cell_t), CELL_EMBED_NUM);

    return IPGUI_ERR_OK;
_err:
    if (edge)
        ipgui_mem_free_def(edge);
    if (cell)
        ipgui_mem_free_def(cell);

    return IPGUI_ERR_NOMEM;
}

__IPGUI_STATIC__ s8_t y_start_cmp(void * a, void * b)
{
    ipgui_edge_t * ea = (ipgui_edge_t *)a;
    ipgui_edge_t * eb = (ipgui_edge_t *)b;

    if (eb->y_start < ea->y_start) return -1;
    else if (eb->y_start > ea->y_start) return 1;

    /* 为了让 AVL 对 y_start 相同的 edge 仍有稳定顺序 */
    if (eb->y_end < ea->y_end) return -1;
    else if (eb->y_end > ea->y_end) return 1;

    if (eb->x_cur.inte < ea->x_cur.inte) return -1;
    else if (eb->x_cur.inte > ea->x_cur.inte) return 1;

    if (eb->x_cur.frac < ea->x_cur.frac) return -1;
    else if (eb->x_cur.frac > ea->x_cur.frac) return 1;

    /* 最后退化到地址，保证严格弱序 */
    {
        uintptr_t va = (uintptr_t)ea;
        uintptr_t vb = (uintptr_t)eb;
        if (vb < va) return -1;
        else if (vb > va) return 1;
    }

    return 0;
}

__IPGUI_STATIC__ s8_t x_cmp(void * a, void * b)
{
    ipgui_cell_t * ea = (ipgui_cell_t *)a;
    ipgui_cell_t * eb = (ipgui_cell_t *)b;

    if (eb->x < ea->x) return -1;
    else if (eb->x > ea->x) return 1;
    else return 0;
}

__IPGUI_STATIC__ s32_t ipgui_edge_scanline_cell_need(ipgui_edge_t * edge)
{
    ipgui_scoord_t adx;

    if (edge->dy <= 0)
        return 0;

    adx = (edge->dx >= 0) ? edge->dx : -edge->dx;

    /* 单扫描线最坏情况下会跨越的像素列数：
     * cells <= ceil(|dx| / dy) + 1
     */
    return (s32_t)((adx + edge->dy - 1) / edge->dy) + 1;
}

__IPGUI_STATIC__ void ipgui_polygon_ras_runtime_clear(
                ipgui_polygon_ras_t * ras)
{
    avl_node_t * node;
    ipgui_edge_t * edge;
    ipgui_cell_t * cell;

    /* clear active edges */
    while (ras->active) {
        edge = ras->active;
        ras->active = edge->next;
        if (ras->active)
            ras->active->prev = (ipgui_edge_t *)0;
        ipgui_membox_free(&ras->edge_membox, (void *)edge);
    }

    /* clear edge buckets */
    while ((node = avl_find_first_node(&ras->edge_buckets)) != (avl_node_t *)0)
    {
        edge = (ipgui_edge_t *)_AVL_NODE2CONTAINER(node, 0);
        avl_node_delete(&edge->hook, &ras->edge_buckets);
        ipgui_membox_free(&ras->edge_membox, (void *)edge);
    }

    /* clear scanline cells */
    while ((node = avl_find_first_node(&ras->scanline_cells)) != (avl_node_t *)0)
    {
        cell = (ipgui_cell_t *)_AVL_NODE2CONTAINER(node, 0);
        avl_node_delete(&cell->hook, &ras->scanline_cells);
        ipgui_membox_free(&ras->cell_membox, (void *)cell);
    }
}

__IPGUI_API__ ipgui_err_t ipgui_polygon_ras_init(
                ipgui_polygon_ras_t * ras)
{
    ipgui_err_t err;
    err = ipgui_polygon_raster_request_mem(ras);
    if (IPGUI_ERR_OK != err)
        return err;
    avl_init(&ras->edge_buckets, y_start_cmp, 0);
    avl_init(&ras->scanline_cells, x_cmp, 0);
    ras->active = (ipgui_edge_t *)0;
    ras->y_max = IPGUI_COORD_MIN;
    ras->y_min = IPGUI_COORD_MAX;

    ras->fill_rule = IPGUI_FILL_RULE_NONZERO;/* default fill rule */

    ras->err = 0; /* no error */

    return IPGUI_ERR_OK;
}

__IPGUI_API__ void ipgui_polygon_ras_set_fill_rule(
                ipgui_polygon_ras_t * ras,
                ipgui_fill_rule_t rule)
{
    ras->fill_rule = rule;
}

__IPGUI_STATIC__ void ipgui_x_step(ipgui_edge_t * edge,
                ipgui_egde_xstep_t * step)
{
    if (!edge->dx) return; /* vertical line */

    edge->x_cur.inte += step->inte;
    edge->x_cur.frac += step->frac;
    while (IPGUI_ABS(edge->x_cur.frac) >= edge->dy)
    {
        if (edge->dx > 0) {
            edge->x_cur.inte += IPGUI_PIXEL_PRECI;
            edge->x_cur.frac -= edge->dy;
        } else {
            edge->x_cur.inte -= IPGUI_PIXEL_PRECI;
            edge->x_cur.frac += edge->dy;
        }
    }
}

__IPGUI_STATIC__ void ipgui_x_full_step(ipgui_edge_t * edge)
{
    ipgui_x_step(edge, &edge->x_full_step);
}

__IPGUI_STATIC__ s32_t ipgui_convert_edge_init(
                    ipgui_edge_t * edge,
                    ipgui_scoord_t y_start,
                    ipgui_scoord_t y_end,
                    ipgui_spoint_t p1,
                    ipgui_spoint_t p2,
                    ipgui_edge_dir_t dir)
{
    ipgui_spoint_t scan_from, scan_to;    /* from to表示扫描方向 */

    if (p1.y > p2.y) {
        scan_from = p2;
        scan_to = p1;
    } else if (p1.y < p2.y) {
        scan_from = p1;
        scan_to = p2;
    } else {
        return 1;                /* 水平线 */
    }

    edge->dx = scan_to.x - scan_from.x;
    edge->dy = scan_to.y - scan_from.y;

    if (scan_from.x != scan_to.x) {       /* 斜线 */
        edge->y_start = scan_from.y;
        edge->y_end = scan_to.y;
        edge->x_cur.inte = (scan_from.x >> IPGUI_PIXEL_BITS) << IPGUI_PIXEL_BITS;
        edge->x_cur.frac = (scan_from.x - edge->x_cur.inte) * edge->dy / IPGUI_PIXEL_PRECI;
        edge->x_full_step.inte = (edge->dx) / (edge->dy);
        edge->x_full_step.inte <<= IPGUI_PIXEL_BITS;
        edge->x_full_step.frac = (edge->dx) % (edge->dy); /* frac/dy 是一个像素的小数部分 */

        /* 由 y_start 和 y_end 重置 edge 的 y_start 和 y_end，
         * 先由扫描线的 y 跨度得出包围盒（连续方块模型）再裁剪 edge
         */
        if (edge->y_start < y_start) {
            ipgui_scoord_t dy = y_start - edge->y_start; /* dy >= 0 */
            ipgui_egde_xstep_t x_off;
            s32_t temp;
            /* 再修正起始 x 坐标，edge->x_cur at edge->y_start */
            edge->y_start = y_start;

            /* 修正整数部分 */
            temp = dy / IPGUI_PIXEL_PRECI;/* xstep 几个完整的像素 */
            x_off.inte = edge->x_full_step.inte * temp;
            x_off.frac = edge->x_full_step.frac * temp;
            x_off.inte += ((x_off.frac / edge->dy) * IPGUI_PIXEL_PRECI);
            x_off.frac = x_off.frac % edge->dy;

            /* 修正小数部分 */
            dy = dy - temp * IPGUI_PIXEL_PRECI;
            temp = (dy * edge->dx) / edge->dy; /* 不足一像素偏移的 x 子像素 */
            temp = (temp * edge->dy) / IPGUI_PIXEL_PRECI;/* 放大至与 frac 相同，有精度损失 */
            x_off.frac += temp;
            ipgui_x_step(edge, &x_off);
        }
        if (edge->y_end >= y_end)
            edge->y_end = y_end;
        edge->y_min = IPGUI_SCANY_MIN(edge->y_start);
        edge->y_max = IPGUI_SCANY_MAX(edge->y_end);

    } else {                    /* 垂线 */
        edge->y_start = scan_from.y;
        edge->y_end = scan_to.y;
        /* 由 y_start 和 y_end 重置 edge 的 y_start 和 y_end，
         * 先由扫描线的 y 跨度得出包围盒（连续方块模型）再裁剪 edge
         */
        if (edge->y_start <= y_start)
            edge->y_start = y_start;
        if (edge->y_end >= y_end)
            edge->y_end = y_end;
        edge->y_min = IPGUI_SCANY_MIN(edge->y_start);
        edge->y_max = IPGUI_SCANY_MAX(edge->y_end);
        edge->x_cur.inte = scan_from.x;
        edge->x_cur.frac = 0;
        edge->x_full_step.inte = 0;
        edge->x_full_step.frac = 0;
    }
    edge->dir = dir;

    return 0;
}

__IPGUI_STATIC__ ipgui_err_t ipgui_polygon_ras_add_edge(
                ipgui_polygon_ras_t * ras,
                ipgui_scoord_t y_start, ipgui_scoord_t y_end,
                ipgui_spoint_t p1, ipgui_spoint_t p2,
                ipgui_edge_dir_t dir,
                ipgui_edge_t ** ret)
{
    ipgui_edge_t * edge;

    /* allocate memory for edge */
    edge = (ipgui_edge_t *)ipgui_membox_alloc(&ras->edge_membox);
    if ((ipgui_edge_t *)0 == edge) {
        /* we can expand the memory box and retry */
        void * expand = (void *)0;
        expand = ipgui_mem_alloc_def(
            EDGE_EMBED_NUM * sizeof(ipgui_edge_t));
        if (!expand) {
            ipgui_dbg_error("no memory error\r\n");
            goto _no_mem_err;
        }
        ipgui_membox_expand(&ras->edge_membox, expand,
            sizeof(ipgui_edge_t), EDGE_EMBED_NUM);

        edge = (ipgui_edge_t *)ipgui_membox_alloc(&ras->edge_membox);
        if ((ipgui_edge_t *)0 == edge) {
            ipgui_dbg_error("no memory error\r\n");
            goto _no_mem_err;
        }
    }

    if (1 == ipgui_convert_edge_init(edge, y_start, y_end, p1, p2, dir)) {
        /* it is a horizontal line */
        ipgui_membox_free(&ras->edge_membox, edge);
        if (ret) *ret = (ipgui_edge_t *)0;
        return IPGUI_ERR_OK;
    }

    /* 单条边在单扫描线上最多允许 EDGE_MAX_SLOPE 个 cell */
    if (ipgui_edge_scanline_cell_need(edge) > EDGE_MAX_SLOPE) {
        ipgui_dbg_error("polygon edge too flat for current CELL_EMBED_NUM\r\n");
        ipgui_membox_free(&ras->edge_membox, edge);
        if (ret) *ret = (ipgui_edge_t *)0;
        return IPGUI_ERR_PARAM;
    }

    edge->next = (ipgui_edge_t *)0;
    edge->prev = (ipgui_edge_t *)0;

    g_avl_node_add((void *)edge, &ras->edge_buckets);

    if (ret) *ret = edge;
    return IPGUI_ERR_OK;

_no_mem_err:
    ras->err = 1; /* set error flag */
    if (ret) *ret = (ipgui_edge_t *)0;
    return IPGUI_ERR_NOMEM;
}

__IPGUI_STATIC__ ipgui_err_t record_cell(
    ipgui_polygon_ras_t * ras,
    ipgui_point_t p,
    ipgui_scoord_t wind_height, ipgui_area_t area)
{
    ipgui_coord_t x = p.x;
    avl_node_t * node;
    ipgui_cell_t same;
    ipgui_cell_t * cell;

    same.x = x;
    same.wind_height = wind_height;
    same.area = area;

    /* 找到 x（非子像素坐标）相同的 cell */
    node = g_avl_node_search((void *)&same,
                &ras->scanline_cells);
    if (node) {
        /* found */
        cell = _AVL_NODE2CONTAINER(node, 0);
        cell->wind_height += wind_height;
        cell->area += area;
        return IPGUI_ERR_OK;
    }

    cell = ipgui_membox_alloc(&ras->cell_membox);
    if ((ipgui_cell_t *)0 == cell) {
        /* no way! */
        ras->err = 1; /* set error flag */
        ipgui_dbg_error("no memory error\r\n");
        return IPGUI_ERR_NOMEM;
    }
    * cell = same;
    g_avl_node_add((void *)cell,
            &ras->scanline_cells);

    return IPGUI_ERR_OK;
}

__IPGUI_STATIC__ void render_line_impl(
    ipgui_polygon_ras_t * ras,
    ipgui_coord_t y,
    ipgui_spoint_t p1, ipgui_spoint_t p2,
    ipgui_edge_dir_t dir)
{
    ipgui_scoord_t fx1, fx2;
    ipgui_coord_t ix1, ix2;
    ipgui_scoord_t dx, dy;
    s32_t step = 0;
    ipgui_scoord_t x_iter;
    ipgui_point_t p;

    ipgui_scoord_t h, acc = 0;
    s32_t fh;
    ipgui_area_t left;

    if (p1.y == p2.y) return;

    ix1 = p1.x >> IPGUI_PIXEL_BITS;
    ix2 = p2.x >> IPGUI_PIXEL_BITS;
    fx1 = p1.x - (ix1 << IPGUI_PIXEL_BITS);
    fx2 = p2.x - (ix2 << IPGUI_PIXEL_BITS);

    dx = p2.x - p1.x;
    dy = p2.y - p1.y;

    if (ix1 == ix2) {
        /* x 跨度只覆盖了一个像素 */
        h = dy;
        left = (fx2 + fx1) * h;

        if (IPGUI_EDGE_DIR_DOWN == dir) {
            left = -left;
            h = -h;
        }
        p.x = ix1; p.y = y;
        record_cell(ras, p, h, left);
        return;
    }

    if      (dx > 0) step = 1;
    else if (dx < 0) { step = -1; dx = -dx; }
    else return;

    x_iter = IPGUI_ABS(ix1 - ix2) + 1; /* x_iter 个像素被影响了 */
    while (x_iter --)
    {
        if (step == 1) { /* dx > 0 */
            if (ix1 != ix2) {
                h = dy * (IPGUI_PIXEL_PRECI - fx1);
                fh = (h % dx) / dy;
                h /= dx;
                left = (fx1 + IPGUI_PIXEL_PRECI) * h;

                acc += h;
                if (fh > dy) {
                    acc += 1;
                    fh -= dy;
                }
            } else {
                h = dy - acc;
                left = (fx2 + fx1) * h;
            }

            if (IPGUI_EDGE_DIR_DOWN == dir) {
                left = -left;
                h = -h;
            }

            /* 记录下 cell */
            p.x = ix1; p.y = y;
            record_cell(ras, p, h, left);

            fx1 = 0; ix1 += step;
        } else {  /* dx < 0 */
            if (ix1 != ix2) {
                h = dy * fx1;
                fh = (h % dx) / dy;
                h /= dx;
                left = fx1 * h;

                acc += h;
                if (fh > dy) {
                    acc += 1;
                    fh -= dy;
                }
            } else {
                h = dy - acc;
                left = (fx2 + fx1) * h;
            }

            if (IPGUI_EDGE_DIR_DOWN == dir) {
                left = -left;
                h = -h;
            }

            /* 记录下 cell */
            p.x = ix1; p.y = y;
            record_cell(ras, p, h, left);

            fx1 = IPGUI_PIXEL_PRECI; ix1 += step;
        }
    }
}

__IPGUI_API__ void ipgui_render_edge_line(
                ipgui_polygon_ras_t * ras,
                ipgui_coord_t y,
                ipgui_edge_t * edge)
{
    ipgui_scoord_t x1, x2;
    ipgui_scoord_t y_scan_next;
    ipgui_scoord_t dy;
    ipgui_spoint_t p1;
    ipgui_spoint_t p2;

    y_scan_next = (y + 1) << IPGUI_PIXEL_BITS;
    if (y_scan_next > edge->y_end)
        y_scan_next = edge->y_end;
    if ((y << IPGUI_PIXEL_BITS) < edge->y_start)
        dy = y_scan_next - edge->y_start;
    else
        dy = y_scan_next - (y << IPGUI_PIXEL_BITS);

    x1 = edge->x_cur.inte;
    x1 += edge->x_cur.frac * IPGUI_PIXEL_PRECI / edge->dy;

    if (dy != IPGUI_PIXEL_PRECI) { /* dy != 64 */
        /* partial step */
        ipgui_egde_xstep_t x_step;
        long long temp = (long long)dy * (long long)edge->dx;
        x_step.inte = (ipgui_scoord_t)(temp / edge->dy);
        x_step.frac = (x_step.inte % IPGUI_PIXEL_PRECI) * edge->dy / IPGUI_PIXEL_PRECI; /* 精度损失 */
        x_step.inte = (x_step.inte / IPGUI_PIXEL_PRECI) * IPGUI_PIXEL_PRECI;
        ipgui_x_step(edge, &x_step);
    } else {
        /* full step */
        ipgui_x_full_step(edge);
    }
    x2 = edge->x_cur.inte;
    x2 += edge->x_cur.frac * IPGUI_PIXEL_PRECI / edge->dy;

    /* now we get the scanline composed of two points
     * calc and record all cells effected by this edge
     */
    p1.x = x1;
    p1.y = y << IPGUI_PIXEL_BITS;
    if (p1.y < edge->y_start)
        p1.y = edge->y_start;
    p2.x = x2; p2.y = y_scan_next;
    render_line_impl(ras, y, p1, p2, edge->dir);
}

__IPGUI_STATIC__ void ipgui_active_edge_insert(
    ipgui_edge_t * edge,
    ipgui_polygon_ras_t * ras)
{
    edge->prev = (ipgui_edge_t *)0;
    edge->next = ras->active;
    if (ras->active) ras->active->prev = edge;
    ras->active = edge;
}

__IPGUI_STATIC__ void ipgui_active_edge_remove(
    ipgui_edge_t * edge,
    ipgui_polygon_ras_t * ras)
{
    if (edge->prev)
        edge->prev->next = edge->next;
    else
        ras->active = edge->next;
    if (edge->next)
        edge->next->prev = edge->prev;
}

__IPGUI_API__ ipgui_err_t ipgui_draw_spolygon(ipgui_surf_t * surf,
                ipgui_aabb_t * clip,
                ipgui_spoint_t * p, s32_t num,
                ipgui_polygon_ras_t * ras,
                ipgui_polygon_style_t * attr)
{
    ipgui_aabb_t buffer, self, draw;
    ipgui_spoint_t edge_p1, edge_p2;
    ipgui_err_t err;
    ipgui_edge_dir_t dir;
    ipgui_coord_t y; /* 扫描线 y 坐标 */

    if (ras->err) {
        return IPGUI_ERR_NOMEM;
    }

    if (num > EDGE_EMBED_NUM) /* 多少个点就需要多少个边，大于最大支持的边数就退出 */
        return IPGUI_ERR_PARAM;

    if (num < 3) return IPGUI_ERR_PARAM;

    /* 生成渲染区域包围盒 */
    buffer = surf->surf;
    buffer.end.x += 1; /* 转换为连续方块模型 */
    buffer.end.y += 1; /* 转换为连续方块模型 */

    buffer.start.x <<= IPGUI_PIXEL_BITS; /* 扩大子像素精度倍 */
    buffer.start.y <<= IPGUI_PIXEL_BITS;
    buffer.end.x <<= IPGUI_PIXEL_BITS;
    buffer.end.y <<= IPGUI_PIXEL_BITS;

    /* 生成多边形包围盒 */
    self.end.x = self.start.x = p[0].x;
    self.end.y = self.start.y = p[0].y;
    for (s32_t i = 1; i < num; i ++) {
        ipgui_aabb_update_with_point(&self, &p[i]);
    }

    /* 计算渲染区域和多边形的交集（连续方块模型） */
    if (0 != ipgui_aabb_intersect(&draw, &buffer, &self))
        return IPGUI_ERR_OK;

    if (clip) {
        if (0 != ipgui_aabb_intersect(&draw, &draw, clip))
            return IPGUI_ERR_OK;
    }

    /* 连续方块模型的边界也不能重合 */
    if (draw.end.x == draw.start.x || draw.end.y == draw.start.y)
        return IPGUI_ERR_OK;

    ras->y_min = IPGUI_SCANY_MIN(draw.start.y);/* 光栅化器的 y 跨度和包围盒的 y 跨度一样 */
    ras->y_max = IPGUI_SCANY_MAX(draw.end.y);

    /* 扫描线的 y 跨度（连续方块模型）扩大 IPGUI_PIXEL_PRECI 倍 */
    {
        ipgui_scoord_t y_start, y_end;
        y_start = ras->y_min << IPGUI_PIXEL_BITS;
        y_end = (ras->y_max + 1) << IPGUI_PIXEL_BITS;

        /* 将与包围盒有交集的边添加到边表中 */
        for (s32_t i = 0; i < num; i ++) {
            edge_p1.x = p[i].x;
            edge_p1.y = p[i].y;
            edge_p2.x = p[(i + 1) % num].x;
            edge_p2.y = p[(i + 1) % num].y;
            dir = (edge_p2.y > edge_p1.y) ? IPGUI_EDGE_DIR_DOWN : IPGUI_EDGE_DIR_UP;

            /* 看是否与包围盒的 y 跨度有交集 */
            if (dir == IPGUI_EDGE_DIR_DOWN) { /* 向下 p2.y > p1.y */
                if (edge_p2.y <= draw.start.y || edge_p1.y >= draw.end.y) continue;
            } else { /* 向上 p2.y < p1.y */
                if (edge_p2.y >= draw.end.y || edge_p1.y <= draw.start.y) continue;
            }

            err = ipgui_polygon_ras_add_edge(ras, y_start, y_end,
                                             edge_p1, edge_p2, dir,
                                             (ipgui_edge_t **)0);
            if (err != IPGUI_ERR_OK) {
                ipgui_polygon_ras_runtime_clear(ras);
                return err;
            }
        }
    }

    /* 还原 draw */
    draw.start.x >>= IPGUI_PIXEL_BITS;
    draw.start.y >>= IPGUI_PIXEL_BITS;
    draw.end.x >>= IPGUI_PIXEL_BITS;
    draw.end.y >>= IPGUI_PIXEL_BITS;

    {
        avl_node_t * node;
        ipgui_edge_t * edge;

        for (y = ras->y_min; y <= ras->y_max; y ++)
        {
            /* 更新活性边链表 */
            while (1)
            {
                node = avl_find_first_node(&ras->edge_buckets);
                if (node == (avl_node_t *)0)
                    break;
                edge = (ipgui_edge_t *)_AVL_NODE2CONTAINER(node, 0);
                if (y >= edge->y_min) {
                    /* 从新边 avl 树中删除，插入到活性边链表中 */
                    avl_node_delete(&edge->hook, &ras->edge_buckets);
                    ipgui_active_edge_insert(edge, ras);
                }
                else
                    break;
            }

            /* 渲染在活性边链表中的边 */
            {
                ipgui_edge_t * iter, * next;
                iter = ras->active;
                while (iter != (ipgui_edge_t *)0)
                {
                    next = iter->next;
                    /* for every edge, calculate covered pixel's left sign area */
                    ipgui_render_edge_line(ras, y, iter);

                    /* remove all edges that are finished */
                    if (y >= iter->y_max) {
                        ipgui_active_edge_remove(iter, ras);
                        ipgui_membox_free(&ras->edge_membox, (void *)iter);
                    }
                    iter = next;
                }
            }

            if (ras->err) {
                ipgui_polygon_ras_runtime_clear(ras);
                return IPGUI_ERR_NOMEM;
            }

            /* raster this line by cells */
            ipgui_polygon_line_ras(ras, draw.start.x, draw.end.x, y, surf, attr);

            /* free all cells */
            {
                ipgui_cell_t * cell;
                cell = (ipgui_cell_t *)0;
                while ((cell = (ipgui_cell_t *)avl_find_first_node(&ras->scanline_cells))
                    != (ipgui_cell_t *)0)
                {
                    avl_node_delete(&cell->hook, &ras->scanline_cells);
                    ipgui_membox_free(&ras->cell_membox, (void *)cell);
                    cell = (ipgui_cell_t *)0;
                }
            }
        }
    }

    while (ras->active) {
        ipgui_edge_t * edge = ras->active;
        ipgui_active_edge_remove(edge, ras);
        ipgui_membox_free(&ras->edge_membox, (void *)edge);
    }

    /* check if rasterizer has edge left */
    if (avl_find_first_node(&ras->edge_buckets))
    {
        ipgui_dbg_error("logic error: edge left!\r\n");
        ipgui_polygon_ras_runtime_clear(ras);
        return IPGUI_ERR_LOGIC;
    }
    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_draw_polygon(ipgui_surf_t * surf,
                ipgui_aabb_t * clip,
                ipgui_point_t * p, s32_t num,
                ipgui_polygon_ras_t * ras,
                ipgui_polygon_style_t * attr)
{
    ipgui_aabb_t buffer, self, draw;
    ipgui_spoint_t edge_p1, edge_p2;
    ipgui_err_t err;
    ipgui_edge_dir_t dir;
    ipgui_coord_t y; /* 扫描线 y 坐标 */

    if (ras->err) {
        return IPGUI_ERR_NOMEM;
    }

    if (num > EDGE_EMBED_NUM) /* 多少个点就需要多少个边，大于最大支持的边数就退出 */
        return IPGUI_ERR_PARAM;

    if (num < 3) return IPGUI_ERR_PARAM;

    /* 生成渲染区域包围盒 */
    buffer = surf->surf;
    buffer.end.x += 1; /* 转换为连续方块模型 */
    buffer.end.y += 1; /* 转换为连续方块模型 */

    /* 生成多边形包围盒 */
    self.end.x = self.start.x = p[0].x;
    self.end.y = self.start.y = p[0].y;
    for (s32_t i = 1; i < num; i ++) {
        ipgui_aabb_update_with_point(&self, &p[i]);
    }

    /* 计算渲染区域和多边形的交集（连续方块模型） */
    if (0 != ipgui_aabb_intersect(&draw, &buffer, &self))
        return IPGUI_ERR_OK;

    if (clip) {
        if (0 != ipgui_aabb_intersect(&draw, &draw, clip))
            return IPGUI_ERR_OK;
    }

    /* 连续方块模型的边界也不能重合 */
    if (draw.end.x == draw.start.x || draw.end.y == draw.start.y)
        return IPGUI_ERR_OK;

    ras->y_min = draw.start.y; /* 光栅化器的 y 跨度和包围盒的 y 跨度一样 */
    ras->y_max = draw.end.y - 1;

    {
        ipgui_scoord_t y_start, y_end;
        y_start = ras->y_min << IPGUI_PIXEL_BITS;
        y_end = (ras->y_max + 1) << IPGUI_PIXEL_BITS;

        /* 将与包围盒有交集的边添加到边表中 */
        for (s32_t i = 0; i < num; i ++) {
            edge_p1.x = p[i].x;
            edge_p1.y = p[i].y;
            edge_p2.x = p[(i + 1) % num].x;
            edge_p2.y = p[(i + 1) % num].y;
            dir = (edge_p2.y > edge_p1.y) ? IPGUI_EDGE_DIR_DOWN : IPGUI_EDGE_DIR_UP;

            /* 看是否与包围盒的 y 跨度有交集 */
            if (dir == IPGUI_EDGE_DIR_DOWN) { /* 向下 p2.y > p1.y */
                if (edge_p2.y <= draw.start.y || edge_p1.y >= draw.end.y) continue;
            } else { /* 向上 p2.y < p1.y */
                if (edge_p2.y >= draw.end.y || edge_p1.y <= draw.start.y) continue;
            }

            /* 转化为子像素坐标（连续方块模型） */
            edge_p1.x = edge_p1.x << IPGUI_PIXEL_BITS;
            edge_p1.y = edge_p1.y << IPGUI_PIXEL_BITS;
            edge_p2.x = edge_p2.x << IPGUI_PIXEL_BITS;
            edge_p2.y = edge_p2.y << IPGUI_PIXEL_BITS;

            err = ipgui_polygon_ras_add_edge(ras, y_start, y_end,
                                             edge_p1, edge_p2, dir,
                                             (ipgui_edge_t **)0);
            if (err != IPGUI_ERR_OK) {
                ipgui_polygon_ras_runtime_clear(ras);
                return err;
            }
        }
    }

    {
        avl_node_t * node;
        ipgui_edge_t * edge;

        for (y = ras->y_min; y <= ras->y_max; y ++)
        {
            /* 更新活性边链表 */
            while (1)
            {
                node = avl_find_first_node(&ras->edge_buckets);
                if (node == (avl_node_t *)0)
                    break;
                edge = (ipgui_edge_t *)_AVL_NODE2CONTAINER(node, 0);
                if (y >= edge->y_min) {
                    /* 从新边 avl 树中删除，插入到活性边链表中 */
                    avl_node_delete(&edge->hook, &ras->edge_buckets);
                    ipgui_active_edge_insert(edge, ras);
                }
                else
                    break;
            }

            /* 渲染在活性边链表中的边 */
            {
                ipgui_edge_t * iter, * next;
                iter = ras->active;
                while (iter != (ipgui_edge_t *)0)
                {
                    next = iter->next;
                    /* for every edge, calculate covered pixel's left sign area */
                    ipgui_render_edge_line(ras, y, iter);

                    /* remove all edges that are finished */
                    if (y >= iter->y_max) {
                        ipgui_active_edge_remove(iter, ras);
                        ipgui_membox_free(&ras->edge_membox, (void *)iter);
                    }
                    iter = next;
                }
            }

            if (ras->err) {
                ipgui_polygon_ras_runtime_clear(ras);
                return IPGUI_ERR_NOMEM;
            }

            /* raster this line by cells */
            ipgui_polygon_line_ras(ras, draw.start.x, draw.end.x, y, surf, attr);

            /* free all cells */
            {
                ipgui_cell_t * cell;
                cell = (ipgui_cell_t *)0;
                while ((cell = (ipgui_cell_t *)avl_find_first_node(&ras->scanline_cells))
                    != (ipgui_cell_t *)0)
                {
                    avl_node_delete(&cell->hook, &ras->scanline_cells);
                    ipgui_membox_free(&ras->cell_membox, (void *)cell);
                    cell = (ipgui_cell_t *)0;
                }
            }
        }
    }

    while (ras->active) {
        ipgui_edge_t * edge = ras->active;
        ipgui_active_edge_remove(edge, ras);
        ipgui_membox_free(&ras->edge_membox, (void *)edge);
    }

    /* 是否有剩余的边没被释放
     * 如果有的话，说明前面的“渲染在活性边链表中的边”逻辑有问题
     */
    if (avl_find_first_node(&ras->edge_buckets))
    {
        ipgui_dbg_error("logic error: edge left!\r\n");
        ipgui_polygon_ras_runtime_clear(ras);
        return IPGUI_ERR_LOGIC;
    }
    return IPGUI_ERR_OK;
}

__IPGUI_STATIC__ __IPGUI_INLINE__
u8_t coverage_scale_down(
    ipgui_area_t cover_area,
    ipgui_fill_rule_t fill_rule)
{
    cover_area >>= IPGUI_PIXEL_BITS * 2 + 1 - 8;
    if (cover_area < 0)
        cover_area = -cover_area - 1;

    /* compute the line's coverage depending on the outline fill rule */
    if (fill_rule != IPGUI_FILL_RULE_NONZERO)
    {
        cover_area &= 511;

        if (cover_area >= 256)
            cover_area = 511 - cover_area;
    }
    else
    {
        /* normal non-zero winding rule */
        if (cover_area >= 256)
            cover_area = 255;
    }
    return (u8_t)cover_area;
}

extern premult_blend_func_t premult_blend_table[PIX_FMT_MAX];
__IPGUI_STATIC__ void ipgui_polygon_line_ras(
                ipgui_polygon_ras_t * ras,
                ipgui_coord_t x_left, ipgui_coord_t x_right,
                ipgui_coord_t y, ipgui_surf_t * surf,
                ipgui_polygon_style_t * attr)
{
    ipgui_cell_t * iter, * first, * last;
    ipgui_cell_t * next;

    ipgui_coord_t ix, iy;

    ipgui_area_t wind_coverage = 0;
    ipgui_area_t area_coverage;
    u8_t cover;
    u8_t alpha;

    first = iter = (ipgui_cell_t *)
    avl_find_first_node(&ras->scanline_cells);

    last = (ipgui_cell_t *)avl_find_last_node(&ras->scanline_cells);

    if (!first || !last)
        return;
    if (first->x >= x_right || last->x < x_left)
        return;

    premult_blend_func_t blend_fn = premult_blend_table[surf->pix_fmt];
    if (!blend_fn) return;

    {
        ipgui_color_t premult;
        iy = y - surf->surf.start.y;
        while (iter) {
            next = (ipgui_cell_t *)avl_next_node(&iter->hook);

            wind_coverage += (iter->wind_height * IPGUI_PIXEL_PRECI << 1);
            area_coverage = wind_coverage - iter->area;
            cover = coverage_scale_down(area_coverage, ras->fill_rule);
            alpha = (cover * attr->alpha) >> 8;

            if (alpha >= 2 && (iter->x >= x_left) && (iter->x < x_right)) {
                ix = iter->x - surf->surf.start.x;
                {
                    u8_t * cr;
                    cr = ipgui_surf_color_get(surf, ix, iy);
                    premult = ipgui_color_combine_opacity_and_premultiply(
                        &attr->color, alpha);
                    blend_fn(premult, cr, attr->blend_mode);
                }
            }
            if (next && (next->x > (iter->x + 1)))
            {
                cover = coverage_scale_down(wind_coverage, ras->fill_rule);
                alpha = (cover * attr->alpha) >> 8;
                if (alpha >= 2) {
                    premult = ipgui_color_combine_opacity_and_premultiply(
                        &attr->color, alpha);
                    for (s32_t x = iter->x + 1; x < next->x; x ++) {
                        if ((x >= x_left) && (x < x_right)) {
                            ix = x - surf->surf.start.x;
                            {
                                u8_t * cr;
                                cr = ipgui_surf_color_get(surf, ix, iy);
                                blend_fn(premult, cr, attr->blend_mode);
                            }
                        }
                    }
                }
            }

            iter = next;
        }
    }
}