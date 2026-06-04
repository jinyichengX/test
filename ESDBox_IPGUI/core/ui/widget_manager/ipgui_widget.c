/*
 * MIT License
 *
 * Copyright (c) 2025 JinYiCheng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ipgui_widget.h"
#include "ipgui_memory.h"
#include "ipgui_defs.h"
#include "ipgui_debug.h"
#include "ipgui_screen.h"

/* set widget's relative position to parent */
__IPGUI_API__ void ipgui_widget_set_rel_pos(ipgui_widget_t * widget, ipgui_coord_t xpos, ipgui_coord_t ypos)
{
    widget->rect.start.x = xpos;
    widget->rect.start.y = ypos;
}

__IPGUI_API__ ipgui_widget_t * ipgui_widget_create(ipgui_widget_t * parent)
{
    if (!parent) {
        // if( scr != ipgui_widget_get_screen(parent))
        //     return (ipgui_widget_t *)0;
    }
    
    ipgui_widget_t * widget = (ipgui_widget_t *)ipgui_mem_alloc(ipgui_smem, sizeof(ipgui_widget_t));

    if (widget)
    {
        ipgui_memset(widget, 0, sizeof(ipgui_widget_t));
        // if(ipgui_widget_set_parent(scr, widget, parent)!= IPGUI_ERR_OK){
        //     ipgui_mem_free(ipgui_smem, widget);
        //     return (ipgui_widget_t *)0;
        // }
        /* default widget size and position */
        if (!parent) {
            widget->rect.start.x = 0;
            widget->rect.start.y = 0;
            // widget->rect.end.x = scr->drv->xreso - 1;
            // widget->rect.end.y = scr->drv->yreso - 1;
        } else {
            widget->rect.start.x = 0;
            widget->rect.start.y = 0;
            widget->rect.end.x = 50;
            widget->rect.end.y = 30;
        }

        widget->all_dirty = 1;
        ipgui_darray_init(&widget->dirty_region, sizeof(ipgui_aabb_t));
        ipgui_darray_init(&widget->user_event_cb, sizeof(ipgui_widget_event_handler_t));

        /* widget layer init */
        widget->childs = (ipgui_widget_t *)0;
        widget->child_num = 0;


        /* attributes init */
        widget->flags       = 0;
        widget->visible     = 1;
        widget->allow_drag  = 1;
        widget->has_focus   = 0;
        widget->focused     = 0;
        widget->dirty       = 0;
        widget->active      = 1;
        widget->opacity     = 255;
        widget->focus_policy = IPGUI_FOCUS_BY_ALL;
    }
    
    return widget;
}


/* traverse widget tree dfs */
__IPGUI_API__ void ipgui_widget_foreach_dfs(ipgui_widget_t * root, wid_ops_t ops, void * args)
{
    if (!root)
        return;
    ipgui_widget_t ** child = &root->childs;
    if (ops)
        // ipgui_dbg_info("name = %s\r\n", root->name);            /* 用ops替代 */
        /* add ops here */
        ops(root, args);
    while (*child) {
        if ((*child)->childs) {
            ipgui_widget_foreach_dfs(*child, ops, args);
        } else if (ops) {
            // ipgui_dbg_info("name = %s\r\n", (*child)->name);    /* 用ops替代 */
            /* add ops here */
            ops(*child, args);
        }
        child = &((*child)->next);
        if (*child == root->childs)
            break;
    }
}

/* if two widget in the same layer collide, if return 1, else return 0 */
__IPGUI_API__ int ipgui_widget_collide_in_same_layer(ipgui_widget_t * widget1, ipgui_widget_t * widget2)
{
    ipgui_rect_t rect1;
    if (widget1->parent != widget2->parent)
        return 0;
    return (0 == ipgui_aabb_intersect(&rect1, &widget1->rect, &widget2->rect)) ? 1 : 0;
}

/* register widget events */
__IPGUI_API__ int ipgui_widget_register_event(ipgui_widget_t * widget, unsigned int codes,
                                            ipgui_widget_event_cb_t cb, void * args)
{
    ipgui_widget_event_handler_t handler;
    handler.codes = codes;
    handler.event_cb = cb;
    handler.args = args;
    if(-1 == ipgui_darray_element_append(&widget->user_event_cb, &handler, 1))
        return -1;
    else return 0;
}

/* handle widget events */
__IPGUI_API__ int ipgui_widget_event_handler(ipgui_widget_t * widget, unsigned int codes)
{
    unsigned int idx;
    ipgui_widget_event_handler_t * iter;
    for (idx = 0; idx < widget->user_event_cb.elem_size; idx ++) {
        iter = (ipgui_widget_event_handler_t *)\
        ipgui_darray_index(&widget->user_event_cb, idx);
        if ((!iter) || (!iter->event_cb)) continue;
        unsigned int filt_code;
        if (filt_code = (iter->codes & codes)) {
            iter->event_cb(filt_code, iter->args);
        } else continue;
    }
    return 0;
}

__IPGUI_STATIC__ void ipgui_aabb_move_rel(ipgui_aabb_t * aabb, ipgui_coord_t rel_x, ipgui_coord_t rel_y)
{
    aabb->start.x += rel_x;
    aabb->start.y += rel_y;
    aabb->end.x   += rel_x;
    aabb->end.y   += rel_y;
}

ipgui_widget_t * ipgui_widget_get_parent(ipgui_widget_t * widget)
{
    return widget->parent;
}

ipgui_widget_t * ipgui_widget_get_root(ipgui_widget_t * widget)
{
    //return widget->parent;
}

/* generate absolute active region of widget in screen(任意控件相对于屏幕的位置) */
__IPGUI_API__ int ipgui_widget_gen_act_aabb_global(ipgui_widget_t * widget, ipgui_aabb_t * res)
{
    ipgui_aabb_t temp;
    ipgui_widget_t * parent, * root;
    ipgui_coord_t rel_x, rel_y;

    temp.start.x = widget->rect.start.x;
    temp.start.y = widget->rect.start.y;
    temp.end.x   = widget->rect.end.x;
    temp.end.y   = widget->rect.end.y;
    parent = ipgui_widget_get_parent(widget);
    while (parent) {
        rel_x = parent->rect.start.x;
        rel_y = parent->rect.start.y;
        ipgui_aabb_move_rel(&temp, rel_x, rel_y);
        parent = ipgui_widget_get_parent(parent);
    }
    root = ipgui_widget_get_root(widget);
    if (0 != ipgui_aabb_intersect(res, &temp, &root->rect)) {
        res->start.x = res->start.y = 0;
        res->end.y = res->end.x = -1;
        return -1;
    }
    return 0;
}

/* generate absolute active region of widget in screen and save to widget->act_aabb */
__IPGUI_API__ int ipgui_widget_sync_act_aabb(ipgui_widget_t * widget, void * args)
{
    return ipgui_widget_gen_act_aabb_global(widget, &widget->act_aabb);
}

/* generate active aabb of widget in local coordinate system（忘了作用，用不到） */
__IPGUI_API__ int ipgui_widget_gen_act_aabb_local(ipgui_widget_t * widget, ipgui_aabb_t * res)
{
    ipgui_aabb_t temp;
    ipgui_widget_t * parent;
    ipgui_coord_t rel_x, rel_y;

    if (0 != ipgui_widget_gen_act_aabb_global(widget, &temp)) {
        * res = temp;
        return -1;
    }
    parent = ipgui_widget_get_parent(widget);
    while (parent) {
        rel_x = parent->rect.start.x;
        rel_y = parent->rect.start.y;
        ipgui_aabb_move_rel(&temp, -rel_x, -rel_y);
        parent = ipgui_widget_get_parent(parent);
    }
    rel_x = widget->rect.start.x;
    rel_y = widget->rect.start.y;
    ipgui_aabb_move_rel(&temp, -rel_x, -rel_y);
    * res = temp;
    return 0;
}

/* generate visible aabb of widget in screen coordinate system（控件在屏幕中用户可见的位置（考虑被父控件裁剪）） */
__IPGUI_API__ int ipgui_widget_gen_visible_aabb_global(ipgui_widget_t * widget, ipgui_aabb_t * res, int flag)
{
    ipgui_aabb_t temp, temp1;
    int surrounded;
    ipgui_widget_t * parent;

    if (ipgui_widget_is_flag_set(widget, IPGUI_WIDGET_FLAG_INVISIBLE))
        return -1;

    /* if root widget, return it's active axis aligned bounding box */
    if (!widget->parent) {
        ipgui_widget_gen_act_aabb_global(widget, &widget->act_aabb);
        * res = widget->act_aabb;
        return 0;
    }

    /* if widget is surrounded by parent, 
     * all of children are surrounded by parent too !!! 
     */
    surrounded = flag;
    surrounded |= !!!ipgui_widget_is_flag_set(widget, IPGUI_WIDGET_FLAG_OVERVIEW_PARENT);

    ipgui_widget_gen_act_aabb_global(widget, &temp);
    parent = ipgui_widget_get_parent(widget);
    if (parent) {
        if (0 != ipgui_widget_gen_visible_aabb_global(parent, &temp1, 0))
            return -1;
        while (parent) {
            if (!parent->parent) break;
            surrounded |= !!!ipgui_widget_is_flag_set(parent, IPGUI_WIDGET_FLAG_OVERVIEW_PARENT);
            if (surrounded) break;
            parent = ipgui_widget_get_parent(parent);
        }
        if (surrounded) {
            if ((0 != ipgui_aabb_intersect(&temp, &temp, &temp1)))
                return -1;
        }
    }
    * res = temp;
    return 0;
}

/* generate visible aabb of widget in screen coordinate system（控件在屏幕中自身可见的位置（考虑被父控件裁剪）） */
__IPGUI_API__ int ipgui_widget_gen_visible_aabb_local(ipgui_widget_t * widget, ipgui_aabb_t * res)
{
    ipgui_aabb_t temp;
    ipgui_widget_t * parent;

    if (ipgui_widget_gen_visible_aabb_global(widget, &temp, 0)) {
        * res = temp;
        return -1;
    }
    parent = ipgui_widget_get_parent(widget);
    while (parent) {
        temp.start.x -= parent->rect.start.x;
        temp.start.y -= parent->rect.start.y;
        temp.end.x   -= parent->rect.start.x;
        temp.end.y   -= parent->rect.start.y;
        parent = ipgui_widget_get_parent(parent);
    }
    temp.start.x -= widget->rect.start.x;
    temp.start.y -= widget->rect.start.y;
    temp.end.x   -= widget->rect.start.x;
    temp.end.y   -= widget->rect.start.y;
    * res = temp;
    return 0;
}

/* 全局坐标转换为控件的可见aabb的相对偏移(相对于控件的左上角全局坐标) */
__IPGUI_API__ ipgui_point_t ipgui_widget_global2_visible_local_offset(
    ipgui_widget_t * widget, ipgui_point_t * gp)
{
    ipgui_aabb_t visible;
    ipgui_point_t ret;

    ipgui_widget_gen_visible_aabb_global(widget, &visible, 0);
    ret.x = gp->x - visible.start.x;
    ret.y = gp->y - visible.start.y;

    return ret;
}


/* point on which widget on the topest layer */
__IPGUI_API__ ipgui_widget_t * ipgui_widget_topest_on(ipgui_widget_t * root, ipgui_point_t * p)
{
    // ipgui_aabb_t visible;
    // ipgui_widget_t * found, * iter;

    // if (ipgui_widget_is_flag_set(root, IPGUI_WIDGET_FLAG_INVISIBLE))
    //     return (ipgui_widget_t *)0;
    // if (ipgui_widget_gen_visible_aabb_global(root, &visible, 0)) {
    //     return (ipgui_widget_t *)0;
    // }
    // if (ipgui_point_in_rect(p, &visible)) {
    //     if (root->child_num == 0)
    //         return root;
    //     int child_num;
    //     for (child_num = 0, iter = root->childs->prev;\
    //             child_num < root->child_num; \
    //             iter = iter->prev, child_num ++)
    //     {
    //         found = ipgui_widget_topest_on(iter, p);
    //         if (found)
    //             return found;
    //     }
    // } else return (ipgui_widget_t *)0;

    // return root;
}
// // 正确的绘制调度
// void ipgui_widget_draw_recursive(ipgui_widget_t* widget, ipgui_tile_t * parent_tile) {
//     if (!widget->visible) return;
    
//     // 1. 获取屏幕绝对可见区域
//     ipgui_aabb_t screen_visible;
//     ipgui_widget_gen_visible_aabb_global(widget, &screen_visible);
    
//     // 2. 创建子tile（继承父tile的缓冲区，但有自己的偏移）
//     ipgui_tile_t widget_tile = *parent_tile;  // 复制父tile
//     widget_tile.x1 = screen_visible.start.x;  // 设置新偏移
//     widget_tile.y1 = screen_visible.start.y;
    
//     // 3. 调用控件的绘制函数
//     if (widget->draw) {
//         widget->draw(widget, &widget_tile);  // 传子tile
//     }
    
//     // 4. 递归绘制子控件（传递同一个子tile）
//     ipgui_widget_t* child = widget->childs;
//     if (child) {
//         do {
//             ipgui_widget_draw_recursive(child, &widget_tile);  // 注意传widget_tile
//             child = child->next;
//         } while (child != widget->childs);
//     }
// }
/* widget scroll begin */

/* widget scroll end */

/* add widget dirty region */
void ipgui_widget_dirty_region_add(ipgui_widget_t * widget, ipgui_aabb_t * dirty, int num)
{
    /* if not enough mem, mark whole widget dirty */
    if (ipgui_darray_element_append(&widget->dirty_region, dirty, num) != 0) {
        ipgui_darray_deinit(&widget->dirty_region);
        widget->all_dirty = 1;
    } else {
        widget->dirty = 1;
    }
}

void ipgui_widget_mark_dirty_recursive(ipgui_widget_t * widget, int rel_x, int rel_y)
{
    ipgui_aabb_t new_aabb_abs;
    ipgui_aabb_t aabb_intersect;
    ipgui_widget_t * iter;
    /* calculate new abs aabb first */
    new_aabb_abs.start.x = rel_x + widget->act_aabb.start.x;
    new_aabb_abs.start.y = rel_y + widget->act_aabb.start.y;
    new_aabb_abs.end.x = rel_x + widget->act_aabb.end.x;
    new_aabb_abs.end.y = rel_y + widget->act_aabb.end.y;

    ipgui_widget_dirty_region_add(widget, &new_aabb_abs, 1); /* children all dirty */

    iter = widget->next;
    while (iter != widget) {
        ipgui_aabb_intersect(&aabb_intersect, &new_aabb_abs, &iter->act_aabb);

        ipgui_widget_dirty_region_add(iter, &aabb_intersect, 1);
        iter = iter->next;
    }
}

__IPGUI_STATIC__ ipgui_widget_move_rel_impl(ipgui_widget_t * widget, int rel_x, int rel_y)
{
    if (rel_x == 0 && rel_y == 0) {
        return;
    }
    widget->rect.start.x += rel_x;
    widget->rect.start.y += rel_y;
    widget->rect.end.x   += rel_x;
    widget->rect.end.y   += rel_y;
}

__IPGUI_STATIC__ ipgui_widget_move_rel(ipgui_widget_t * widget, int rel_x, int rel_y)
{
    
}

/* clip aabb for segment render */
__IPGUI_STATIC__ int ipgui_clip_aabb_with_buffer(
    ipgui_aabb_t * ret, ipgui_aabb_t * aabb,
    void * buffer, int size, char pixel_size, int * valid_size)
{

    int line_width = aabb->end.x - aabb->start.x + 1;
    int line_size = line_width * pixel_size;
    int vert_num;

    ret->start.x = aabb->start.x;
    ret->start.y = aabb->start.y;
    ret->end.x = aabb->end.x;

    vert_num = size / line_size;
    if (vert_num > 0) {
        ret->end.y = aabb->start.y + (vert_num - 1);
        if (ret->end.y > aabb->end.y) {
            ret->end.y = aabb->end.y;
        }
        vert_num = ret->end.y - ret->start.y + 1;
    } else {
        ret->start.x = ret->end.x = ret->start.y = ret->end.y = 0;
        return -1;
    }
    *valid_size = vert_num * line_size;

    return 0;
}

__IPGUI_STATIC__ void ipgui_widget_redraw_done(ipgui_widget_t * widget)
{
    widget->dirty = 0;
    widget->all_dirty = 0;
    ipgui_darray_deinit(&widget->dirty_region);
}

void ipgui_widget_redraw_dirty_region(ipgui_widget_t * widget, void * buffer, int buffer_size)
{
    ipgui_widget_t * iter;

    if (!buffer) return;
    if (widget->dirty != 1) return;

    ipgui_aabb_t dirty_region;
    ipgui_aabb_t * abs_dirty = (ipgui_aabb_t *)widget->dirty_region.elements;

    int dirty_size;

    if (widget->all_dirty == 1) {

    } else {
        
    }
    abs_dirty = (ipgui_aabb_t *)widget->dirty_region.elements;

    ipgui_clip_aabb_with_buffer(&dirty_region, abs_dirty, buffer, 
        buffer_size, sizeof(ipgui_color_t), &dirty_size);

    while (dirty_size != 0) {
        /* handle dirty level by level */
        widget->draw(widget, (void *)&dirty_region);

        /* reclip region */
        abs_dirty->start.y = dirty_region.end.y + 1;
        ipgui_clip_aabb_with_buffer(&dirty_region, abs_dirty, buffer, 
            buffer_size, sizeof(ipgui_color_t), &dirty_size);
    }

    ipgui_widget_redraw_done(widget);
}

void ipgui_widget_draw_region(ipgui_widget_t * widget, ipgui_aabb_t * region, void * buffer, int buffer_size)
{
    ipgui_aabb_t clip_region;
    ipgui_clip_aabb_with_buffer(&clip_region, region, buffer, 
        buffer_size, sizeof(ipgui_color_t), &buffer_size);

    while (buffer_size != 0) {
        /* draw self first
         * then draw children
         */
        widget->draw(widget, (void *)&clip_region);

        /* draw children */
        ipgui_widget_t * iter = widget->next;
        while (iter != widget) {
            ipgui_widget_draw_region(iter, &clip_region, buffer, buffer_size);
            iter = iter->next;
        }

        /* reclip region */
        clip_region.start.y = clip_region.end.y + 1;
        ipgui_clip_aabb_with_buffer(&clip_region, region, buffer, 
            buffer_size, sizeof(ipgui_color_t), &buffer_size);
    }
}

/* 全局重渲染widget及其所有子控件 */
void ipgui_redraw_all(ipgui_widget_t * widget, void * buffer, int buffer_size)
{
    /* get abs position of the widget */


    /* clip widget's aabb */

}