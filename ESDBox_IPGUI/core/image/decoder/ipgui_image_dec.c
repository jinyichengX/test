#include "ipgui_image_dec.h"
#include "ipgui_memory.h"

static inline int str_cmp_simple(const char * s1, const char * s2, int len)
{
    for (int idx = 0; idx < len; idx++) {
        if (s1[idx] != s2[idx])
            return 1;
    }
    return 0;
}

ipgui_image_file_type_t ipgui_image_fmt_get(const char * src_path)
{
    static const char * string[5] = {
        "bmp", "jpg", "png", "gif", "jpeg"
    };
    char * suffix;
    int len = ipgui_strlen(src_path);

    if (len < 5) /* at least 4,example: 1.jpg */
        return IPGUI_IMG_FILE_TYPE_UNKNOWN;
    
    suffix = (char *)src_path + len - 1; /* get the last char */
    for (int idx = 0; idx < len; idx++) {
        if (*suffix == '.')
            break;
        suffix --;
    }
    if (* suffix != '.')
        return IPGUI_IMG_FILE_TYPE_UNKNOWN;
    
    suffix ++; /* point to the first char after '.' */
    len = len - (suffix - src_path); /* get the suffix length after '.' */
    for (int idx = 0; idx < 5; idx++) {
        if (ipgui_strlen(string[idx]) != len)
            continue;
        if (0 == str_cmp_simple(suffix, string[idx], len)) {
            return (ipgui_image_file_type_t)idx;
        }
    }
    return IPGUI_IMG_FILE_TYPE_CUSTOM;
}

// ipgui_err_t ipgui_image_create(ipgui_img_t ** img, ipgui_image_file_type_t file_type)
// {
    
// }