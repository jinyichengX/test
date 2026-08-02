/*
 * IPGUI Box Shadow Rendering
 *
 * Algorithm: Separable 3-pass box blur ≈ Gaussian convolution
 *
 *   Step 1: Compute shadow geometry (core_rect + blur expansion)
 *   Step 2: Generate rounded-rectangle mask (4 radii, integer only)
 *   Step 3: 3-pass box blur (horizontal → vertical → horizontal)
 *   Step 4: Blend mask × shadow_color → surface
 *
 * Memory: shadow_w × shadow_h bytes for mask buffer.
 *   For typical MCU displays (320×240), worst-case ~20 KB.
 *   If alloc fails, function returns silently.
 *
 * Efficiency: Box blur uses sliding window (O(1) per pixel, integer only).
 *   3-pass ≈ Gaussian with σ ≈ blur_w / 2.
 */

#include "ipgui_draw_box_shadow.h"
#include "ipgui_utils.h"
#include "ipgui_color.h"
#include "ipgui_blend.h"
#include <stdlib.h>
#include <string.h>

/*-----------------------------------------------------------------------------
 * 1D box blur — sliding window, O(1) per pixel, integer only
 *   buf[0..len-1]  →  tmp[0..len-1]  →  copy back
 *   buf and tmp MUST be distinct (no aliasing).
 *   Edge pixels are extended (clamped), not zero-padded.
 *-----------------------------------------------------------------------------*/
__IPGUI_STATIC__ void box_blur_1d_h(u8_t *buf, int len, int window, u8_t *tmp)
{
    int half  = window >> 1;
    int half2 = window - half - 1;
    u32_t sum;
    int x, i;

    /* Prime sum at position 0: window covers [-half, half2] */
    sum = 0;
    for (i = -half; i <= half2; i++) {
        int idx;
        if (i < 0)       idx = 0;
        else if (i >= len) idx = len - 1;
        else             idx = i;
        sum += buf[idx];
    }

    for (x = 0; x < len; x++) {
        tmp[x] = (u8_t)((sum + (u32_t)(window >> 1)) / (u32_t)window);

        /* remove leftmost pixel of current window */
        {
            int left = x - half;
            if (left < 0) sum -= buf[0];
            else          sum -= buf[left];
        }
        /* add rightmost pixel of next window */
        {
            int right = x + half2 + 1;
            if (right >= len) sum += buf[len - 1];
            else              sum += buf[right];
        }
    }
    memcpy(buf, tmp, (size_t)len);
}

/*-----------------------------------------------------------------------------
 * Fill rounded-rectangle mask
 *   mask[mask_w × mask_h]  covers shadow_area
 *   core_rect is positioned inside shadow_area:
 *       core_left   = shadow_start.x + blur_pad
 *       core_top    = shadow_start.y + blur_pad
 *       core_right  = shadow_end.x   - blur_pad
 *       core_bottom = shadow_end.y   - blur_pad
 *
 *   Each pixel: 255 (inside shape) or 0 (outside)
 *   Supports 4 different corner radii; any radius = 0 means square corner.
 *-----------------------------------------------------------------------------*/
__IPGUI_STATIC__ void fill_rounded_mask(
    u8_t  *mask,
    int    mw,           /* mask width  = shadow_area width  */
    int    mh,           /* mask height = shadow_area height */
    int    core_l,       /* core_rect left, in mask-local coords */
    int    core_t,
    int    core_r,       /* core_rect right  (inclusive) */
    int    core_b,       /* core_rect bottom (inclusive) */
    int    r_tl, int r_tr, int r_bl, int r_br)
{
    int x, y;

    for (y = 0; y < mh; y++) {
        for (x = 0; x < mw; x++) {
            int inside = 0;

            /* fast bounding-box reject */
            if (x < core_l || x > core_r || y < core_t || y > core_b) {
                mask[y * mw + x] = 0;
                continue;
            }

            inside = 1;

            /* Top-left corner */
            if (r_tl > 0 && x < core_l + r_tl && y < core_t + r_tl) {
                int dx = (core_l + r_tl) - x - 1;
                int dy = (core_t + r_tl) - y - 1;
                if (dx * dx + dy * dy > r_tl * r_tl) inside = 0;
            }

            /* Top-right corner */
            if (inside && r_tr > 0 && x > core_r - r_tr && y < core_t + r_tr) {
                int dx = x - (core_r - r_tr);
                int dy = (core_t + r_tr) - y - 1;
                if (dx * dx + dy * dy > r_tr * r_tr) inside = 0;
            }

            /* Bottom-left corner */
            if (inside && r_bl > 0 && x < core_l + r_bl && y > core_b - r_bl) {
                int dx = (core_l + r_bl) - x - 1;
                int dy = y - (core_b - r_bl);
                if (dx * dx + dy * dy > r_bl * r_bl) inside = 0;
            }

            /* Bottom-right corner */
            if (inside && r_br > 0 && x > core_r - r_br && y > core_b - r_br) {
                int dx = x - (core_r - r_br);
                int dy = y - (core_b - r_br);
                if (dx * dx + dy * dy > r_br * r_br) inside = 0;
            }

            mask[y * mw + x] = inside ? (u8_t)255 : (u8_t)0;
        }
    }
}

/*-----------------------------------------------------------------------------
 * Main entry
 *-----------------------------------------------------------------------------*/
__IPGUI_API__ void ipgui_draw_box_shadow(
    ipgui_surf_t              *surf,
    ipgui_aabb_t              *clip,
    ipgui_aabb_t              *box,
    ipgui_box_style_t         *style,
    ipgui_box_shadow_style_t  *shadow_style)
{
    ipgui_aabb_t core, shadow, render;
    ipgui_coord_t blur_w, blur_pad;
    ipgui_coord_t core_l, core_t, core_r, core_b;
    ipgui_coord_t mw, mh;
    ipgui_coord_t r_tl, r_tr, r_bl, r_br;
    int window;
    u8_t *mask, *tmp;
    size_t mask_size;

    /* (1) Sanity */
    if (surf == NULL || box == NULL || shadow_style == NULL) return;

    blur_w = shadow_style->blur;
    if (blur_w < 0) blur_w = 0;

    if (blur_w == 0 && shadow_style->spread == 0 && shadow_style->offset_x == 0
        && shadow_style->offset_y == 0) {
        /* no blur, no spread, no offset → invisible shadow */
        return;
    }

    /* (2) Compute core_rect = box + offset + spread */
    core.start.x = box->start.x + shadow_style->offset_x - shadow_style->spread;
    core.start.y = box->start.y + shadow_style->offset_y - shadow_style->spread;
    core.end.x   = box->end.x   + shadow_style->offset_x + shadow_style->spread;
    core.end.y   = box->end.y   + shadow_style->offset_y + shadow_style->spread;

    /* (3) Compute shadow_area = core ± (blur/2 + 1) */
    blur_pad     = (blur_w / 2) + 1;
    shadow.start.x = core.start.x - blur_pad;
    shadow.start.y = core.start.y - blur_pad;
    shadow.end.x   = core.end.x   + blur_pad;
    shadow.end.y   = core.end.y   + blur_pad;

    /* (4) Clip shadow_area with clip and surf bounds */
    {
        ipgui_aabb_t surf_clip;
        surf_clip.start.x = surf->surf.start.x;
        surf_clip.start.y = surf->surf.start.y;
        surf_clip.end.x   = surf->surf.end.x;
        surf_clip.end.y   = surf->surf.end.y;

        if (clip != NULL) {
            if (!ipgui_aabb_intersect(&render, &shadow, clip)) return;
            {
                ipgui_aabb_t tmp_render;
                if (!ipgui_aabb_intersect(&tmp_render, &render, &surf_clip)) return;
                render = tmp_render;
            }
        } else {
            if (!ipgui_aabb_intersect(&render, &shadow, &surf_clip)) return;
        }
    }

    mw = ipgui_aabb_width(&render);
    mh = ipgui_aabb_height(&render);
    if (mw <= 0 || mh <= 0) return;

    /* (5) Core rect in local coords (render.start = origin) */
    core_l = core.start.x - render.start.x;
    core_t = core.start.y - render.start.y;
    core_r = core.end.x   - render.start.x;
    core_b = core.end.y   - render.start.y;

    /* clamp core to render area (for extreme offset) */
    if (core_l < 0) core_l = 0;
    if (core_t < 0) core_t = 0;
    if (core_r >= mw) core_r = mw - 1;
    if (core_b >= mh) core_b = mh - 1;

    /* (6) Corner radii */
    r_tl = r_tr = r_bl = r_br = 0;
    if (style != NULL) {
        r_tl = style->left_top_radius;
        r_tr = style->right_top_radius;
        r_bl = style->left_bottom_radius;
        r_br = style->right_bottom_radius;
    }
    /* clamp radii to core size */
    {
        int cw = core_r - core_l + 1;
        int ch = core_b - core_t + 1;
        if (2 * r_tl > cw) r_tl = cw / 2;
        if (2 * r_tr > cw) r_tr = cw / 2;
        if (2 * r_bl > cw) r_bl = cw / 2;
        if (2 * r_br > cw) r_br = cw / 2;
        if (2 * r_tl > ch) r_tl = ch / 2;
        if (2 * r_tr > ch) r_tr = ch / 2;
        if (2 * r_bl > ch) r_bl = ch / 2;
        if (2 * r_br > ch) r_br = ch / 2;
    }

    /* (7) Allocate mask + temp buffer.
     * tmp needs 2×max(w,h): one half for column extraction/first-pass row,
     * the other half as blur output (box_blur_1d requires buf≠tmp). */
    mask_size = (size_t)mw * (size_t)mh;

    mask = (u8_t *)malloc(mask_size);
    tmp  = (u8_t *)malloc((size_t)(mw > mh ? mw : mh) * 2u);
    if (mask == NULL || tmp == NULL) {
        if (mask) free(mask);
        if (tmp)  free(tmp);
        return; /* silent fail */
    }

    /* (8) Fill rounded-rect angle mask */
    fill_rounded_mask(mask, (int)mw, (int)mh,
                      (int)core_l, (int)core_t, (int)core_r, (int)core_b,
                      (int)r_tl, (int)r_tr, (int)r_bl, (int)r_br);

    /* (9) 3-pass box blur ≈ Gaussian                                              */
    if (blur_w > 0) {
        /* 3 passes with window = blur_w / 2 */
        /* (min window size is 3 to avoid degeneracy) */
        window = blur_w / 2;
        if (window < 3) window = blur_w;
        if (window < 3) window = 3;

        /* Pass 1: horizontal */
        {
            int y;
            for (y = 0; y < mh; y++) {
                box_blur_1d_h(mask + y * mw, (int)mw, window, tmp);
            }
        }

        /* Pass 2: vertical — extract column into tmp[0..mh-1],
         * blur into tmp[N..N+mh-1], write back */
        {
            int x, y;
            int N = (mw > mh) ? mw : mh; /* half of tmp */
            u8_t *col = tmp;
            u8_t *out = tmp + N;
            for (x = 0; x < mw; x++) {
                for (y = 0; y < mh; y++) {
                    col[y] = mask[y * mw + x];
                }
                box_blur_1d_h(col, (int)mh, window, out);
                for (y = 0; y < mh; y++) {
                    mask[y * mw + x] = out[y];
                }
            }
        }

        /* Pass 3: horizontal */
        {
            int y;
            for (y = 0; y < mh; y++) {
                box_blur_1d_h(mask + y * mw, (int)mw, window, tmp);
            }
        }
    }

    /* (10) Blend */
    {
        /* Pre-multiply opacity into color for the blend engine */
        ipgui_color_t col = ipgui_color_combine_opacity_and_premultiply(
            &shadow_style->color, shadow_style->opacity);
        ipgui_aabb_t mask_aabb = render;

        ipgui_blend_color(surf, clip, &render,
                          col, (u8_t)255, /* opacity already in color */
                          mask, &mask_aabb,
                          IPGUI_BLEND_NORMAL);
    }

    /* (11) Cleanup */
    free(tmp);
    free(mask);
}

/*-----------------------------------------------------------------------------
 * Test entry: pure rectangle shadow (no corners, no spread, no offset)
 * Two-pass separable box blur with malloc/free, for desktop validation only.
 *-----------------------------------------------------------------------------*/
__IPGUI_API__ void ipgui_draw_box_shadow_test(
    ipgui_surf_t  *surf,
    ipgui_coord_t  box_x, ipgui_coord_t box_y,
    ipgui_coord_t  box_w, ipgui_coord_t box_h,
    ipgui_coord_t  blur_w,
    ipgui_color_t  shadow_color, u8_t opacity)
{
    ipgui_aabb_t          box;
    ipgui_box_shadow_style_t ss;
    ipgui_box_style_t     s;

    box.start.x   = box_x;
    box.start.y   = box_y;
    box.end.x     = box_x + box_w - 1;
    box.end.y     = box_y + box_h - 1;

    ss.color    = shadow_color;
    ss.opacity  = opacity;
    ss.blur     = blur_w;
    ss.spread   = 0;
    ss.offset_x = 0;
    ss.offset_y = 0;

    s.left_top_radius     = 0;
    s.right_top_radius    = 0;
    s.left_bottom_radius  = 0;
    s.right_bottom_radius = 0;

    ipgui_draw_box_shadow(surf, NULL, &box, &s, &ss);
}
