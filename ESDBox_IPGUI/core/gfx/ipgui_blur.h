/*
 * ipgui_blur.h - Alpha box blur for GUI rendering
 *
 * 3-pass box blur approximates Gaussian blur (central limit theorem).
 * Sliding window approach, no integral image.
 */

#ifndef IPGUI_BLUR_H
#define IPGUI_BLUR_H

#include "ipgui_types.h"
#include "ipgui_coord.h"
#include "ipgui_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A8 surface: 8-bit alpha-only, row-major, stride == width */
typedef struct {
    u8_t           * mask;
    ipgui_coord_t    w;
    ipgui_coord_t    h;
} ipgui_mask_surface_t;

extern __IPGUI_API__ void ipgui_average_blur_hor(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          kn_size, /* average blur kernel size */
    ipgui_mask_surface_t * out);

extern __IPGUI_API__ void ipgui_average_blur_ver(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          kn_size, /* average blur kernel size */
    ipgui_mask_surface_t * out);

extern __IPGUI_API__ void ipgui_blur(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          h_kn_size, /* horizontal blur kernel size */
    ipgui_coord_t          v_kn_size, /* vertical blur kernel size */
    ipgui_mask_surface_t * out);

#ifdef __cplusplus
}
#endif

#endif /* IPGUI_BLUR_H */
