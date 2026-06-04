/*绘制函数指针,裁剪区域,缓冲区引用,纹理/图像资源*/

#include "ipgui_widget_render.h"
#include "ipgui_core.h"

/* area is a region rel to widget's parent, 
 * it is in the widghet's coordinate system 
 */

//  /*从根控件（屏幕控件）开始重绘脏矩形，那么只需要从根控件以Z序遍历下去就能把整个脏区完全绘制出来了*/
// __IPGUI_API__ void ipgui_widget_render(ipgui_widget_t * widget, ipgui_aabb_t * area)
// {
//     if(ipgui_aabb_empty(area)) return;


// }

// /* 向上传递脏矩形时，遍历父控件并偏移相对于父控件的相对位置直到根控件，
//  * 最后得到相对于根控件的坐标，再将这个坐标标记为脏区域
//  */

// /* the dr(dirty rectangle) is the rel area to widget's parent */
// __IPGUI_API__ void ipgui_mark_dirty(ipgui_aabb_t * dr, ipgui_widget_t * widget)
// {
    
// }

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

__IPGUI_API__ void ipgui_draw_dirty(ipgui_surf_t * surf, ipgui_aabb_t * dr)
{

}