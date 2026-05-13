// /* 
// 图片在ipgui_image_show_attr_t中的ipgui_aabb_t dest显示，显示模式由ipgui_image_mode_t控制，
// dest中显示完图像后最后再由tile裁剪到屏幕上。（非程序流程而是符合人类直觉的流程） */

// #include "ipgui_image_ras.h"
// #include "ipgui_math.h"

// void image_show_orig_impl()
// {

// }

// void image_show_impl(ipgui_tile_t * tile, 
//     ipgui_aabb_t * dest, ipgui_aabb_t * draw, 
//     ipgui_img_dsc_t * img, ipgui_image_show_attr_t * attr)
// {
//     int sm = (int)(attr->show_mode);

//     if (sm <= IPGUI_IMG_SHOW_ORI_MAX_FLAG) {
//         /* clip draw */
//         ipgui_aabb_t orig;
//         switch (sm)
//         {
//         case IPGUI_IMG_SHOW_ORI_CENTER:
//         case IPGUI_IMG_SHOW_ORI_TOP:
//         case IPGUI_IMG_SHOW_ORI_BOTTOM:
//         case IPGUI_IMG_SHOW_ORI_LEFT:
//         case IPGUI_IMG_SHOW_ORI_RIGHT:
//         case IPGUI_IMG_SHOW_ORI_TOP_LEFT:
//         case IPGUI_IMG_SHOW_ORI_TOP_RIGHT:
//         case IPGUI_IMG_SHOW_ORI_BOTTOM_LEFT:
//         case IPGUI_IMG_SHOW_ORI_BOTTOM_RIGHT:
//             image_show_orig_impl(tile, dest, draw, img, attr->lerp, attr->opacity, attr->show_mode);
//         break;
//         }
//     } else if (sm <= IPGUI_IMG_SHOW_FIT_BOTTOM_RIGHT) {

//     }
//         // case IPGUI_IMG_SHOW_FIT_CENTER:
//         // case IPGUI_IMG_SHOW_FIT_TOP_LEFT:
//         // case IPGUI_IMG_SHOW_FIT_TOP_RIGHT:
//         // case IPGUI_IMG_SHOW_FIT_BOTTOM_LEFT:
//         // case IPGUI_IMG_SHOW_FIT_BOTTOM_RIGHT:
//         // image_show_fit_impl(tile, dest, draw, img, attr);
//         // break;

//         // case IPGUI_IMG_SHOW_FILL:
//         // image_show_fill_impl(tile, dest, draw, img, attr);
//         // break;

//     //     default: break;
//     // }
// }

// __IPGUI_API__ void ipgui_image_show(
//     ipgui_tile_t * tile, /* 目标tile */
//     ipgui_aabb_t * clip, /* 裁剪区域 */
//     ipgui_aabb_t * dest, /* 显示图片的目标区域 */
//     ipgui_img_dsc_t * img,
//     ipgui_image_show_attr_t * attr)
// {
//     if (!tile || !attr || !img || !clip) return;
//     if (attr->opacity < 4) return;
//     if (!img->pixmap || img->w == 0 || img->h == 0) return;

//     ipgui_aabb_t buffer, draw;
//     buffer = ipgui_tile_aabb_gen(tile); /* generate tile aabb: buffer */
    
//     if (clip) {
//         if (0 != ipgui_aabb_intersect(&draw, &buffer, clip))
//             return;
//     } else {
//         draw = buffer;
//     }

//     /* clip dest(draw is contained by dest) */
//     if (0 != ipgui_aabb_intersect(&draw, &draw, dest))
//         return;
    
//     image_show_impl(tile, dest, &draw, img, attr);
// }