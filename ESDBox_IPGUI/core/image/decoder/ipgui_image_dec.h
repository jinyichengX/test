#ifndef IPGUI_IMAGE_DEC_H
#define IPGUI_IMAGE_DEC_H

#include "ipgui_utils.h"
#include "ipgui_coord.h"
#include "ipgui_transform.h"
#include "ipgui_vfs.h"
#include "ipgui_widget.h"

IPGUI_HEADER_BEGIN

typedef enum{
    IPGUI_IMG_FILE_TYPE_BMP,      /* bmp */
    IPGUI_IMG_FILE_TYPE_JPG,      /* jpg */
    IPGUI_IMG_FILE_TYPE_PNG,      /* png */
    IPGUI_IMG_FILE_TYPE_GIF,      /* gif */
    IPGUI_IMG_FILE_TYPE_JPEG,      /* gif */
    IPGUI_IMG_FILE_TYPE_CUSTOM,   /* 自定义格式 */
    IPGUI_IMG_FILE_TYPE_UNKNOWN 
}ipgui_image_file_type_t;

typedef struct ipgui_img_dec_ctx ipgui_img_dec_t;
typedef struct ipgui_image_operations_ctx
{
    int (* loader)(const char * path);
    // int (* get_info)(const char * path, ipgui_image_info_t * info);
    // int (* decoder)(ipgui_img_buf_t * buf, ipgui_coord_t x1, ipgui_coord_t y1, ipgui_coord_t x2, ipgui_coord_t y2);
}ipgui_img_file_ops_t;

typedef struct {
    ipgui_coord_t width;
    ipgui_coord_t height;
    unsigned char * buf;
    char bpp : 6;
    char premult_alpha : 1;
    char has_alpha : 1;
}ipgui_image_info_t;

typedef struct ipgui_img_dec_ctx{
    void * priv;
    ipgui_image_file_type_t file_type;
    ipgui_img_file_ops_t * file_ops;
}ipgui_img_dec_t;

extern ipgui_image_file_type_t ipgui_image_fmt_get(const char * src_path);

IPGUI_HEADER_END

#endif