/*位置 (x, y)

尺寸 (width, height)

内边距 (padding)

外边距 (margin)

最小/最大尺寸约束*/
#include "ipgui_widget_geometry.h"

__IPGUI_API__ ipgui_widget_t * ipgui_widget_get_topest_at(struct widget_link_t * root, ipgui_coord_t x, ipgui_coord_t y)
{
    
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

// __IPGUI_STATIC__ void ipgui_aabb_move_rel(ipgui_aabb_t * aabb, ipgui_coord_t rel_x, ipgui_coord_t rel_y)
// {
//     aabb->start.x += rel_x;
//     aabb->start.y += rel_y;
//     aabb->end.x   += rel_x;
//     aabb->end.y   += rel_y;
// }

// ipgui_widget_t * ipgui_widget_get_parent(ipgui_widget_t * widget)
// {
//     return widget->parent;
// }

// ipgui_widget_t * ipgui_widget_get_root(ipgui_widget_t * widget)
// {
//     //return widget->parent;
// }

// /* generate absolute active region of widget in screen(任意控件相对于屏幕的位置) */
// __IPGUI_API__ int ipgui_widget_gen_act_aabb_global(ipgui_widget_t * widget, ipgui_aabb_t * res)
// {
//     ipgui_aabb_t temp;
//     ipgui_widget_t * parent, * root;
//     ipgui_coord_t rel_x, rel_y;

//     temp.start.x = widget->rect.start.x;
//     temp.start.y = widget->rect.start.y;
//     temp.end.x   = widget->rect.end.x;
//     temp.end.y   = widget->rect.end.y;
//     parent = ipgui_widget_get_parent(widget);
//     while (parent) {
//         rel_x = parent->rect.start.x;
//         rel_y = parent->rect.start.y;
//         ipgui_aabb_move_rel(&temp, rel_x, rel_y);
//         parent = ipgui_widget_get_parent(parent);
//     }
//     root = ipgui_widget_get_root(widget);
//     if (0 != ipgui_aabb_intersect(res, &temp, &root->rect)) {
//         res->start.x = res->start.y = 0;
//         res->end.y = res->end.x = -1;
//         return -1;
//     }
//     return 0;
// }

// /* generate absolute active region of widget in screen and save to widget->act_aabb */
// __IPGUI_API__ int ipgui_widget_sync_act_aabb(ipgui_widget_t * widget, void * args)
// {
//     return ipgui_widget_gen_act_aabb_global(widget, &widget->act_aabb);
// }

// /* generate active aabb of widget in local coordinate system（忘了作用，用不到） */
// __IPGUI_API__ int ipgui_widget_gen_act_aabb_local(ipgui_widget_t * widget, ipgui_aabb_t * res)
// {
//     ipgui_aabb_t temp;
//     ipgui_widget_t * parent;
//     ipgui_coord_t rel_x, rel_y;

//     if (0 != ipgui_widget_gen_act_aabb_global(widget, &temp)) {
//         * res = temp;
//         return -1;
//     }
//     parent = ipgui_widget_get_parent(widget);
//     while (parent) {
//         rel_x = parent->rect.start.x;
//         rel_y = parent->rect.start.y;
//         ipgui_aabb_move_rel(&temp, -rel_x, -rel_y);
//         parent = ipgui_widget_get_parent(parent);
//     }
//     rel_x = widget->rect.start.x;
//     rel_y = widget->rect.start.y;
//     ipgui_aabb_move_rel(&temp, -rel_x, -rel_y);
//     * res = temp;
//     return 0;
// }

// /* generate visible aabb of widget in screen coordinate system（控件在屏幕中用户可见的位置（考虑被父控件裁剪）） */
// __IPGUI_API__ int ipgui_widget_gen_visible_aabb_global(ipgui_widget_t * widget, ipgui_aabb_t * res, int flag)
// {
//     ipgui_aabb_t temp, temp1;
//     int surrounded;
//     ipgui_widget_t * parent;

//     if (ipgui_widget_is_flag_set(widget, IPGUI_WIDGET_FLAG_INVISIBLE))
//         return -1;

//     /* if root widget, return it's active axis aligned bounding box */
//     if (!widget->parent) {
//         ipgui_widget_gen_act_aabb_global(widget, &widget->act_aabb);
//         * res = widget->act_aabb;
//         return 0;
//     }

//     /* if widget is surrounded by parent, 
//      * all of children are surrounded by parent too !!! 
//      */
//     surrounded = flag;
//     surrounded |= !!!ipgui_widget_is_flag_set(widget, IPGUI_WIDGET_FLAG_OVERVIEW_PARENT);

//     ipgui_widget_gen_act_aabb_global(widget, &temp);
//     parent = ipgui_widget_get_parent(widget);
//     if (parent) {
//         if (0 != ipgui_widget_gen_visible_aabb_global(parent, &temp1, 0))
//             return -1;
//         while (parent) {
//             if (!parent->parent) break;
//             surrounded |= !!!ipgui_widget_is_flag_set(parent, IPGUI_WIDGET_FLAG_OVERVIEW_PARENT);
//             if (surrounded) break;
//             parent = ipgui_widget_get_parent(parent);
//         }
//         if (surrounded) {
//             if ((0 != ipgui_aabb_intersect(&temp, &temp, &temp1)))
//                 return -1;
//         }
//     }
//     * res = temp;
//     return 0;
// }

// /* generate visible aabb of widget in screen coordinate system（控件在屏幕中自身可见的位置（考虑被父控件裁剪）） */
// __IPGUI_API__ int ipgui_widget_gen_visible_aabb_local(ipgui_widget_t * widget, ipgui_aabb_t * res)
// {
//     ipgui_aabb_t temp;
//     ipgui_widget_t * parent;

//     if (ipgui_widget_gen_visible_aabb_global(widget, &temp, 0)) {
//         * res = temp;
//         return -1;
//     }
//     parent = ipgui_widget_get_parent(widget);
//     while (parent) {
//         temp.start.x -= parent->rect.start.x;
//         temp.start.y -= parent->rect.start.y;
//         temp.end.x   -= parent->rect.start.x;
//         temp.end.y   -= parent->rect.start.y;
//         parent = ipgui_widget_get_parent(parent);
//     }
//     temp.start.x -= widget->rect.start.x;
//     temp.start.y -= widget->rect.start.y;
//     temp.end.x   -= widget->rect.start.x;
//     temp.end.y   -= widget->rect.start.y;
//     * res = temp;
//     return 0;
// }

// /* 全局坐标转换为控件的可见aabb的相对偏移(相对于控件的左上角全局坐标) */
// __IPGUI_API__ ipgui_point_t ipgui_widget_global2_visible_local_offset(
//     ipgui_widget_t * widget, ipgui_point_t * gp)
// {
//     ipgui_aabb_t visible;
//     ipgui_point_t ret;

//     ipgui_widget_gen_visible_aabb_global(widget, &visible, 0);
//     ret.x = gp->x - visible.start.x;
//     ret.y = gp->y - visible.start.y;

//     return ret;
// }
