#include "ipgui_path.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))


ipgui_err_t ipgui_path_fixed_add (ipgui_path_t *path,
		       char	     op,
		       const ipgui_point_t  *points,
		       int		     num_points);
__IPGUI_API__ void ipgui_path_init(ipgui_path_t * path)
{
    // cairo_list_init (&path->buf.base.link);

    path->buf.base.num_ops = 0;
    path->buf.base.num_points = 0;
    path->buf.base.size_ops = ARRAY_LENGTH (path->buf.op);
    path->buf.base.size_points = ARRAY_LENGTH (path->buf.points);
    path->buf.base.op = path->buf.op;
    path->buf.base.points = path->buf.points;

    path->current_point.x = 0;
    path->current_point.y = 0;
    path->last_move_point = path->current_point;

    path->has_current_point = FALSE;
    path->needs_move_to = TRUE;
    path->has_extents = FALSE;
    path->has_curve_to = FALSE;
    path->stroke_is_rectilinear = TRUE;
    path->fill_is_rectilinear = TRUE;
    path->fill_maybe_region = TRUE;
    path->fill_is_empty = TRUE;

    // path->extents.p1.x = path->extents.p1.y = 0;
    // path->extents.p2.x = path->extents.p2.y = 0;
}
void
ipgui_path_fixed_new_sub_path (ipgui_path_t *path)
{
    if (path->needs_move_to==0) {
        /* If the current subpath doesn't need_move_to, it contains at least one command */
        if (path->fill_is_rectilinear) {
            /* Implicitly close for fill */
            path->fill_is_rectilinear = path->current_point.x == path->last_move_point.x ||
                        path->current_point.y == path->last_move_point.y;
            path->fill_maybe_region &= path->fill_is_rectilinear;
        }
        path->needs_move_to = TRUE;
    }

    path->has_current_point = FALSE;
}
ipgui_err_t
ipgui_path_fixed_move_to (ipgui_path_t  *path,
			   ipgui_coord_t	x,
			   ipgui_coord_t	y)
{
    ipgui_path_fixed_new_sub_path (path);

    path->has_current_point = TRUE;
    path->current_point.x = x;
    path->current_point.y = y;
    path->last_move_point = path->current_point;

    return IPGUI_ERR_OK;
}
#include <stdio.h>
//应用 move_to 操作，更新路径状态
static ipgui_err_t
ipgui_path_fixed_move_to_apply (ipgui_path_t  *path)
{
    if (likely (! path->needs_move_to))
	return IPGUI_ERR_OK;

    path->needs_move_to = FALSE;

    if (path->has_extents) {
	    //_cairo_box_add_point (&path->extents, &path->current_point);
    } else {
        //_cairo_box_set (&path->extents, &path->current_point, &path->current_point);
        path->has_extents = TRUE;
    }

    if (path->fill_maybe_region) {
        // path->fill_maybe_region = _cairo_fixed_is_integer (path->current_point.x) &&
        //             _cairo_fixed_is_integer (path->current_point.y);
        printf("fill maybe region\n");
    }

    path->last_move_point = path->current_point;

    return ipgui_path_fixed_add (path, IPGUI_PATH_CODE_MOVE_TO, &path->current_point, 1);
}
/* 倒数第2个点 */
static inline const ipgui_point_t *
ipgui_path_fixed_penultimate_point (ipgui_path_t *path)
{
    ipgui_path_buf_t *buf;

    //buf = cairo_path_tail (path);
    buf = &path->buf.base;

    if (likely (buf->num_points >= 2)) {
	    return &buf->points[buf->num_points - 2];
    } else {
        // ipgui_path_buf_t *prev_buf = cairo_path_buf_prev (buf);

        // assert (prev_buf->num_points >= 2 - buf->num_points);
        // return &prev_buf->points[prev_buf->num_points - (2 - buf->num_points)];
        printf("last op\n");
    }
}
/* 最后一个操作 */
static char
ipgui_path_fixed_last_op (ipgui_path_t *path)
{
    ipgui_path_buf_t * buf;

    //buf = cairo_path_tail (path);
    buf = &path->buf.base;
    //assert (buf->num_ops != 0);

    return buf->op[buf->num_ops - 1];
}
/* 删除最后一个line_to，且lineto操作必须是最后一个path to操作 */
static void
ipgui_path_fixed_drop_line_to (ipgui_path_t *path)
{
    ipgui_path_buf_t *buf;

    //assert (ipgui_path_fixed_last_op (path) == IPGUI_PATH_CODE_LINE_TO);
    //buf = cairo_path_tail (path);
    buf = &path->buf.base;

    buf->num_points--;
    buf->num_ops--;
}

ipgui_err_t ipgui_path_fixed_add (ipgui_path_t *path,
		       char	     op,
		       const ipgui_point_t  *points,
		       int		     num_points)
{
    ipgui_path_buf_t *buf;

    //assert (buf->num_ops < buf->size_ops);
    //buf->op[buf->num_ops++] = op;
    //buf->points[buf->num_points++] = point;
    //assert (buf->num_points <= buf->size_points);
    //return IPGUI_ERR_OK;
    printf("add op\n");
    return IPGUI_ERR_OK;
}

ipgui_err_t
_cairo_path_fixed_line_to (ipgui_path_t *path,
			   ipgui_coord_t	x,
			   ipgui_coord_t	y)
{
    ipgui_err_t status;
    ipgui_point_t point;

    point.x = x;
    point.y = y;

    /* When there is not yet a current point, the line_to operation
     * becomes a move_to instead. Note: We have to do this by
     * explicitly calling into _cairo_path_fixed_move_to to ensure
     * that the last_move_point state is updated properly.
     */
    if (! path->has_current_point)
	return ipgui_path_fixed_move_to (path, point.x, point.y);

    status = ipgui_path_fixed_move_to_apply (path);
    if (unlikely (status))
	return status;

    /* If the previous op was but the initial MOVE_TO and this segment
     * is degenerate, then we can simply skip this point. Note that
     * a move-to followed by a degenerate line-to is a valid path for
     * stroking, but at all other times is simply a degenerate segment.
     */
    if (ipgui_path_fixed_last_op (path) != IPGUI_PATH_CODE_MOVE_TO) {
	if (x == path->current_point.x && y == path->current_point.y)
	    return IPGUI_ERR_OK;
    }

    /* If the previous op was also a LINE_TO with the same gradient,
     * then just change its end-point rather than adding a new op.
     */
    if (ipgui_path_fixed_last_op (path) == IPGUI_PATH_CODE_LINE_TO) {
        const ipgui_point_t *p;

        p = ipgui_path_fixed_penultimate_point (path);
        if (p->x == path->current_point.x && p->y == path->current_point.y) {
            /* previous line element was degenerate, replace */
            ipgui_path_fixed_drop_line_to (path);
        } else {
            // cairo_slope_t prev, self;

            // _cairo_slope_init (&prev, p, &path->current_point);
            // _cairo_slope_init (&self, &path->current_point, &point);
            // if (_cairo_slope_equal (&prev, &self) &&
            // /* cannot trim anti-parallel segments whilst stroking */
            //     ! _cairo_slope_backwards (&prev, &self))
            // {
            //     ipgui_path_fixed_drop_line_to (path);
            //     /* In this case the flags might be more restrictive than
            //     * what we actually need.
            //     * When changing the flags definition we should check if
            //     * changing the line_to point can affect them.
            //     */
            // }
        }
    }

    if (path->stroke_is_rectilinear) {
        path->stroke_is_rectilinear = path->current_point.x == x ||
                        path->current_point.y == y;
        path->fill_is_rectilinear &= path->stroke_is_rectilinear;
        path->fill_maybe_region &= path->fill_is_rectilinear;
        if (path->fill_maybe_region) {
            // path->fill_maybe_region = _cairo_fixed_is_integer (x) &&
            //             _cairo_fixed_is_integer (y);
                    printf("fill maybe region\n");
        }
        if (path->fill_is_empty) {
            path->fill_is_empty = path->current_point.x == x &&
                    path->current_point.y == y;
        }
    }

    path->current_point = point;

    //_cairo_box_add_point (&path->extents, &point);

    return ipgui_path_fixed_add (path, IPGUI_PATH_CODE_LINE_TO, &point, 1);
}
ipgui_err_t
ipgui_path_fixed_rel_line_to (ipgui_path_t *path,
			       ipgui_coord_t	   dx,
			       ipgui_coord_t	   dy)
{
    if (unlikely (! path->has_current_point))
	return IPGUI_ERR_NOK;

    return _cairo_path_fixed_line_to (path,
				      path->current_point.x + dx,
				      path->current_point.y + dy);
}
// static struct cg_path_t * cg_path_create(void)
// {
// 	struct cg_path_t * path = malloc(sizeof(struct cg_path_t));
// 	path->contours = 0;
// 	path->start.x = 0.0;
// 	path->start.y = 0.0;

//     path->elements.data = NULL;
//     path->elements.size = 0;
//     path->elements.capacity = 0;

//     path->points.data = NULL;
//     path->points.size = 0;
//     path->points.capacity = 0;
// 	return path;
// }

// static void cg_path_destroy(struct cg_path_t * path)
// {
// 	if(path)
// 	{
// 		if(path->elements.data)
// 		if(path->points.data)
// 			free(path->points.data);
// 		free(path);
// 	}
// }

// static inline void cg_path_get_current_point(struct cg_path_t * path, double * x, double * y)
// {
// 	if(path->points.size == 0)
// 	{
// 		*x = 0.0;
// 		*y = 0.0;
// 	}
// 	else
// 	{
// 		*x = path->points.data[path->points.size - 1].x;
// 		*y = path->points.data[path->points.size - 1].y;
// 	}
// }

// static void cg_path_move_to(struct cg_path_t * path, double x, double y)
// {
// 	cg_array_ensure(path->elements, 1);
// 	cg_array_ensure(path->points, 1);

// 	path->elements.data[path->elements.size] = CG_PATH_ELEMENT_MOVE_TO;
// 	path->elements.size += 1;
// 	path->contours += 1;
// 	path->points.data[path->points.size].x = x;
// 	path->points.data[path->points.size].y = y;
// 	path->points.size += 1;
// 	path->start.x = x;
// 	path->start.y = y;
// }

// static void cg_path_line_to(struct cg_path_t * path, double x, double y)
// {
// 	cg_array_ensure(path->elements, 1);
// 	cg_array_ensure(path->points, 1);

// 	path->elements.data[path->elements.size] = CG_PATH_ELEMENT_LINE_TO;
// 	path->elements.size += 1;
// 	path->points.data[path->points.size].x = x;
// 	path->points.data[path->points.size].y = y;
// 	path->points.size += 1;
// }

// static void cg_path_curve_to(struct cg_path_t * path, double x1, double y1, double x2, double y2, double x3, double y3)
// {
// 	cg_array_ensure(path->elements, 1);
// 	cg_array_ensure(path->points, 3);

// 	path->elements.data[path->elements.size] = CG_PATH_ELEMENT_CURVE_TO;
// 	path->elements.size += 1;
// 	struct cg_point_t * points = path->points.data + path->points.size;
// 	points[0].x = x1;
// 	points[0].y = y1;
// 	points[1].x = x2;
// 	points[1].y = y2;
// 	points[2].x = x3;
// 	points[2].y = y3;
// 	path->points.size += 3;
// }

// static void cg_path_quad_to(struct cg_path_t * path, double x1, double y1, double x2, double y2)
// {
// 	double x, y;
// 	cg_path_get_current_point(path, &x, &y);

// 	double cx = 2.0 / 3.0 * x1 + 1.0 / 3.0 * x;//二次贝塞尔曲线转三次贝塞尔曲线
// 	double cy = 2.0 / 3.0 * y1 + 1.0 / 3.0 * y;
// 	double cx1 = 2.0 / 3.0 * x1 + 1.0 / 3.0 * x2;
// 	double cy1 = 2.0 / 3.0 * y1 + 1.0 / 3.0 * y2;
// 	cg_path_curve_to(path, cx, cy, cx1, cy1, x2, y2);
// }

// static void cg_path_close(struct cg_path_t * path)
// {
// 	if(path->elements.size == 0)
// 		return;
// 	if(path->elements.data[path->elements.size - 1] == CG_PATH_ELEMENT_CLOSE)
// 		return;
// 	cg_array_ensure(path->elements, 1);
// 	cg_array_ensure(path->points, 1);
// 	path->elements.data[path->elements.size] = CG_PATH_ELEMENT_CLOSE;
// 	path->elements.size += 1;
// 	path->points.data[path->points.size].x = path->start.x;
// 	path->points.data[path->points.size].y = path->start.y;
// 	path->points.size += 1;
// }

// static void cg_path_rel_move_to(struct cg_path_t * path, double dx, double dy)
// {
// 	double x, y;
// 	cg_path_get_current_point(path, &x, &y);
// 	cg_path_move_to(path, dx + x, dy + y);
// }

// static void cg_path_rel_line_to(struct cg_path_t * path, double dx, double dy)
// {
// 	double x, y;
// 	cg_path_get_current_point(path, &x, &y);
// 	cg_path_line_to(path, dx + x, dy + y);
// }

// static void cg_path_rel_curve_to(struct cg_path_t * path, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3)
// {
// 	double x, y;
// 	cg_path_get_current_point(path, &x, &y);
// 	cg_path_curve_to(path, dx1 + x, dy1 + y, dx2 + x, dy2 + y, dx3 + x, dy3 + y);
// }

// static void cg_path_rel_quad_to(struct cg_path_t * path, double dx1, double dy1, double dx2, double dy2)
// {
// 	double x, y;
// 	cg_path_get_current_point(path, &x, &y);
// 	cg_path_quad_to(path, dx1 + x, dy1 + y, dx2 + x, dy2 + y);
// }

// static inline void cg_path_add_rectangle(struct cg_path_t * path, double x, double y, double w, double h)
// {
// 	cg_path_move_to(path, x, y);
// 	cg_path_line_to(path, x + w, y);
// 	cg_path_line_to(path, x + w, y + h);
// 	cg_path_line_to(path, x, y + h);
// 	cg_path_line_to(path, x, y);
// 	cg_path_close(path);
// }

// static inline void cg_path_add_round_rectangle(struct cg_path_t * path, double x, double y, double w, double h, double rx, double ry)
// {
// 	rx = CG_MIN(rx, w * 0.5);
// 	ry = CG_MIN(ry, h * 0.5);

// 	double right = x + w;
// 	double bottom = y + h;
// 	double cpx = rx * 0.55228474983079339840;
// 	double cpy = ry * 0.55228474983079339840;

// 	cg_path_move_to(path, x, y + ry);
// 	cg_path_curve_to(path, x, y + ry - cpy, x + rx - cpx, y, x + rx, y);
// 	cg_path_line_to(path, right - rx, y);
// 	cg_path_curve_to(path, right - rx + cpx, y, right, y + ry - cpy, right, y + ry);
// 	cg_path_line_to(path, right, bottom - ry);
// 	cg_path_curve_to(path, right, bottom - ry + cpy, right - rx + cpx, bottom, right - rx, bottom);
// 	cg_path_line_to(path, x + rx, bottom);
// 	cg_path_curve_to(path, x + rx - cpx, bottom, x, bottom - ry + cpy, x, bottom - ry);
// 	cg_path_line_to(path, x, y + ry);
// 	cg_path_close(path);
// }

// static void cg_path_add_ellipse(struct cg_path_t * path, double cx, double cy, double rx, double ry)
// {
// 	double left = cx - rx;
// 	double top = cy - ry;
// 	double right = cx + rx;
// 	double bottom = cy + ry;
// 	double cpx = rx * 0.55228474983079339840;
// 	double cpy = ry * 0.55228474983079339840;

// 	cg_path_move_to(path, cx, top);
// 	cg_path_curve_to(path, cx + cpx, top, right, cy - cpy, right, cy);
// 	cg_path_curve_to(path, right, cy + cpy, cx + cpx, bottom, cx, bottom);
// 	cg_path_curve_to(path, cx - cpx, bottom, left, cy + cpy, left, cy);
// 	cg_path_curve_to(path, left, cy - cpy, cx - cpx, top, cx, top);
// 	cg_path_close(path);
// }

// static void cg_path_add_arc(struct cg_path_t * path, double cx, double cy, double r, double a0, double a1, int ccw)
// {
// 	double da = a1 - a0;
// 	if(fabs(da) > 6.28318530717958647693)
// 	{
// 		da = 6.28318530717958647693;
// 	}
// 	else if(da != 0.0 && ccw != (da < 0.0))
// 	{
// 		da += 6.28318530717958647693 * (ccw ? -1 : 1);
// 	}
// 	int seg_n = (int)(ceil(fabs(da) / 1.57079632679489661923));
// 	double seg_a = da / seg_n;
// 	double d = (seg_a / 1.57079632679489661923) * 0.55228474983079339840 * r;
// 	double a = a0;
// 	double ax = cx + cos(a) * r;
// 	double ay = cy + sin(a) * r;
// 	double dx = -sin(a) * d;
// 	double dy = cos(a) * d;
// 	if(path->points.size == 0)
// 		cg_path_move_to(path, ax, ay);
// 	else
// 		cg_path_line_to(path, ax, ay);
// 	for(int i = 0; i < seg_n; i++)
// 	{
// 		double cp1x = ax + dx;
// 		double cp1y = ay + dy;
// 		a += seg_a;
// 		ax = cx + cos(a) * r;
// 		ay = cy + sin(a) * r;
// 		dx = -sin(a) * d;
// 		dy = cos(a) * d;
// 		double cp2x = ax - dx;
// 		double cp2y = ay - dy;
// 		cg_path_curve_to(path, cp1x, cp1y, cp2x, cp2y, ax, ay);
// 	}
// }

// static inline void cg_path_clear(struct cg_path_t * path)
// {
// 	path->elements.size = 0;
// 	path->points.size = 0;
// 	path->contours = 0;
// 	path->start.x = 0.0;
// 	path->start.y = 0.0;
// }