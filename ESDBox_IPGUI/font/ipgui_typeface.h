#ifndef IPGUI_TYPEFACE_H
#define IPGUI_TYPEFACE_H

// 区分下面三个概念
// 字体（英语：typeface）：一整套的字形，一个或多个字型的一个或多个尺寸的集合。
// 字型（英语：font）：一套具有同样风格和尺寸的字形。
// 字形（英语：glyph）：单个字的形体。


#include "ipgui_utils.h"
#include "ipgui_vfs.h"

IPGUI_HEADER_BEGIN _______________MARKER_______________

typedef void * (* pTakeFontsIndx)(void * pvData);

typedef enum
{
    IPGUI_FONT_TYPE_BITMAP,
#define IPGUI_FONT_TYPE_BITMAP 0
    IPGUI_FONT_TYPE_VECTOR,
#define IPGUI_FONT_TYPE_VECTOR 1
}ipgui_font_type_e;

typedef struct
{
    char where;
    union{
        struct {
            ipgui_fs_t * fs;
            const char * path;
        } font_in_disk;
        struct {
            void * data;
        } font_in_ram;
    };
}ipgui_font_tar_t;

typedef struct{
#define IPGUI_CHARSET_ENCODE
    ipgui_font_type_e type;
    ipgui_font_tar_t tar;
}ipgui_font_t;

typedef unsigned char ipgui_font_size_t;

typedef struct{
    ipgui_font_t font;
    ipgui_font_size_t size;
}ipgui_typeface_t;

IPGUI_HEADER_END   _______________MARKER_______________

#endif