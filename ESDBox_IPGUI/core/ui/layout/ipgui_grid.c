#include "ipgui_grid.h"

#include "ipgui_widget.h"
#include "ipgui_memory.h"

#define GRID_COLS   IPGUI_GRID_MAX_COLS
#define GRID_ROWS   IPGUI_GRID_MAX_ROWS
#define GRID_AXIS_MAX   (GRID_COLS > GRID_ROWS ? GRID_COLS : GRID_ROWS)
#define GRID_AUTO   IPGUI_GRID_AUTO

/* 单轴轨道运行状态 */
typedef struct {
    u8_t            type[GRID_AXIS_MAX];    /* ipgui_grid_track_type_t */
    ipgui_coord_t   size[GRID_AXIS_MAX];    /* FIXED: 像素; AUTO: 内容尺寸; FR: 分配结果 */
    ipgui_coord_t   fr[GRID_AXIS_MAX];      /* FR 份额 */
    ipgui_coord_t   pos[GRID_AXIS_MAX];     /* 轨道起点（相对内容区） */
    u8_t            def_count;              /* 显式定义的轨道数量 */
} grid_axis_t;

/* 初始化轨道：显式定义之外的轨道全部按 auto 处理 */
static void grid_axis_init(
    grid_axis_t *               axis,
    const ipgui_grid_track_t *  defs,
    u8_t                        def_count)
{
    int i;

    ipgui_memset(axis, 0, sizeof(*axis));

    axis->def_count = (def_count > GRID_AXIS_MAX) ? GRID_AXIS_MAX : def_count;

    for (i = 0; i < GRID_AXIS_MAX; i++) {
        if (defs && i < axis->def_count) {
            axis->type[i] = defs[i].type;
            if (defs[i].type == IPGUI_GRID_TRACK_FIXED) {
                axis->size[i] = (defs[i].value > 0) ? defs[i].value : 0;
            } else if (defs[i].type == IPGUI_GRID_TRACK_FR) {
                axis->fr[i] = (defs[i].value > 0) ? defs[i].value : 1;
            } else {
                axis->type[i] = IPGUI_GRID_TRACK_AUTO;
            }
        } else {
            axis->type[i] = IPGUI_GRID_TRACK_AUTO;
        }
    }
}

/*
 * 解析最终轨道尺寸与起点
 * fixed / auto 尺寸已就绪，FR 轨道按剩余空间等比例分配，余数给最后一条 FR。
 */
static void grid_axis_resolve(
    grid_axis_t *   axis,
    int             used,
    ipgui_coord_t   avail,
    ipgui_coord_t   gap)
{
    ipgui_coord_t used_px = 0;
    ipgui_coord_t fr_total = 0;
    ipgui_coord_t free_px;
    ipgui_coord_t remain;
    ipgui_coord_t pos = 0;
    int last_fr = -1;
    int i;

    if (used <= 0) return;
    if (used > GRID_AXIS_MAX) used = GRID_AXIS_MAX;

    for (i = 0; i < used; i++) {
        if (axis->type[i] == IPGUI_GRID_TRACK_FR)
            fr_total += axis->fr[i];
        else
            used_px += axis->size[i];
    }

    free_px = avail - used_px;
    if (used > 1) free_px -= (ipgui_coord_t)(used - 1) * gap;
    if (free_px < 0) free_px = 0;

    remain = free_px;
    for (i = 0; i < used; i++) {
        if (axis->type[i] != IPGUI_GRID_TRACK_FR) continue;

        axis->size[i] = (ipgui_coord_t)((s64_t)free_px * axis->fr[i] / fr_total);
        remain -= axis->size[i];
        last_fr = i;
    }
    if (last_fr >= 0) axis->size[last_fr] += remain;

    for (i = 0; i < used; i++) {
        axis->pos[i] = pos;
        pos += axis->size[i] + gap;
    }
}

/* 占位矩阵：occ[col] 的第 row 位为 1 表示该格已被占用 */
static int grid_area_free(const u32_t * occ, int col, int row, int cs, int rs)
{
    int c, r;

    for (c = col; c < col + cs; c++) {
        for (r = row; r < row + rs; r++) {
            if (occ[c] & ((u32_t)1u << r)) return 0;
        }
    }
    return 1;
}

static void grid_area_occupy(u32_t * occ, int col, int row, int cs, int rs)
{
    int c, r;

    for (c = col; c < col + cs; c++) {
        for (r = row; r < row + rs; r++) {
            occ[c] |= ((u32_t)1u << r);
        }
    }
}

/* 把跨格内容尺寸摊到其覆盖的 auto 轨道上 */
static void grid_fit_span(
    grid_axis_t *   axis,
    int             start,
    int             span,
    ipgui_coord_t   need,
    ipgui_coord_t   gap)
{
    ipgui_coord_t req;
    ipgui_coord_t each;
    int has_auto = 0;
    int i;

    for (i = start; i < start + span; i++) {
        if (axis->type[i] == IPGUI_GRID_TRACK_AUTO) has_auto = 1;
    }
    if (!has_auto) return;

    req = need - (ipgui_coord_t)(span - 1) * gap;
    if (req < 0) req = 0;
    each = (req + span - 1) / span;             /* 向上取整 */

    for (i = start; i < start + span; i++) {
        if (axis->type[i] == IPGUI_GRID_TRACK_AUTO && axis->size[i] < each)
            axis->size[i] = each;
    }
}

/*
 * 排布一遍所有子控件
 *
 * write == 0：只做占位与 auto 轨道尺寸统计
 * write == 1：复用同一套放置算法，写入最终几何
 * 返回实际占用的列数 / 行数。
 */
static void grid_pass(
    ipgui_widget_t *            container,
    const ipgui_grid_cfg_t *    cfg,
    grid_axis_t *               ax,
    grid_axis_t *               ay,
    int                         ncols,
    int                         write,
    int *                       used_cols,
    int *                       used_rows)
{
    u32_t occ[GRID_COLS];
    int used_c = (cfg->col_count > 0) ? cfg->col_count : 1;
    int used_r = cfg->row_count;
    int cursor_col = 0;
    int cursor_row = 0;
    int idx = 0;
    int i;
    struct widget_link_t * link = container->link.first_child;

    for (i = 0; i < GRID_COLS; i++) occ[i] = 0;
    if (used_c > GRID_COLS) used_c = GRID_COLS;
    if (used_r > GRID_ROWS) used_r = GRID_ROWS;

    while (link) {
        ipgui_widget_t * child = ipgui_container_of(link, ipgui_widget_t, link);
        const ipgui_grid_item_t * item =
            (cfg->items && idx < cfg->item_count) ? &cfg->items[idx] : (const ipgui_grid_item_t *)0;

        int cs = 1, rs = 1;
        int ex_c = -1, ex_r = -1;
        int c0 = -1, r0 = -1;

        if (item) {
            cs = item->col_span ? item->col_span : 1;
            rs = item->row_span ? item->row_span : 1;
            if (item->col != GRID_AUTO) ex_c = item->col;
            if (item->row != GRID_AUTO) ex_r = item->row;
        }
        if (cs > GRID_COLS) cs = GRID_COLS;
        if (rs > GRID_ROWS) rs = GRID_ROWS;
        if (cs > ncols) cs = ncols;            /* ncols 已包含最大跨列 */

        if (ex_c >= 0 && ex_r >= 0) {
            /* 行列都显式指定：直接落位，允许与其它控件重叠 */
            c0 = (ex_c > GRID_COLS - cs) ? GRID_COLS - cs : ex_c;
            r0 = (ex_r > GRID_ROWS - rs) ? GRID_ROWS - rs : ex_r;
            if (c0 < 0 || r0 < 0) { c0 = -1; r0 = -1; }
        } else if (ex_c >= 0) {
            /* 只指定列：在该列内自上而下找空位 */
            c0 = (ex_c > GRID_COLS - cs) ? GRID_COLS - cs : ex_c;
            if (c0 < 0) c0 = -1;
            else {
                for (r0 = 0; r0 + rs <= GRID_ROWS; r0++) {
                    if (grid_area_free(occ, c0, r0, cs, rs)) break;
                }
                if (r0 + rs > GRID_ROWS) c0 = -1;
            }
        } else if (ex_r >= 0) {
            /* 只指定行：在该行内自左向右找空位 */
            r0 = (ex_r > GRID_ROWS - rs) ? GRID_ROWS - rs : ex_r;
            if (r0 < 0) r0 = -1;
            else {
                for (c0 = 0; c0 + cs <= GRID_COLS; c0++) {
                    if (grid_area_free(occ, c0, r0, cs, rs)) break;
                }
                if (c0 + cs > GRID_COLS) c0 = -1;
            }
        } else {
            /* 自动流：从游标起按行优先扫描第一个放得下的位置 */
            int c = cursor_col;
            int r = cursor_row;

            while (r + rs <= GRID_ROWS) {
                if (c + cs > ncols) { c = 0; r++; continue; }
                if (grid_area_free(occ, c, r, cs, rs)) { c0 = c; r0 = r; break; }
                c++;
                if (c >= ncols) { c = 0; r++; }
            }
            if (c0 >= 0) { cursor_col = c0; cursor_row = r0; }
        }

        if (c0 >= 0 && r0 >= 0) {
            grid_area_occupy(occ, c0, r0, cs, rs);
            if (c0 + cs > used_c) used_c = c0 + cs;
            if (r0 + rs > used_r) used_r = r0 + rs;

            if (!write) {
                grid_fit_span(ax, c0, cs, child->w, cfg->col_gap);
                grid_fit_span(ay, r0, rs, child->h, cfg->row_gap);
            } else {
                ipgui_coord_t x = cfg->pad_left + ax->pos[c0];
                ipgui_coord_t y = cfg->pad_top  + ay->pos[r0];
                ipgui_coord_t w = ax->pos[c0 + cs - 1] + ax->size[c0 + cs - 1] - ax->pos[c0];
                ipgui_coord_t h = ay->pos[r0 + rs - 1] + ay->size[r0 + rs - 1] - ay->pos[r0];

                if (x != child->x || y != child->y || w != child->w || h != child->h) {
                    /* 旧位置先标脏，改写后新位置再标脏 */
                    ipgui_widget_mark_dirty(child);
                    child->x = x;
                    child->y = y;
                    child->w = w;
                    child->h = h;
                    ipgui_widget_mark_dirty(child);
                }
            }
        }
        /* 放不下（越界）的子控件保持原样，不参与本次排布 */

        idx++;
        link = link->sib_next;
        if (link == container->link.first_child) break;
    }

    *used_cols = used_c;
    *used_rows = used_r;
}

__IPGUI_API__ ipgui_err_t ipgui_grid_apply(
    ipgui_widget_t *        container,
    const ipgui_grid_cfg_t * cfg)
{
    grid_axis_t ax, ay;
    ipgui_coord_t avail_w, avail_h;
    int ncols, max_span, used_c, used_r;
    int i;

    if (!container || !cfg || !cfg->cols || cfg->col_count == 0)
        return IPGUI_ERR_PARAM;

    if (IPGUI_YES == ipgui_widget_link_is_detached(&container->link))
        return IPGUI_ERR_OBJ_INVALID;

    if (!container->link.first_child)
        return IPGUI_ERR_OK;

    /* 自动流的可用列数：不小于显式列数，也需容纳最大跨列 */
    ncols = (cfg->col_count > 0) ? cfg->col_count : 1;
    max_span = 1;
    for (i = 0; cfg->items && i < cfg->item_count; i++) {
        int s = cfg->items[i].col_span ? cfg->items[i].col_span : 1;
        if (s > max_span) max_span = s;
    }
    if (max_span > ncols) ncols = max_span;
    if (ncols > GRID_COLS) ncols = GRID_COLS;

    grid_axis_init(&ax, cfg->cols, cfg->col_count);
    grid_axis_init(&ay, cfg->rows, cfg->row_count);

    /* 第一遍：占位 + auto 轨道尺寸统计 */
    grid_pass(container, cfg, &ax, &ay, ncols, 0, &used_c, &used_r);

    /* 解析最终轨道尺寸 */
    avail_w = container->w - cfg->pad_left - cfg->pad_right;
    avail_h = container->h - cfg->pad_top  - cfg->pad_bottom;
    if (avail_w < 0) avail_w = 0;
    if (avail_h < 0) avail_h = 0;

    grid_axis_resolve(&ax, used_c, avail_w, cfg->col_gap);
    grid_axis_resolve(&ay, used_r, avail_h, cfg->row_gap);

    /* 第二遍：同一套放置算法，写入几何 */
    grid_pass(container, cfg, &ax, &ay, ncols, 1, &used_c, &used_r);

    return IPGUI_ERR_OK;
}
