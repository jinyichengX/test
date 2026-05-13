#ifndef IPGUI_POINTER_H
#define IPGUI_POINTER_H

#include "ipgui_coord.h"

typedef enum {
    IPGUI_PDIS_RELEASED = 0,
    IPGUI_PDIS_PRESSED,
}ipgui_pdis_t; /* pointer device input state */

typedef struct {
    ipgui_coord_t x;
    ipgui_coord_t y;
    ipgui_pdis_t  state;
    // int channel; /* 多点触摸通道，例如0表示主触点 */
}ipgui_pdid_t;               /* pointer device input data */

typedef struct {
    void * priv_data;                  /* private data    */
    int (* read)(ipgui_pdid_t * pdid); /* read input data */
}ipgui_pointer_drv_t;

#endif