#include "ipgui_widget.h"
#include "ipgui_memory.h"
#include "ipgui_defs.h"

#include "ipgui_screen.h"

/* create widget */
__IPGUI_API__ ipgui_widget_t * ipgui_widget_create(ipgui_widget_t * parent)
{
    ipgui_widget_t * widget = (ipgui_widget_t *)
        ipgui_mem_alloc_def(sizeof(ipgui_widget_t));

    if (!widget) return (ipgui_widget_t *)0;

    /* init widget position and size */
    widget->x = 0;
    widget->y = 0;
    widget->w = 0;
    widget->h = 0;

    /* init flags */
    widget->flags = IPGUI_WIDGET_FLAG_NONE;

    /* init widget tree link node */
    ipgui_widget_link_init(&widget->link);
    /* add to parent's child list */
    if (parent) {
        ipgui_widget_link_set_parent(&widget->link, &parent->link);
        // ipgui_widget_mark_dirty();
    } else {   
        /* it is a detached widget, leave it */;
    }
    
    /* init event handler and render callback */
    widget->event_handler = 0;
    widget->render        = 0;

    return widget;
}

/* 设置渲染回调 */
__IPGUI_API__ void ipgui_widget_set_render(
    ipgui_widget_t * widget,
    void (*render)(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx))
{
    if (widget)
        widget->render = render;
}

/* 设置事件处理回调 */
__IPGUI_API__ void ipgui_widget_set_event_handler(
    ipgui_widget_t * widget,
    void (*handler)(struct ipgui_widget * widget, ipgui_widget_evt_t * evt))
{
    if (widget)
        widget->event_handler = handler;
}

/* 获取控件所在屏幕 */
__IPGUI_API__ ipgui_scr_t * ipgui_widget_get_screen(ipgui_widget_t * widget)
{
    if (!widget) return (ipgui_scr_t *)0;

    if (IPGUI_YES == ipgui_widget_link_is_detached(&widget->link))
        return (ipgui_scr_t *)0;

    struct widget_link_t * _root;
    ipgui_scr_t * scr;
    _root = ipgui_widget_link_get_root(&widget->link);

    /* 根节点的 link 嵌入在 ipgui_scr_t.tree.root 中 */
    scr = ipgui_container_of(_root, ipgui_scr_t, tree.root);
    return scr;
}

/**
 * @brief 计算控件在屏幕中的绝对像素坐标(AABB包围盒)，且不考虑被父控件裁剪
 * @param widget 目标控件句柄
 * @param r 输出参数：存储控件的屏幕绝对坐标包围盒
 *          -> start: 控件左上角绝对坐标
 *          -> end  : 控件右下角绝对坐标
 */
__IPGUI_API__ void ipgui_widget_abs_pos(ipgui_widget_t * widget, ipgui_aabb_t * r)
{
    if (!widget) return;

    /* if it is detached from tree, return */
    if(IPGUI_YES == ipgui_widget_link_is_detached(&widget->link))
        return;

    /* 初始化累加变量，先填入当前控件的相对坐标与大小 */
    ipgui_coord_t abs_x = widget->x;
    ipgui_coord_t abs_y = widget->y;

    /* 沿着树向上追溯，累加所有父控件的坐标偏移 */
    struct widget_link_t * _link = &widget->link;
    ipgui_widget_t * parent;
    while (_link->parent) {
        parent = ipgui_container_of(_link->parent, ipgui_widget_t, link);
        
        abs_x += parent->x;
        abs_y += parent->y;
        
        _link = _link->parent;
    }

    r->start.x = abs_x;
    r->start.y = abs_y;
    r->end.x   = abs_x + widget->w - 1;
    r->end.y   = abs_y + widget->h - 1;
}

/* 将父控件局部坐标转为全局坐标系的 aabb */
__IPGUI_API__ void ipgui_widget_local_to_global(
    ipgui_widget_t * widget, ipgui_aabb_t * out)
{
    if (!widget || !out) return;

    out->start.x = widget->x;
    out->start.y = widget->y;
    out->end.x   = widget->x + widget->w - 1;
    out->end.y   = widget->y + widget->h - 1;

    if (IPGUI_YES == ipgui_widget_link_is_detached(&widget->link))
        return;

    struct widget_link_t * _link = &widget->link;
    ipgui_widget_t * parent;
    while (_link->parent) {
        parent = ipgui_container_of(_link->parent, ipgui_widget_t, link);
        
        out->start.x += parent->x;
        out->start.y += parent->y;
        out->end.x   += parent->x;
        out->end.y   += parent->y;
        
        _link = _link->parent;
    }
}

__IPGUI_API__ void ipgui_widget_mark_dirty(ipgui_widget_t * widget)
{
    if (!widget) return;

    /* if it is detached from tree, return */
    if(IPGUI_YES == ipgui_widget_link_is_detached(&widget->link))
        return;

    /* 标记自身为脏 */
    widget->flags |= IPGUI_WIDGET_FLAG_DIRTY;

    /* 获取控件所在的树的根节点tree->link 
     * 再由根节点找到对应的屏幕
     */
    ipgui_scr_t * scr = ipgui_widget_get_screen(widget);
    if (!scr) return;

    /* 计算控件的绝对坐标 */
    ipgui_aabb_t dr;
    ipgui_widget_abs_pos(widget, &dr);

    /* clip with screen's resolution */
    ipgui_aabb_t scr_aabb;
    scr_aabb.start.x = 0;
    scr_aabb.start.y = 0;
    scr_aabb.end.x = scr->drv->xreso - 1;
    scr_aabb.end.y = scr->drv->yreso - 1;

    if(0 != ipgui_aabb_overlap(&dr, &dr, &scr_aabb))
        return;
    
    ipgui_dirty_rect_add(&scr->dirty, (ipgui_dirty_rect_t *)&dr);
}
