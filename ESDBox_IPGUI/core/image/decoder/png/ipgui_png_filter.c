/*
 * PNG 滤波器反解和颜色转换
 */

#include "ipgui_png_dec.h"
#include "ipgui_vfs.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

/* 外部inflate函数 */
extern int ipgui_inflate(const unsigned char *in, unsigned int in_size,
                         unsigned char *out, unsigned int out_size,
                         unsigned int *out_len);

/* Paeth预测器 */
static inline unsigned char paeth(unsigned char a, unsigned char b, unsigned char c)
{
    int p = a + b - c;
    int pa = p > a ? p - a : a - p;
    int pb = p > b ? p - b : b - p;
    int pc = p > c ? p - c : c - p;
    
    if (pa <= pb && pa <= pc) return a;
    if (pb <= pc) return b;
    return c;
}

/* 反滤波一行 */
static void unfilter_row(unsigned char *row, const unsigned char *prev,
                         int filter, int bpp, int rowbytes)
{
    int i;
    
    switch (filter) {
        case PNG_FILTER_NONE:
            break;
            
        case PNG_FILTER_SUB:
            for (i = bpp; i < rowbytes; i++) {
                row[i] += row[i - bpp];
            }
            break;
            
        case PNG_FILTER_UP:
            if (prev) {
                for (i = 0; i < rowbytes; i++) {
                    row[i] += prev[i];
                }
            }
            break;
            
        case PNG_FILTER_AVG:
            if (prev) {
                for (i = 0; i < bpp; i++) {
                    row[i] += prev[i] / 2;
                }
                for (i = bpp; i < rowbytes; i++) {
                    row[i] += (row[i - bpp] + prev[i]) / 2;
                }
            } else {
                for (i = bpp; i < rowbytes; i++) {
                    row[i] += row[i - bpp] / 2;
                }
            }
            break;
            
        case PNG_FILTER_PAETH:
            if (prev) {
                for (i = 0; i < bpp; i++) {
                    row[i] += paeth(0, prev[i], 0);
                }
                for (i = bpp; i < rowbytes; i++) {
                    row[i] += paeth(row[i - bpp], prev[i], prev[i - bpp]);
                }
            } else {
                for (i = bpp; i < rowbytes; i++) {
                    row[i] += paeth(row[i - bpp], 0, 0);
                }
            }
            break;
    }
}

/* 从行数据提取像素并转换为BGRA32 */
static void row_to_bgra(struct png_inf *inf, const unsigned char *row,
                        unsigned int *out, int start_x, int count)
{
    int x, idx;
    unsigned char r, g, b, a;
    
    for (x = 0; x < count; x++) {
        int px = start_x + x;
        a = 255;
        
        switch (inf->color_type) {
            case PNG_COLOR_GRAY:
                if (inf->bit_depth == 8) {
                    r = g = b = row[px];
                } else if (inf->bit_depth == 16) {
                    r = g = b = row[px * 2];
                } else {
                    /* 1/2/4位 */
                    int bits = inf->bit_depth;
                    int ppb = 8 / bits;
                    int byte_idx = px / ppb;
                    int bit_idx = (ppb - 1 - (px % ppb)) * bits;
                    unsigned char val = (row[byte_idx] >> bit_idx) & ((1 << bits) - 1);
                    /* 扩展到8位 */
                    val = val * 255 / ((1 << bits) - 1);
                    r = g = b = val;
                }
                /* 检查tRNS透明色 */
                if (inf->trns && inf->trns_entries >= 2) {
                    unsigned short gray_key = (inf->trns[0] << 8) | inf->trns[1];
                    if (r == (gray_key & 0xFF)) a = 0;
                }
                break;
                
            case PNG_COLOR_RGB:
                if (inf->bit_depth == 8) {
                    idx = px * 3;
                    r = row[idx];
                    g = row[idx + 1];
                    b = row[idx + 2];
                } else {
                    idx = px * 6;
                    r = row[idx];
                    g = row[idx + 2];
                    b = row[idx + 4];
                }
                /* 检查tRNS透明色 */
                if (inf->trns && inf->trns_entries >= 6) {
                    if (r == inf->trns[1] && g == inf->trns[3] && b == inf->trns[5]) {
                        a = 0;
                    }
                }
                break;
                
            case PNG_COLOR_PALETTE:
                {
                    int bits = inf->bit_depth;
                    int ppb = 8 / bits;
                    int byte_idx = px / ppb;
                    int bit_idx = (ppb - 1 - (px % ppb)) * bits;
                    unsigned char pal_idx = (row[byte_idx] >> bit_idx) & ((1 << bits) - 1);
                    
                    if (inf->palette && pal_idx < inf->plte_entries) {
                        r = inf->palette[pal_idx * 3];
                        g = inf->palette[pal_idx * 3 + 1];
                        b = inf->palette[pal_idx * 3 + 2];
                    } else {
                        r = g = b = 0;
                    }
                    if (inf->trns && pal_idx < inf->trns_entries) {
                        a = inf->trns[pal_idx];
                    }
                }
                break;
                
            case PNG_COLOR_GRAY_ALPHA:
                if (inf->bit_depth == 8) {
                    idx = px * 2;
                    r = g = b = row[idx];
                    a = row[idx + 1];
                } else {
                    idx = px * 4;
                    r = g = b = row[idx];
                    a = row[idx + 2];
                }
                break;
                
            case PNG_COLOR_RGBA:
                if (inf->bit_depth == 8) {
                    idx = px * 4;
                    r = row[idx];
                    g = row[idx + 1];
                    b = row[idx + 2];
                    a = row[idx + 3];
                } else {
                    idx = px * 8;
                    r = row[idx];
                    g = row[idx + 2];
                    b = row[idx + 4];
                    a = row[idx + 6];
                }
                break;
                
            default:
                r = g = b = a = 0;
        }
        
        /* BGRA32格式输出 */
        out[x] = b | (g << 8) | (r << 16) | (a << 24);
    }
}

/* 收集所有IDAT数据 */
static unsigned char *collect_idat(struct png_inf *inf, ipgui_file_t *f, 
                                   unsigned int *total_size)
{
    unsigned char *idat_data = ipgui_mem_alloc_def(inf->idat_total_sz);
    if (!idat_data) return NULL;
    
    unsigned int offset = inf->idat_offset;
    unsigned int collected = 0;
    unsigned char chunk_hdr[8];
    int br;
    
    ipgui_vfs_fseek(f, IPGUI_FILE_SEEK_SET, offset - 8);
    
    while (collected < inf->idat_total_sz) {
        ipgui_vfs_fread(f, chunk_hdr, 8, &br);
        unsigned int len = (chunk_hdr[0] << 24) | (chunk_hdr[1] << 16) |
                          (chunk_hdr[2] << 8) | chunk_hdr[3];
        
        if (chunk_hdr[4] == 'I' && chunk_hdr[5] == 'D' &&
            chunk_hdr[6] == 'A' && chunk_hdr[7] == 'T') {
            ipgui_vfs_fread(f, idat_data + collected, len, &br);
            collected += len;
            ipgui_vfs_fseek(f, IPGUI_FILE_SEEK_CUR, 4); /* skip CRC */
        } else {
            ipgui_vfs_fseek(f, IPGUI_FILE_SEEK_CUR, len + 4);
        }
    }
    
    *total_size = collected;
    return idat_data;
}

/* PNG解码上下文，用于缓存解压数据 */
typedef struct {
    unsigned char *raw_data;     /* 解压后的原始数据 */
    unsigned int raw_size;
    unsigned char *prev_row;     /* 上一行(反滤波后) */
    unsigned char *curr_row;     /* 当前行(反滤波后) */
    int row_bytes;               /* 每行像素字节数(不含filter) */
    int current_y;               /* 当前已解码到的行 */
    int decoded;                 /* 是否已解压 */
} png_decode_ctx_t;

static png_decode_ctx_t g_png_ctx = {0};

/* 完整解码PNG */
ipgui_err_t ipgui_png_read_linebyline(struct png_inf *inf,
    ipgui_coord_t img_x, ipgui_coord_t img_y,
    void *buffer, ipgui_coord_t pix_num,
    ipgui_coord_t *pixs_nr)
{
    ipgui_err_t ret = IPGUI_ERR_OK;
    ipgui_file_t file;
    unsigned int *out = (unsigned int *)buffer;
    
    if (img_x < 0 || img_y < 0 || img_x >= inf->w || img_y >= inf->h) {
        return IPGUI_ERR_PNG_RANGE;
    }
    
    /* 计算每行字节数 */
    int bits_per_pixel;
    switch (inf->color_type) {
        case PNG_COLOR_GRAY:       bits_per_pixel = inf->bit_depth; break;
        case PNG_COLOR_RGB:        bits_per_pixel = inf->bit_depth * 3; break;
        case PNG_COLOR_PALETTE:    bits_per_pixel = inf->bit_depth; break;
        case PNG_COLOR_GRAY_ALPHA: bits_per_pixel = inf->bit_depth * 2; break;
        case PNG_COLOR_RGBA:       bits_per_pixel = inf->bit_depth * 4; break;
        default: return IPGUI_ERR_PNG_COLOR;
    }
    int row_bytes = (inf->w * bits_per_pixel + 7) / 8;
    
    /* 首次调用时解压整个图像 */
    if (!g_png_ctx.decoded) {
        ipgui_link_fs_auto(&file);
        ret = ipgui_vfs_fopen(&file, inf->path, IPGUI_FILE_MODE_READ, 0, 0);
        if (ret != IPGUI_ERR_OK) return ret;
        
        /* 收集IDAT */
        unsigned int idat_size;
        unsigned char *idat = collect_idat(inf, &file, &idat_size);
        ipgui_vfs_fclose(&file);
        
        if (!idat) return IPGUI_ERR_MEM;
        
        /* 解压 */
        unsigned int raw_size = (row_bytes + 1) * inf->h;
        g_png_ctx.raw_data = ipgui_mem_alloc_def(raw_size);
        if (!g_png_ctx.raw_data) {
            ipgui_mem_free_def(idat);
            return IPGUI_ERR_MEM;
        }
        
        if (ipgui_inflate(idat, idat_size, g_png_ctx.raw_data, 
                          raw_size, &g_png_ctx.raw_size) < 0) {
            ipgui_mem_free_def(idat);
            ipgui_mem_free_def(g_png_ctx.raw_data);
            g_png_ctx.raw_data = NULL;
            return IPGUI_ERR_PNG_INFLATE;
        }
        ipgui_mem_free_def(idat);
        
        /* 分配行缓冲 */
        g_png_ctx.row_bytes = row_bytes;
        g_png_ctx.prev_row = ipgui_mem_alloc_def(row_bytes);
        g_png_ctx.curr_row = ipgui_mem_alloc_def(row_bytes);
        if (!g_png_ctx.prev_row || !g_png_ctx.curr_row) {
            return IPGUI_ERR_MEM;
        }
        
        g_png_ctx.current_y = -1;
        g_png_ctx.decoded = 1;
    }
    
    /* 逐像素输出 */
    int pixels_done = 0;
    int max_pix = (inf->w - img_x) + (inf->h - img_y - 1) * inf->w;
    if (pix_num > max_pix) pix_num = max_pix;
    
    ipgui_coord_t cx = img_x, cy = img_y;
    
    while (pixels_done < pix_num) {
        /* 需要解码新行? */
        if (cy != g_png_ctx.current_y) {
            /* 交换prev/curr */
            unsigned char *tmp = g_png_ctx.prev_row;
            g_png_ctx.prev_row = g_png_ctx.curr_row;
            g_png_ctx.curr_row = tmp;
            
            /* 定位到该行 */
            unsigned int row_off = cy * (row_bytes + 1);
            unsigned char filter = g_png_ctx.raw_data[row_off];
            
            /* 复制行数据 */
            for (int i = 0; i < row_bytes; i++) {
                g_png_ctx.curr_row[i] = g_png_ctx.raw_data[row_off + 1 + i];
            }
            
            /* 反滤波 */
            unfilter_row(g_png_ctx.curr_row, 
                         (cy > 0) ? g_png_ctx.prev_row : NULL,
                         filter, inf->bpp, row_bytes);
            
            g_png_ctx.current_y = cy;
        }
        
        /* 本行剩余像素 */
        int row_remain = inf->w - cx;
        int to_read = (pix_num - pixels_done < row_remain) 
                    ? (pix_num - pixels_done) : row_remain;
        
        /* 转换为BGRA */
        row_to_bgra(inf, g_png_ctx.curr_row, out + pixels_done, cx, to_read);
        
        pixels_done += to_read;
        cx += to_read;
        if (cx >= inf->w) {
            cx = 0;
            cy++;
        }
    }
    
    if (pixs_nr) *pixs_nr = pixels_done;
    return IPGUI_ERR_OK;
}

/* 释放解码上下文 */
void ipgui_png_ctx_free(void)
{
    if (g_png_ctx.raw_data) {
        ipgui_mem_free_def(g_png_ctx.raw_data);
        g_png_ctx.raw_data = NULL;
    }
    if (g_png_ctx.prev_row) {
        ipgui_mem_free_def(g_png_ctx.prev_row);
        g_png_ctx.prev_row = NULL;
    }
    if (g_png_ctx.curr_row) {
        ipgui_mem_free_def(g_png_ctx.curr_row);
        g_png_ctx.curr_row = NULL;
    }
    g_png_ctx.decoded = 0;
    g_png_ctx.current_y = -1;
}