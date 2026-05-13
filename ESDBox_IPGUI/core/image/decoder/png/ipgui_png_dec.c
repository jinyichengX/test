#include "ipgui_png_dec.h"
#include "ipgui_vfs.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

/* PNG签名 8字节 */
static const unsigned char png_sig[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};

/* 大端序读取 */
static inline unsigned int be32(const unsigned char *s)
{
    return ((unsigned int)s[0] << 24) | ((unsigned int)s[1] << 16) |
           ((unsigned int)s[2] << 8)  | (unsigned int)s[3];
}

static inline unsigned short be16(const unsigned char *s)
{
    return ((unsigned short)s[0] << 8) | (unsigned short)s[1];
}

/* chunk类型比较 */
static inline int chunk_is(const unsigned char *type, const char *name)
{
    return type[0] == name[0] && type[1] == name[1] &&
           type[2] == name[2] && type[3] == name[3];
}

/* 计算每像素位数和通道数 */
static void calc_pixel_info(struct png_inf *inf)
{
    switch (inf->color_type) {
        case PNG_COLOR_GRAY:       inf->channels = 1; break;
        case PNG_COLOR_RGB:        inf->channels = 3; break;
        case PNG_COLOR_PALETTE:    inf->channels = 1; break;
        case PNG_COLOR_GRAY_ALPHA: inf->channels = 2; break;
        case PNG_COLOR_RGBA:       inf->channels = 4; break;
        default:                   inf->channels = 1; break;
    }
    /* 每像素字节数(用于滤波器计算) */
    int bits = inf->channels * inf->bit_depth;
    inf->bpp = (bits + 7) / 8;
}

/* 解析IHDR */
static ipgui_err_t parse_ihdr(const unsigned char *data, struct png_inf *inf)
{
    inf->w = be32(data);
    inf->h = be32(data + 4);
    inf->bit_depth = data[8];
    inf->color_type = data[9];
    
    if (data[10] != 0) return IPGUI_ERR_PNG_COMPRESS;  /* 仅支持deflate */
    if (data[11] != 0) return IPGUI_ERR_PNG_FILTER;    /* 仅支持标准滤波 */
    
    inf->interlace = data[12];
    if (inf->interlace != 0) {
        ipgui_dbg_warning("Adam7 interlace not fully supported\r\n");
    }
    
    /* 验证位深度与颜色类型组合 */
    switch (inf->color_type) {
        case PNG_COLOR_GRAY:
            if (inf->bit_depth != 1 && inf->bit_depth != 2 &&
                inf->bit_depth != 4 && inf->bit_depth != 8 &&
                inf->bit_depth != 16)
                return IPGUI_ERR_PNG_DEPTH;
            break;
        case PNG_COLOR_RGB:
        case PNG_COLOR_GRAY_ALPHA:
        case PNG_COLOR_RGBA:
            if (inf->bit_depth != 8 && inf->bit_depth != 16)
                return IPGUI_ERR_PNG_DEPTH;
            break;
        case PNG_COLOR_PALETTE:
            if (inf->bit_depth != 1 && inf->bit_depth != 2 &&
                inf->bit_depth != 4 && inf->bit_depth != 8)
                return IPGUI_ERR_PNG_DEPTH;
            break;
        default:
            return IPGUI_ERR_PNG_COLOR;
    }
    
    calc_pixel_info(inf);
    return IPGUI_ERR_OK;
}

/* 解析PNG头部和chunks信息 */
ipgui_err_t ipgui_png_dec(const char *path, struct png_inf *inf)
{
    ipgui_err_t ret = IPGUI_ERR_OK;
    ipgui_file_t file;
    unsigned char buf[32];
    int br;
    
    /* 初始化inf */
    inf->path = path;
    inf->palette = NULL;
    inf->trns = NULL;
    inf->plte_entries = 0;
    inf->trns_entries = 0;
    inf->idat_offset = 0;
    inf->idat_total_sz = 0;
    
    ipgui_link_fs_auto(&file);
    ret = ipgui_vfs_fopen(&file, path, IPGUI_FILE_MODE_READ, 0, 0);
    if (ret != IPGUI_ERR_OK) return ret;
    
    /* 验证PNG签名 */
    ret = ipgui_vfs_fread(&file, buf, 8, &br);
    if (ret != IPGUI_ERR_OK || br != 8) goto _return;
    
    for (int i = 0; i < 8; i++) {
        if (buf[i] != png_sig[i]) {
            ret = IPGUI_ERR_PNG_SIG;
            goto _return;
        }
    }
    
    int ihdr_found = 0, iend_found = 0;
    unsigned int file_pos = 8;
    
    /* 解析chunks */
    while (!iend_found) {
        /* 读取chunk头: length(4) + type(4) */
        ret = ipgui_vfs_fread(&file, buf, 8, &br);
        if (ret != IPGUI_ERR_OK || br != 8) {
            ret = IPGUI_ERR_PNG_CHUNK;
            goto _return;
        }
        
        unsigned int chunk_len = be32(buf);
        unsigned char *chunk_type = buf + 4;
        file_pos += 8;
        
        if (chunk_is(chunk_type, "IHDR")) {
            if (chunk_len != 13) {
                ret = IPGUI_ERR_PNG_IHDR;
                goto _return;
            }
            unsigned char ihdr_data[13];
            ret = ipgui_vfs_fread(&file, ihdr_data, 13, &br);
            if (ret != IPGUI_ERR_OK || br != 13) goto _return;
            
            ret = parse_ihdr(ihdr_data, inf);
            if (ret != IPGUI_ERR_OK) goto _return;
            ihdr_found = 1;
            file_pos += 13;
            
            /* 跳过CRC */
            ipgui_vfs_fseek(&file, IPGUI_FILE_SEEK_CUR, 4);
            file_pos += 4;
            
        } else if (chunk_is(chunk_type, "PLTE")) {
            if (chunk_len % 3 != 0 || chunk_len > 768) {
                ret = IPGUI_ERR_PNG_PLTE;
                goto _return;
            }
            inf->plte_entries = chunk_len / 3;
            inf->palette = ipgui_mem_alloc_def(chunk_len);
            if (!inf->palette) {
                ret = IPGUI_ERR_MEM;
                goto _return;
            }
            ret = ipgui_vfs_fread(&file, inf->palette, chunk_len, &br);
            if (ret != IPGUI_ERR_OK || br != (int)chunk_len) goto _return;
            file_pos += chunk_len;
            
            ipgui_vfs_fseek(&file, IPGUI_FILE_SEEK_CUR, 4);
            file_pos += 4;
            
        } else if (chunk_is(chunk_type, "tRNS")) {
            inf->trns = ipgui_mem_alloc_def(chunk_len);
            if (!inf->trns) {
                ret = IPGUI_ERR_MEM;
                goto _return;
            }
            inf->trns_entries = chunk_len;
            ret = ipgui_vfs_fread(&file, inf->trns, chunk_len, &br);
            if (ret != IPGUI_ERR_OK || br != (int)chunk_len) goto _return;
            file_pos += chunk_len;
            
            ipgui_vfs_fseek(&file, IPGUI_FILE_SEEK_CUR, 4);
            file_pos += 4;
            
        } else if (chunk_is(chunk_type, "IDAT")) {
            if (inf->idat_offset == 0) {
                inf->idat_offset = file_pos;
            }
            inf->idat_total_sz += chunk_len;
            
            /* 跳过IDAT数据和CRC */
            ipgui_vfs_fseek(&file, IPGUI_FILE_SEEK_CUR, chunk_len + 4);
            file_pos += chunk_len + 4;
            
        } else if (chunk_is(chunk_type, "IEND")) {
            iend_found = 1;
        } else {
            /* 跳过未知chunk */
            ipgui_vfs_fseek(&file, IPGUI_FILE_SEEK_CUR, chunk_len + 4);
            file_pos += chunk_len + 4;
        }
    }
    
    if (!ihdr_found) {
        ret = IPGUI_ERR_PNG_IHDR;
    }
    
_return:
    ipgui_vfs_fclose(&file);
    if (ret != IPGUI_ERR_OK) {
        ipgui_png_free(inf);
    }
    return ret;
}

void ipgui_png_free(struct png_inf *inf)
{
    if (inf->palette) {
        ipgui_mem_free_def(inf->palette);
        inf->palette = NULL;
    }
    if (inf->trns) {
        ipgui_mem_free_def(inf->trns);
        inf->trns = NULL;
    }
}

void test_png(void)
{
    static unsigned char * buffer[1024*1024];
    struct png_inf inf;
    ipgui_png_dec("image.png", &inf);
    ipgui_png_read_linebyline(&inf, 0, 0, buffer, inf.w * inf.h, NULL);
    ipgui_png_ctx_free();  /* 释放解码缓存 */
    ipgui_png_free(&inf);  /* 释放调色板等 */
}