#include "ipgui_draw_line.h"

extern premult_blend_func_t premult_blend_table[PIX_FMT_MAX];

/* Wu's draw thin line algorithm */
__IPGUI_API__ void ipgui_draw_thin_line(
        ipgui_surf_t       * surf, 
        ipgui_aabb_t       * clip,
        ipgui_line_t       * line, 
        ipgui_line_style_t * style)
{
    if ((!surf) || (!line) || (!style) || (style->opacity < 3))
        return;

    if (style->width > 1)
        return;

    if (style->paint.type == IPGUI_PAINT_IMAGE)
        return;

    ipgui_aabb_t self, draw;
    self.start.x = IPGUI_MIN(line->start.x, line->end.x);
    self.start.y = IPGUI_MIN(line->start.y, line->end.y);
    self.end.x   = IPGUI_MAX(line->start.x, line->end.x);
    self.end.y   = IPGUI_MAX(line->start.y, line->end.y);

    if (clip) {
        if (0 != ipgui_aabb_overlap(&draw, clip, &surf->surf))
            return;
    } else {
        draw = surf->surf;
    }

    if (0 != ipgui_aabb_overlap(&draw, &draw, &self))
        return;

    char flat;
    ipgui_coord_t x1 = line->start.x;
    ipgui_coord_t y1 = line->start.y;
    ipgui_coord_t x2 = line->end.x;
    ipgui_coord_t y2 = line->end.y;
    ipgui_coord_t dx_abs = IPGUI_ABS(x2 - x1);
    ipgui_coord_t dy_abs = IPGUI_ABS(y2 - y1);
    flat = dx_abs > dy_abs ? 1 : 0;

    ipgui_coord_t ix, iy;
    u8_t * cr;
    u8_t alpha;
    s32_t cover; /* 0-255 */
    s32_t err;
    ipgui_color_t premult;ipgui_color_t paint_cr;
    premult_blend_func_t blend_func = premult_blend_table[surf->pix_fmt];

    if (flat) {
        ipgui_coord_t y_step;
        ipgui_coord_t dy;
        ipgui_coord_t ys, xs = IPGUI_MIN(x1, x2); /* 从小x值开始 */
        if (xs == x1) { 
            ys = y1;
            dy = y2 - y1;
        } else {
            ys = y2;
            dy = y1 - y2;
        }
        if (dy > 0) y_step = 1;
        else y_step = -1;
        err = dy * (draw.start.x - xs); /* 起始误差 */
        
        ipgui_coord_t x, y = ys + (err / dx_abs);
        err %= dx_abs;
        for (x = draw.start.x; x <= draw.end.x; x ++) {
            if (IPGUI_ABS(err) >= dx_abs) {
                if (err < 0) {
                    err += dx_abs;
                } else {
                    err -= dx_abs;
                }
                y += y_step;
            }
            cover = 255 - (IPGUI_ABS(err) * 255 / dx_abs);
            alpha = cover * style->opacity >> 8;
            if (ipgui_point_in_aabb(x, y, &draw)) {
                ix = x - surf->surf.start.x;
                iy = y - surf->surf.start.y;
                cr = ipgui_surf_color_get(surf, ix, iy);

                if (style->paint.type == IPGUI_PAINT_COLOR) paint_cr = style->paint.src.color;
                else if (style->paint.type == IPGUI_PAINT_GRADIENT) {
                    if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_LINEAR) {
                        ipgui_liner_gradient_color_get(&style->paint.src.grad_src.grad.liner_grad, ipgui_get_liner_gradient_pos_at_xy(&style->paint.src.grad_src.grad.liner_grad, x, y), &paint_cr);
                    } else if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_RADIAL) {
                        ipgui_radial_gradient_color_get(&style->paint.src.grad_src.grad.radial_grad, ipgui_get_radial_gradient_pos_at_xy(&style->paint.src.grad_src.grad.radial_grad, x, y), &paint_cr);
                    } else if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_CONIC) {
                        ipgui_conic_gradient_color_get(&style->paint.src.grad_src.grad.conic_grad, ipgui_get_conic_gradient_pos_at_xy(&style->paint.src.grad_src.grad.conic_grad, x, y), &paint_cr);
                    }
                }

                premult = ipgui_color_combine_opacity_and_premultiply(&paint_cr, alpha);
                blend_func(premult, cr, style->blend_mode);
            }
            cover = 255 - cover;
            alpha = cover * style->opacity >> 8;
            if (ipgui_point_in_aabb(x, (y + y_step), &draw)) {
                ix = x - surf->surf.start.x;
                iy = y + y_step - surf->surf.start.y;
                cr = ipgui_surf_color_get(surf, ix, iy);

                if (style->paint.type == IPGUI_PAINT_COLOR) paint_cr = style->paint.src.color;
                else if (style->paint.type == IPGUI_PAINT_GRADIENT) {
                    if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_LINEAR) {
                        ipgui_liner_gradient_color_get(&style->paint.src.grad_src.grad.liner_grad, ipgui_get_liner_gradient_pos_at_xy(&style->paint.src.grad_src.grad.liner_grad, x, y), &paint_cr);
                    } else if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_RADIAL) {
                        ipgui_radial_gradient_color_get(&style->paint.src.grad_src.grad.radial_grad, ipgui_get_radial_gradient_pos_at_xy(&style->paint.src.grad_src.grad.radial_grad, x, y), &paint_cr);
                    } else if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_CONIC) {
                        ipgui_conic_gradient_color_get(&style->paint.src.grad_src.grad.conic_grad, ipgui_get_conic_gradient_pos_at_xy(&style->paint.src.grad_src.grad.conic_grad, x, y), &paint_cr);
                    }
                }

                premult = ipgui_color_combine_opacity_and_premultiply(&paint_cr, alpha);
                blend_func(premult, cr, style->blend_mode);
            }
            err += dy;
        }
    } else {
        ipgui_coord_t x_step;
        ipgui_coord_t dx;
        ipgui_coord_t xs, ys = IPGUI_MIN(y1, y2); /* 从小y值开始 */
        if (ys == y1) {
            xs = x1;
            dx = x2 - x1;
        } else {
            xs = x2;
            dx = x1 - x2;
        }
        if (dx > 0) x_step = 1;
        else x_step = -1;
        err = dx * (draw.start.y - ys); /* 起始误差 */

        ipgui_coord_t x = xs + (err / dy_abs), y;
        err %= dy_abs;
        for (y = draw.start.y; y <= draw.end.y; y ++) {
            if (IPGUI_ABS(err) >= dy_abs) {
                if (err < 0) {
                    err += dy_abs;
                } else {
                    err -= dy_abs;
                }
                x += x_step;
            }
            cover = 255 - (IPGUI_ABS(err) * 255 / dy_abs);
            alpha = cover * style->opacity >> 8;
            if (ipgui_point_in_aabb(x, y, &draw)) {
                ix = x - surf->surf.start.x;
                iy = y - surf->surf.start.y;
                cr = ipgui_surf_color_get(surf, ix, iy);

                if (style->paint.type == IPGUI_PAINT_COLOR) paint_cr = style->paint.src.color;
                else if (style->paint.type == IPGUI_PAINT_GRADIENT) {
                    if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_LINEAR) {
                        ipgui_liner_gradient_color_get(&style->paint.src.grad_src.grad.liner_grad, ipgui_get_liner_gradient_pos_at_xy(&style->paint.src.grad_src.grad.liner_grad, x, y), &paint_cr);
                    } else if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_RADIAL) {
                        ipgui_radial_gradient_color_get(&style->paint.src.grad_src.grad.radial_grad, ipgui_get_radial_gradient_pos_at_xy(&style->paint.src.grad_src.grad.radial_grad, x, y), &paint_cr);
                    } else if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_CONIC) {
                        ipgui_conic_gradient_color_get(&style->paint.src.grad_src.grad.conic_grad, ipgui_get_conic_gradient_pos_at_xy(&style->paint.src.grad_src.grad.conic_grad, x, y), &paint_cr);
                    }
                }

                premult = ipgui_color_combine_opacity_and_premultiply(&paint_cr, alpha);
                blend_func(premult, cr, style->blend_mode);
            }
            cover = 255 - cover;
            alpha = cover * style->opacity >> 8;
            if (ipgui_point_in_aabb((x + x_step), y, &draw)) {
                ix = x + x_step - surf->surf.start.x;
                iy = y - surf->surf.start.y;
                cr = ipgui_surf_color_get(surf, ix, iy);

                if (style->paint.type == IPGUI_PAINT_COLOR) paint_cr = style->paint.src.color;
                else if (style->paint.type == IPGUI_PAINT_GRADIENT) {
                    if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_LINEAR) {
                        ipgui_liner_gradient_color_get(&style->paint.src.grad_src.grad.liner_grad, ipgui_get_liner_gradient_pos_at_xy(&style->paint.src.grad_src.grad.liner_grad, x, y), &paint_cr);
                    } else if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_RADIAL) {
                        ipgui_radial_gradient_color_get(&style->paint.src.grad_src.grad.radial_grad, ipgui_get_radial_gradient_pos_at_xy(&style->paint.src.grad_src.grad.radial_grad, x, y), &paint_cr);
                    } else if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_CONIC) {
                        ipgui_conic_gradient_color_get(&style->paint.src.grad_src.grad.conic_grad, ipgui_get_conic_gradient_pos_at_xy(&style->paint.src.grad_src.grad.conic_grad, x, y), &paint_cr);
                    }
                }

                premult = ipgui_color_combine_opacity_and_premultiply(&paint_cr, alpha);
                blend_func(premult, cr, style->blend_mode);
            }
            err += dx;
        }
    }
}

/* draw a horizontal line */
__IPGUI_STATIC__ void ipgui_draw_hor_line(       
    ipgui_surf_t       * surf,
    ipgui_aabb_t       * clip,
    ipgui_line_t       * line,
    ipgui_line_style_t * style)
{
    ipgui_aabb_t self, draw;

    /* clip surf first */
    if (clip) {
        if (0 != ipgui_aabb_overlap(&draw, &surf->surf, clip))
            return;/* not intersect, then just return */
    } else {
        draw = surf->surf;
    }

    /* calc horizontal line region */
    self.start.x = IPGUI_MIN(line->start.x, line->end.x);
    self.end.x   = IPGUI_MAX(line->start.x, line->end.x);
    self.start.y = line->start.y - (style->width >> 1);
    self.end.y   = self.start.y  + style->width - 1;

    /* secondly, clip self */
    if (0 != ipgui_aabb_overlap(&draw, &self, &draw))
        return;/* not intersect, then just return */

    ipgui_blend(
        surf, 
        (ipgui_aabb_t *)0,
        &draw, 
        &style->paint, 
        style->opacity,
        (u8_t *)0,
        (ipgui_aabb_t *)0,
        style->blend_mode);

    if (style->cap == IPGUI_LINE_CAP_ROUND) {
        /* 画半圆 */
    }

}

/* draw a vertical line */
__IPGUI_STATIC__ void ipgui_draw_ver_line(       
        ipgui_surf_t       * surf,
        ipgui_aabb_t       * clip,
        ipgui_line_t       * line,
        ipgui_line_style_t * style)
{
    ipgui_aabb_t self, draw;

    /* clip surf first */
    if (clip) {
        if (0 != ipgui_aabb_overlap(&draw, &surf->surf, clip))
            return;/* not intersect, then just return */
    } else {
        draw = surf->surf;
    }

    /* calc vertical line region */
    self.start.y = IPGUI_MIN(line->start.y, line->end.y);
    self.end.y   = IPGUI_MAX(line->start.y, line->end.y);
    self.start.x = line->start.x - (style->width >> 1);
    self.end.x   = self.start.x  + style->width - 1;

    /* secondly, clip self */
    if (0 != ipgui_aabb_overlap(&draw, &self, &draw))
        return;/* not intersect, then just return */

    ipgui_blend(
        surf, 
        (ipgui_aabb_t *)0,
        &draw, 
        &style->paint, 
        style->opacity,
        (u8_t *)0,
        (ipgui_aabb_t *)0,
        style->blend_mode);

    if (style->cap == IPGUI_LINE_CAP_ROUND) {
        /* 画半圆 */
    }
}

__IPGUI_STATIC__ void ipgui_draw_line_round_cap(
                ipgui_aabb_t       * draw,
                ipgui_line_t       * line,
                ipgui_line_style_t * style)
{

}

__IPGUI_STATIC__ void ipgui_draw_line_butt_cap(
                ipgui_aabb_t       * draw,
                ipgui_line_t       * line,
                ipgui_line_style_t * style)
{

}

__IPGUI_API__ void ipgui_draw_line(       
                ipgui_surf_t       * surf,
                ipgui_aabb_t       * clip,
                ipgui_line_t       * line, 
                ipgui_line_style_t * style)
{
    if ((!surf) || (!line) || (!style))
        return;

    if (style->opacity < 3)
        return;

    if (line->start.x == line->end.x) {
        ipgui_draw_ver_line(surf, clip, line, style);
        return;
    }
    else if (line->start.y == line->end.y) {
        ipgui_draw_hor_line(surf, clip, line, style);
        return;
    }

    /* clip surf and get draw aabb */
    ipgui_aabb_t draw, self;
    if (clip) {
        if (0 != ipgui_aabb_overlap(&draw, &surf->surf, clip))
            return;/* not intersect, then just return */
    } else {
        draw = surf->surf;
    }

    /* calc line region and clip self with draw */
    // self.start.y = IPGUI_MIN(line->start.y, line->end.y);
    // self.end.y   = IPGUI_MAX(line->start.y, line->end.y);
    // self.start.x = line->start.x - (style->width >> 1);
    // self.end.x   = self.start.x  + style->width - 1;

    if (0 != ipgui_aabb_overlap(&draw, &self, &draw))
        return;/* not intersect, then just return */

        
    if (style->cap == IPGUI_LINE_CAP_ROUND)
        ipgui_draw_line_round_cap(&draw, line, style);
    else if (style->cap == IPGUI_LINE_CAP_BUTT) ;
        ipgui_draw_line_butt_cap(&draw, line, style);
}