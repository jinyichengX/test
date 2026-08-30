#ifndef IPGUI_BMP_DEC_H
#define IPGUI_BMP_DEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_utils.h"
#include "ipgui_coord.h"

struct bmp_inf;

__IPGUI_API__ ipgui_err_t ipgui_bmp_dec(const s8_t * path, struct bmp_inf * inf);
__IPGUI_API__ ipgui_err_t ipgui_bmp_read_linebyline(struct bmp_inf * inf, 
    ipgui_coord_t img_x, ipgui_coord_t img_y,
    void * buffer, ipgui_coord_t pix_num, ipgui_coord_t * pix_nr);
__IPGUI_API__ void ipgui_bmp_close(struct bmp_inf * inf);

#ifdef __cplusplus
}
#endif

#endif
