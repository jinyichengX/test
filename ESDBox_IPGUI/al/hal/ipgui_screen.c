#include "ipgui_screen.h"
#include "ipgui_rect_slice.h"
#include "ipgui_widget.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

__IPGUI_API__ ipgui_err_t ipgui_screen_init(ipgui_scr_t * scr, ipgui_scr_drv_t * drv)
{
    if (!scr)
        return IPGUI_ERR_NOK;
    ipgui_memset((void *)scr, 0, sizeof(ipgui_scr_t));
    scr->drv = drv;

    /* 初始化脏矩形管理器 */
    ipgui_dirty_rect_mgr_init(&scr->dirty);
    // ipgui_dirty_rect_add_xywh(&scr->dirty, 
    //     0, 
    //     0, 
    //     scr->drv->xreso, 
    //     scr->drv->yreso);

    /* 初始化控件树 */
    ipgui_widget_tree_init(&scr->tree);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_scr_create_pfb(
    ipgui_scr_t   * scr, 
    u8_t          * buf, 
    u32_t           buf_size, 
    ipgui_pix_fmt_t pix_fmt)
{
    if (!scr || !buf || !buf_size) {
        ipgui_dbg_error("param is invalid\n");
        return IPGUI_ERR_PARAM;
    }

    if (pix_fmt >= PIX_FMT_MAX) {
        ipgui_dbg_error("pix_fmt is invalid\n");
        return IPGUI_ERR_PARAM;
    }

    if (scr->drv->xreso <= 0 || scr->drv->yreso <= 0) {
        ipgui_dbg_error("screen resolution is invalid\n");
        return IPGUI_ERR_PARAM;
    }

    u8_t * top_buf = buf + buf_size;
    u32_t valid_size;
    u8_t * pfb_buf = IPGUI_ALIGN_U32(buf);
    
    if (pfb_buf >= top_buf) return IPGUI_ERR_NOK;

    valid_size = top_buf - pfb_buf;

    u8_t px_size = 0;
    switch ((u8_t)pix_fmt) {
        case PIX_FMT_RGB565:
        case PIX_FMT_BGR565:   px_size = 2; break;
        case PIX_FMT_RGB888:
        case PIX_FMT_BGR888:   px_size = 4; break;
        case PIX_FMT_RGBA8888:
        case PIX_FMT_BGRA8888: px_size = 4; break;
        default: break;
    }

    scr->pfb.num_pixs = valid_size / px_size;
    scr->pfb.color    = pfb_buf;
    scr->pfb.pix_fmt  = pix_fmt;
    scr->pfb.pix_size = px_size;

    /* clear buf */
    ipgui_memset(pfb_buf, 0, valid_size);

    return IPGUI_ERR_OK;
}

/* =========================================================================
 * 内部：DFS 遍历渲染单个控件及其子树
 *
 * 采用 Z-order（先序遍历 DFS）确保：
 *   - 父控件先于子控件绘制
 *   - 同级控件按插入顺序绘制
 *   - 子控件像素覆盖父控件像素（自然叠层）
 *
 * 裁剪策略（两级裁剪）：
 *   1. 控件全局 AABB 与 clip 求交 —— 完全不可见则跳过整棵子树
 *   2. 父控件边界裁剪 —— 子控件被父控件边界截断（除非 OVERFLOW_VISIBLE）
 *
 * 坐标空间策略（Option C — 控件本地坐标系）：
 *   - DFS 内部裁剪计算使用全局坐标（性能优化，避免重复遍历）
 *   - 在调用 widget->render() 前，将 surf、clip、parent_clip 平移到
 *     以控件自身左上角为原点 (0,0) 的本地坐标系
 *   - 控件绘制代码从 (0,0) 开始，widget->w/h 即为画布边界
 *
 * 性能要点：
 *   - 使用 ipgui_aabb_overlap 做快速剔除，避免遍历不可见子树
 *   - parent_clip 在递归中累积，避免每层都重新计算祖先链
 * ========================================================================= */
__IPGUI_STATIC__ void ipgui_screen_render_widget_dfs(
    struct widget_link_t * link,
    ipgui_widget_render_ctx_t * ctx)
{
    if (!link || !ctx) return;

    ipgui_widget_t * widget;

    /* 根节点 link 不是控件，从 first_child 开始遍历 */
    struct widget_link_t ** child = &link->first_child;
    while (*child) {
        widget = ipgui_container_of(*child, ipgui_widget_t, link);

        /* ---- 步骤 1: 检查可见性 ---- */
        if (widget->flags & IPGUI_WIDGET_FLAG_INVISIBLE) {
            child = &((*child)->sib_next);
            if (*child == link->first_child) break;
            continue;
        }

        /* ---- 步骤 2: 计算全局 AABB（用于裁剪判定） ---- */
        ipgui_aabb_t global_aabb;
        ipgui_widget_abs_pos(widget, &global_aabb);

        /* ---- 步骤 3: 与脏矩形切片(clip)求交（全局坐标） ---- */
        ipgui_aabb_t intersect;
        if (0 != ipgui_aabb_overlap(&intersect, &global_aabb, ctx->clip)) {
            /* 控件完全在脏矩形外，跳过整个子树 */
            child = &((*child)->sib_next);
            if (*child == link->first_child) break;
            continue;
        }

        /* ---- 步骤 4: 与父控件裁剪区求交（全局坐标） ---- */
        if (ctx->parent_clip) {
            ipgui_aabb_t clipped;
            if (0 != ipgui_aabb_overlap(&clipped, &intersect, ctx->parent_clip)) {
                /* 完全被父控件裁剪掉，跳过 */
                child = &((*child)->sib_next);
                if (*child == link->first_child) break;
                continue;
            }
            intersect = clipped;
        }

        /* ---- 步骤 5: 计算子控件的累积裁剪区（全局坐标，用于后续递归） ---- */
        ipgui_aabb_t child_clip;
        ipgui_aabb_t parent_global;
        ipgui_widget_local_to_global(widget, &parent_global);

        if (widget->flags & IPGUI_WIDGET_FLAG_OVERFLOW_VISIBLE) {
            /* OVERFLOW_VISIBLE: 不裁剪子控件，继承父级的 parent_clip */
            child_clip = global_aabb; /* 仅用于和 dirty rect 做交 */
        } else {
            /* 默认：子控件被限制在 parent_global 内 */
            child_clip = parent_global;
            /* 若父控件已有裁剪约束，取交集 */
            if (ctx->parent_clip) {
                if (0 != ipgui_aabb_overlap(&child_clip, &child_clip, ctx->parent_clip)) {
                    /* 交集为空则不渲染子控件 */
                    child_clip = parent_global; /* 回退，至少用自身边界 */
                }
            }
        }

        /* ---- 步骤 6: 构造控件本地坐标系渲染上下文并调用 render ---- */
        /*
         * Option C 坐标转换：将 surf.surf 和 clip 从全局坐标平移
         * 到以控件自身左上角为原点 (0,0) 的本地坐标系。
         *
         * 转换公式：local = global - global_aabb.start
         *
         * 缓冲区偏移公式（ipgui_surf_color_get 内部）：
         *   offset = (y - surf.surf.start.y)*stride + (x - surf.surf.start.x)*pix_size
         * 该公式在控件本地坐标下依然正确，因为 surf->color 仍指向 PFB 首字节，
         * 而 (local_y - surf.surf.start.y) = (global_y - widget_abs_y) - (dirty.start.y - widget_abs_y)
         *                                = global_y - dirty.start.y（与全局坐标系一致）
         */
        {
            ipgui_widget_render_ctx_t local_ctx;
            ipgui_surf_t              local_surf;
            ipgui_aabb_t              local_clip;
            ipgui_aabb_t              local_parent_clip_storage;

            /* 复制 surf 结构体，仅平移 surf.surf 的坐标空间 */
            local_surf = *ctx->surf;
            local_surf.surf.start.x  = ctx->surf->surf.start.x - global_aabb.start.x;
            local_surf.surf.start.y  = ctx->surf->surf.start.y - global_aabb.start.y;
            local_surf.surf.end.x    = ctx->surf->surf.end.x   - global_aabb.start.x;
            local_surf.surf.end.y    = ctx->surf->surf.end.y   - global_aabb.start.y;

            /* 平移 clip（脏矩形）到控件本地坐标 */
            local_clip.start.x = ctx->clip->start.x - global_aabb.start.x;
            local_clip.start.y = ctx->clip->start.y - global_aabb.start.y;
            local_clip.end.x   = ctx->clip->end.x   - global_aabb.start.x;
            local_clip.end.y   = ctx->clip->end.y   - global_aabb.start.y;

            /* 平移 parent_clip（父控件累积裁剪区）到控件本地坐标 */
            if (ctx->parent_clip) {
                local_parent_clip_storage.start.x = ctx->parent_clip->start.x - global_aabb.start.x;
                local_parent_clip_storage.start.y = ctx->parent_clip->start.y - global_aabb.start.y;
                local_parent_clip_storage.end.x   = ctx->parent_clip->end.x   - global_aabb.start.x;
                local_parent_clip_storage.end.y   = ctx->parent_clip->end.y   - global_aabb.start.y;
                local_ctx.parent_clip = &local_parent_clip_storage;
            } else {
                local_ctx.parent_clip = (ipgui_aabb_t *)0;
            }

            local_ctx.surf      = &local_surf;
            local_ctx.clip      = &local_clip;
            local_ctx.user_data = ctx->user_data;

            if (widget->render) {
                widget->render(widget, &local_ctx);
            }
        }

        /* ---- 步骤 7: 递归渲染子控件（传递全局坐标上下文） ---- */
        if ((*child)->first_child) {
            ipgui_widget_render_ctx_t sub_ctx;
            sub_ctx.surf      = ctx->surf;
            sub_ctx.clip      = ctx->clip;
            sub_ctx.user_data = ctx->user_data;

            if (widget->flags & IPGUI_WIDGET_FLAG_OVERFLOW_VISIBLE) {
                /* 子控件可溢出边界，继承当前 parent_clip（全局坐标） */
                sub_ctx.parent_clip = ctx->parent_clip;
            } else {
                /* 子控件受当前控件边界约束（全局坐标） */
                sub_ctx.parent_clip = &child_clip;
            }

            ipgui_screen_render_widget_dfs(*child, &sub_ctx);
        }

        /* ---- 步骤 8: 前进到下一个兄弟节点 ---- */
        child = &((*child)->sib_next);
        if (*child == link->first_child)
            break;
    }
}

__IPGUI_STATIC__ void ipgui_screen_render_dirty_rect_slice(
    ipgui_scr_t * scr,
    ipgui_pfb_t * pfb,/* here pfb size is as same as dirty size */
    ipgui_aabb_t * dirty)
{
    /* clear pfb */
    ipgui_coord_t w, h;
    w = ipgui_aabb_width(dirty);
    h = ipgui_aabb_height(dirty);

    /* 填充黑色（argb8888/bgra8888 下 0x00000000 = 完全透明黑） */
    ipgui_memset(pfb->color, 0, (u32_t)w * (u32_t)h * pfb->pix_size);

    /* 构造surf：由pfb切片生成绘制表面（根级别：全局坐标 = 本地坐标） */
    ipgui_surf_t surf;
    surf.surf    = * dirty;                        /* 脏矩形区域（屏幕绝对坐标） */
    surf.color   = pfb->color;                    /* PFB 首地址 */
    surf.stride  = (u32_t)w * pfb->pix_size;      /* 每行字节跨度 */
    surf.pix_fmt = pfb->pix_fmt;
    surf.pix_size = pfb->pix_size;

    /* 构造渲染上下文（根级别：控件树根控件位于 (0,0)，全局=本地） */
    ipgui_widget_render_ctx_t render_ctx;
    render_ctx.surf        = &surf;
    render_ctx.clip        = dirty;
    render_ctx.parent_clip = (ipgui_aabb_t *)0;   /* 根控件无父裁剪约束 */
    render_ctx.user_data   = (void *)0;

    /* 遍历控件树，渲染相交控件 */
    ipgui_screen_render_widget_dfs(&scr->tree.root, &render_ctx);

    /* 将pfb切片刷新到物理屏幕 */
    if (scr->drv && scr->drv->fill_region) {
        scr->drv->fill_region(
            scr,
            dirty->start.x,
            dirty->start.y,
            dirty->end.x,
            dirty->end.y,
            pfb->color,
            surf.stride);
    }
}

/* render dirty rect of screen */
__IPGUI_STATIC__ void ipgui_screen_render_dirty_rect(
    ipgui_scr_t * scr,
    ipgui_dirty_rect_t * dirty)
{
    ipgui_pfb_t * pfb = &scr->pfb;
    ipgui_aabb_t _dirty = {
        .start = {.x = dirty->x1, .y = dirty->y1}, 
        .end   = {.x = dirty->x2, .y = dirty->y2}
    };

    /* 将脏矩形区域切分成多个小块进行渲染
     * 直到全部遍历完为止
     */
    ipgui_rect_slice_ctx slice_ctx;
    ipgui_rect_slice_ctx_init(&slice_ctx, &_dirty, pfb->num_pixs);
    
    ipgui_aabb_t slice_rect;
    while (ipgui_get_rect_slice(&slice_ctx, &slice_rect)) {
        ipgui_screen_render_dirty_rect_slice(scr, pfb, &slice_rect);
    }
}

/* 
 * ipgui_screen_render
 * 屏幕渲染主入口——在每次帧刷新时调用。
 * 流程：
 *   1. 检查是否有脏区域，无则直接返回（零开销快速路径）
 *   2. 调用 ipgui_dirty_rect_flush() 做全局最优合并（减少渲染次数）
 *   3. 遍历合并后的脏矩形池，逐个调用 ipgui_screen_render_dirty_rect()
 *   4. 渲染完成后重置脏矩形管理器
 */
__IPGUI_API__ void ipgui_screen_render(ipgui_scr_t * scr)
{
    /* check if the screen have dirty region */
    if (scr->dirty.pool_num == 0) return;

    /* 渲染前最优合并一次 */
    ipgui_dirty_rect_flush(&scr->dirty);

    s32_t idx = 0;
    ipgui_dirty_rect_t * dirty; 
    for (; idx < scr->dirty.pool_num; idx ++) {
        dirty = &scr->dirty.pool[idx];
        ipgui_screen_render_dirty_rect(scr, dirty);
    }

    /* reset dirty rect manager */
    ipgui_dirty_rect_mgr_reset(&scr->dirty);
}

/*
 * hit-test 内部递归
 *
 * 从 root 的子控件中按 Z-order（从最上层到最下层）查找点 (x,y) 落在哪个控件上。
 *
 * 参数：
 *   root       : 当前层级父节点的 link（只遍历其 children）
 *   x, y       : 屏幕全局坐标
 *   parent_clip: 祖先链累积的可见区域（全局坐标），子控件必须在此区域内方可命中
 *
 * 遮挡与裁剪机制：
 *   Q1: 同层兄弟遮挡怎么处理？
 *       遍历顺序本身就是遮挡处理——从上往下遍历，上层先被检查，
 *       命中后立即 return，下层兄弟根本不会被问到。
 *   Q2: 父控件对子控件的裁剪怎么处理？
 *       visible = 控件全局AABB ∩ parent_clip，parent_clip 沿着递归向下传递并逐步收窄。
 *       子控件的可见区域不可能超过祖先累积裁剪区。
 *   Q3: INVISIBLE 控件能点击吗？
 *       能。INVISIBLE 只控制渲染，不影响 hit-test。DISABLED 才跳过。
 *       例如：声明一个 INVISIBLE 的全屏控件即可作为透明手势热区。
 *
 * 遍历规则：
 *   1. 同级从最后一个（最上层）往前（最下层）遍历，循环链表终止条件：child == first
 *   2. 跳过 DISABLED 控件（IPGUI_WIDGET_FLAG_DISABLED）
 *   3. 优先递归子控件（子控件画在父控件之上），子控件无人认领才返回父控件自身
 */
__IPGUI_STATIC__ ipgui_widget_t * ipgui_screen_point_on_recurse(
    struct widget_link_t  * root,
    ipgui_coord_t           x,
    ipgui_coord_t           y,
    const ipgui_aabb_t    * parent_clip)
{
    struct widget_link_t * first; /* Z-order最下层（first_child） */
    struct widget_link_t * child;

    first = root->first_child;
    if (!first)
        return (ipgui_widget_t *)0;

    /* 从最上层往最下层遍历（child = child->sib_prev），碰到first时终止 */
    child = first->sib_prev;

    for (;;) {
        ipgui_widget_t * w = ipgui_container_of(child, ipgui_widget_t, link);

        /* 跳过禁用的控件（INVISIBLE状态仅控制渲染，不影响点击） */
        if (!(w->flags & IPGUI_WIDGET_FLAG_DISABLED)) {
            ipgui_aabb_t aabb;
            ipgui_aabb_t visible;

            /* 控件全局包围盒 */
            ipgui_widget_abs_pos(w, &aabb);

            /* 与祖先累积裁剪区求交 = 控件实际可见区域 */
            if (parent_clip) {
                if (0 != ipgui_aabb_overlap(&visible, &aabb, parent_clip))
                    goto _next_sibling;
            } else {
                visible = aabb;
            }

            /* 点是否在可见区域内 */
            if (ipgui_point_in_aabb(x, y, &visible)) {

                /* 优先问子控件（子控件在上层） */
                ipgui_widget_t * hit = ipgui_screen_point_on_recurse(&w->link, x, y, &visible);
                if (hit)
                    return hit;

                /* 无子控件命中，返回当前控件 */
                return w;
            }
        }

    _next_sibling:
        if (child == first)
            break;
        child = child->sib_prev;
    }

    return (ipgui_widget_t *)0;
}

/* 屏幕上的hit-test：返回点 (x,y) 处最上层的控件 */
__IPGUI_API__ ipgui_widget_t * ipgui_screen_point_on(
    ipgui_scr_t  * scr,
    ipgui_coord_t  x,
    ipgui_coord_t  y)
{
    ipgui_aabb_t screen_aabb;

    if (!scr)
        return (ipgui_widget_t *)0;

    /* 检查点是否在屏幕范围内 */
    screen_aabb.start.x = 0;
    screen_aabb.start.y = 0;
    screen_aabb.end.x   = scr->drv->xreso - 1;
    screen_aabb.end.y   = scr->drv->yreso - 1;

    if (!ipgui_point_in_aabb(x, y, &screen_aabb))
        return (ipgui_widget_t *)0;

    /* 以屏幕作为初始parent_clip，从根节点开始递归 */
    return ipgui_screen_point_on_recurse(&scr->tree.root, x, y, &screen_aabb);
}

__IPGUI_API__ void ipgui_screen_handle_widget_event(ipgui_scr_t * scr, ipgui_widget_evt_t * evt)
{
    if (!scr || !evt) return;

    /* 按 Z-order 逆序遍历控件树（最后绘制的在最上面，最先接收到事件）
     * 找到第一个包含事件坐标的控件，调用其事件处理回调
     * 当前为预留框架，待控件事件系统完善后实现完整的事件分发逻辑
     */
    (void)scr;
    (void)evt;
}

__IPGUI_API__ void ipgui_screen_putpixel(ipgui_scr_t * scr, ipgui_coord_t x, ipgui_coord_t y, u8_t * pix)
{
    if (scr && scr->drv && scr->drv->put_pixel) {
        scr->drv->put_pixel(scr, x, y, pix);
    }
}

__IPGUI_API__ void ipgui_screen_flush(ipgui_scr_t * scr)
{
    if (scr && scr->drv && scr->drv->flush) {
        scr->drv->flush(scr);
    }
}

__IPGUI_API__ void ipgui_screen_fill_region(ipgui_scr_t * scr, 
        ipgui_coord_t x1, ipgui_coord_t y1, ipgui_coord_t x2, ipgui_coord_t y2, 
        u8_t * pix_buf, s32_t stride)
{
    if (scr && scr->drv && scr->drv->fill_region) {
        scr->drv->fill_region(scr, x1, y1, x2, y2, pix_buf, stride);
    }
}
