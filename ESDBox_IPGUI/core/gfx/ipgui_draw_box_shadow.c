#include "ipgui_draw_box_shadow.h"
#include "ipgui_blur.h"

__IPGUI_STATIC__ u8_t * gen_gauss_kernel(ipgui_coord_t blur)
{
  u8_t * gus_mask;
  gus_mask = ipgui_mask_buf_acquire();
  if (!gus_mask)
    return (u8_t *)0;

  ipgui_mask_surface_t in, out;
  out.mask = in.mask = gus_mask;
  out.w = out.h = 10;
  in.w = in.h = 2;

  ipgui_average_blur_hor(&in, blur, &out);
  ipgui_average_blur_ver(&out, blur, &out);

  return gus_mask;
}

__IPGUI_STATIC__ void release_gauss_kernel(u8_t * gus_mask)
{
  ipgui_mask_buf_release(gus_mask);
}

void ipgui_draw_box_outer_shadow(
  ipgui_surf_t              * surf,
  ipgui_aabb_t              * clip,
  ipgui_aabb_t              * box,
  ipgui_box_style_t         * style,
  ipgui_box_outer_shadow_style_t  * shadow_style)
{
  if (!surf || !box || !shadow_style 
      || (shadow_style->opacity < 3) || (shadow_style->blur < 0))
    return;

  /* 判断是否需要绘制，draw只在画圆角起作用 */
  ipgui_aabb_t draw;
  if (clip) {
      if (0 != ipgui_aabb_overlap(&draw, clip, &surf->surf))
          return;
  } else draw = surf->surf;

  ipgui_aabb_t box_self;
  box_self.start.x = box->start.x - (style ? style->left_padding   : 0);
  box_self.start.y = box->start.y - (style ? style->top_padding    : 0);
  box_self.end.x   = box->end.x   + (style ? style->right_padding  : 0);
  box_self.end.y   = box->end.y   + (style ? style->bottom_padding : 0);

  /* shift padding box */
  /* 这里就得到了要画的阴影的参考盒子 */
  box_self.start.x += shadow_style->offset_x;
  box_self.end.x   += shadow_style->offset_x;
  box_self.start.y += shadow_style->offset_y;
  box_self.end.y   += shadow_style->offset_y;

  /* calculate shadow box */
  ipgui_aabb_t shadow_box;
  shadow_box = box_self;
  shadow_box.start.x -= shadow_style->blur;
  shadow_box.end.x   -= shadow_style->blur;
  shadow_box.start.y -= shadow_style->blur;
  shadow_box.end.y   -= shadow_style->blur;
  
  /* use draw to clip shadow box */
  ipgui_aabb_clip(&shadow_box, clip, &draw);

  /* get gauss blur kernel */
  u8_t * conv_knl;
  conv_knl = gen_gauss_kernel(shadow_style->blur);
  if (!conv_knl) /* there is no enough memory */
    return;
  
  // /* 规整圆角半径，不超过padding box的半宽/半高的最小值 */
  // ipgui_coord_t r_lt, r_rt, r_lb, r_rb;
  // get_max_radius(&padding_box, style, &r_lt, &r_rt, &r_lb, &r_rb);

  // ipgui_coord_t l_max = IPGUI_MAX(r_lt, r_lb);
  // ipgui_coord_t r_max = IPGUI_MAX(r_rt, r_rb);
  // ipgui_coord_t l_min = IPGUI_MIN(r_lt, r_lb);
  // ipgui_coord_t r_min = IPGUI_MIN(r_rt, r_rb);

  /* release guass mask */
  release_gauss_kernel(conv_knl);

}