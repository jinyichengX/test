#include "ipgui_bmp_dec.h"
#include "ipgui_vfs.h"
#include "ipgui_coord.h"
#include "ipgui_utils.h"
#include "ipgui_debug.h"
#include "ipgui_memory.h"

/* bmp调色板的单位大小为4字节 按B,G,R,X（X一般为0）顺序存储 */

IPGUI_ST_ALIGN(1) struct bmp_file_hdr
{
    s8_t tc1;         /* type code must be 'B' */
    s8_t tc2;         /* type code must be 'M' */ 
    s8_t size[4];     /* can be ignored */
    s8_t reserved[4]; /* ignored, must all be 0 */
    u8_t pixs_off[4]; /* offset off pixmap */
};

IPGUI_ST_ALIGN(1) struct bmp_hdr
{
    u8_t hsz[4]; /* bmp header size */
    s8_t w[4]; /* image width */
    s8_t h[4]; /* image height */
    u8_t planes[2]; /* must be 1!!! or an err bmp */
    u8_t bpp[2]; /* bits per pixel：如果为1 4 8，每个像素存储的是调色板索引 */
    u8_t compress[4]; /* 0:not compressed(most case) 1/2:RLE */
    u8_t img_sz[4]; /* ignored */
    u8_t xppm[4]; /* ignored, pixels per meter(像素/米) */
    u8_t yppm[4];   /* ignored, pixels per meter(像素/米) */
    u8_t cr_used[4]; /* color used(调色板实际使用的颜色数为0表示所有) */
    u8_t cr_important[4]; /* ignored, important color number */
    u8_t r_mask[4];
    u8_t g_mask[4];
    u8_t b_mask[4];
};

#define BFH_SZ sizeof(struct bmp_file_hdr) /* bmp file header size */
#define BH_SZ  sizeof(struct bmp_hdr) /* bmp header size */

static inline s32_t le16(u8_t * s)
{
    return  (*s) | \
            ((*(s + 1)) << 8);
}

static inline s32_t le32(u8_t * s)
{
    return  (*s) | \
            ((*(s + 1)) << 8) | \
            ((*(s + 2)) << 16) | \
            ((*(s + 3)) << 24);
}

struct bmp_inf
{
    /* path */
    const s8_t * path;

    /* direct info */
    s32_t pixs_off; /* 颜色偏移/调色板索引偏移 */

    u32_t has_pattle : 1;
    u32_t pat_off : 29; /* 调色板偏移 */
    u32_t flip : 1; /* 0正向/1倒向 */
    u32_t has_mask : 1;
    u32_t * pattle;
    ipgui_coord_t w;
    ipgui_coord_t h;

    u32_t r_mask, g_mask, b_mask; /* 只支持bpp为16 */

    s8_t bpp; /* 颜色位数/调色板索引值位数 */

    // /* generated info */
    // s32_t bpl; /* bytes per line */
};

typedef ipgui_err_t (* bmp_dec)(void * bh, struct bmp_inf * inf);/* in this func, bh must be cast to v3/v4/v5 pointer */
 
struct bmp_hdr_and_dec {
    s32_t bhsz; /* bmp header size */ 
    bmp_dec bmp_decoder;
};

static ipgui_err_t bmp_decoder_standard(void * bh, struct bmp_inf * inf);
static ipgui_err_t bmp_decoder_v4(void * bh, struct bmp_inf * inf);
static ipgui_err_t bmp_decoder_v5(void * bh, struct bmp_inf * inf);

/* some fixed parameter, can't be modified!!! */
#define IPGUI_BMP_FIL_HDR_SIZE 14
#define IPGUI_BMP_STD_HDR_SIZE 40
#define IPGUI_BMP_V4_HDR_SIZE  108
#define IPGUI_BMP_V5_HDR_SIZE  124

const static struct bmp_hdr_and_dec bmp_hdr_and_dec_arr[3] = {
    {IPGUI_BMP_STD_HDR_SIZE, bmp_decoder_standard},
    {IPGUI_BMP_V4_HDR_SIZE, bmp_decoder_v4},
    {IPGUI_BMP_V5_HDR_SIZE, bmp_decoder_v5}
};

static ipgui_err_t bmp_decoder_standard(void * bh, struct bmp_inf * inf)
{
    s32_t compress;
    struct bmp_hdr * bh_std;
    bh_std = (struct bmp_hdr *)bh;

    /* get size of bmp image */
    inf->w = le32((u8_t *)&bh_std->w);
    inf->h = le32((s8_t *)&bh_std->h);
    if (inf->h > 0) {
        inf->flip = 1;
    } else {
        ipgui_dbg_warning("flip=0 have not been tested yet\r\n");
        inf->flip = 0;
        inf->h = -inf->h;
    }

    if (le16((s8_t *)&bh_std->planes) != 1)    /* get planes, must be 0x0001 */
        return IPGUI_ERR_BMP_BAD;
    
    inf->bpp = le16((s8_t *)&bh_std->bpp);    /* get bpp */
    compress = le32((s8_t *)&bh_std->compress); /* get compress */
    if ((compress != 0) && (compress != 3)) {
        /* do not support RLE now */
        return IPGUI_ERR_BMP_RLE;
    }

    if (compress == 3) {
        inf->has_mask = 1;
    }

    if ((inf->bpp == 1) || (inf->bpp == 4) || (inf->bpp == 8)) {
        inf->has_pattle = 1;
        inf->pat_off = 54;
    } else inf->has_pattle = 0;

    return IPGUI_ERR_OK;
}

/* same with bmp_decoder_standard */
static ipgui_err_t bmp_decoder_v4(void * bh, struct bmp_inf * inf)
{
    ipgui_err_t ret;

    ret = bmp_decoder_standard(bh, inf);
    if (ret == IPGUI_ERR_OK) {
        if (inf->has_pattle == 1){
            ipgui_dbg_warning("have not confirm offset is 122\r\n");
            inf->pat_off = IPGUI_BMP_FIL_HDR_SIZE + IPGUI_BMP_V4_HDR_SIZE;
        }
    }

    return ret;
}

/* same with bmp_decoder_standard */
static ipgui_err_t bmp_decoder_v5(void * bh, struct bmp_inf * inf)
{
    ipgui_err_t ret;

    ret = bmp_decoder_standard(bh, inf);
    if (ret == IPGUI_ERR_OK) {
        if (inf->has_pattle == 1){
            ipgui_dbg_warning("have not confirm offset is 138\r\n");
            inf->pat_off = IPGUI_BMP_FIL_HDR_SIZE + IPGUI_BMP_V5_HDR_SIZE;
        }
    }

    return ret;
}

static s32_t validate_masks(u32_t rm, u32_t gm, u32_t bm)
{
    return ((rm & gm) == 0) && ((rm & bm) == 0) && ((gm & bm) == 0);
}

/* path is the name of bmp file */
ipgui_err_t ipgui_bmp_dec(const s8_t * path, struct bmp_inf * inf)
{
    ipgui_err_t ret;
    ipgui_file_t file;
    struct bmp_file_hdr bfh;
    struct bmp_hdr bh;
    s32_t rd_sz, pix_off, hsz;

    ipgui_memset(&file, 0, sizeof(ipgui_file_t));
    ipgui_link_fs_auto(&file);
    ret = ipgui_vfs_fopen(&file, path, IPGUI_FILE_MODE_READ, 0, 0);
    if (ret != IPGUI_ERR_OK)
        return ret;

    ret = ipgui_vfs_fread(&file, &bfh, BFH_SZ, &rd_sz);
    if (ret != IPGUI_ERR_OK)
        goto _return;

    if (bfh.tc1 != 'B' || bfh.tc2 != 'M') {
        ret = IPGUI_ERR_BMP_FAKE;
        goto _return;
    }
    
    /* get offset info from bmp file header */
    pix_off = le32(bfh.pixs_off);
    inf->pixs_off = pix_off;

    /* get bmp header size from bmp header */
    ret = ipgui_vfs_fread(&file, &bh, BH_SZ, &rd_sz);
    if ((ret != IPGUI_ERR_OK) || (BH_SZ != rd_sz))
        goto _return;

    hsz = le32((s8_t *)&bh.hsz);
    /* check if header is supportted, v4 header or v5 header may not support */
    ret = IPGUI_ERR_NOK;
    for (s32_t i = 0; i < IPGUI_ARRAY_LEN(bmp_hdr_and_dec_arr); i ++) {
        if (hsz != bmp_hdr_and_dec_arr[i].bhsz)
            continue;
        if (bmp_hdr_and_dec_arr[i].bmp_decoder) {
            ret = bmp_hdr_and_dec_arr[i].bmp_decoder((void *)&bh, inf);
            break;
        } else continue;
    }

    if (ret != IPGUI_ERR_OK)
        goto _return;
        
    inf->path = path;
    if (inf->has_pattle) {
        /* copy pattle to inf->pattle */
        s32_t pattle_sz = inf->pixs_off - inf->pat_off;
        inf->pattle = ipgui_mem_alloc_def(pattle_sz);
        if (!inf->pattle) {
            ret = IPGUI_ERR_NOMEM;
            goto _return;
        }
        s32_t br = 0;
        ret = ipgui_vfs_fseek(&file, IPGUI_FILE_SEEK_SET, inf->pat_off);
        if (ret != IPGUI_ERR_OK) goto _return;
        ret = ipgui_vfs_fread(&file, inf->pattle, pattle_sz, &br);
        if (ret != IPGUI_ERR_OK) goto _return;
        if (br != pattle_sz) {
            ipgui_mem_free_def(inf->pattle);
            inf->pattle = (u32_t *)0;
            ret = IPGUI_ERR_BMP_PAT_READ;
            goto _return;
        }
    }

    if (inf->has_mask) {
        if (inf->bpp == 16) {
            /* 磁盘中严格按照R,G,B（每个颜色的掩码4字节）顺序存储的，共12字节 */
            inf->r_mask = le32(bh.r_mask);
            inf->g_mask = le32(bh.g_mask);
            inf->b_mask = le32(bh.b_mask);
            if (!validate_masks(inf->r_mask, inf->g_mask, inf->b_mask))
                ret = IPGUI_ERR_BMP_MASK;
        } else if (inf->bpp == 32) {
            /* 用不上，忽略 */
        }
    }
_return:
    ipgui_vfs_fclose(&file);
    return ret;
}

/* read 24 or 32 bits pixels */
static ipgui_err_t read_true_color(ipgui_file_t * f,
    struct bmp_inf * inf, 
    ipgui_coord_t img_x, ipgui_coord_t img_y, /* rel x,y coord of image */
    void * buffer, ipgui_coord_t pix_num)
{
    s32_t off;
    u8_t skip_sz, px_sz;/* per line skip bytes, per pixel bytes */
    s32_t line_sz;  /* per line bytes(with skip size) */
    ipgui_err_t ret;

    px_sz = inf->bpp >> 3;
    line_sz = (((inf->w * inf->bpp) + 31) >> 5) << 2;
    skip_sz = (4 - ((inf->w * px_sz) % 4)) % 4;
    
    s32_t btr, br, row_remain; /* pixels to read in a row */
    if (inf->flip == 0) {
        ipgui_dbg_warning("flip=0 have not been tested yet\r\n");
        off = inf->pixs_off + img_y * line_sz + img_x * px_sz;

        /* read a row once */
        while (pix_num) {
            ret = ipgui_vfs_fseek(f, IPGUI_FILE_SEEK_SET, off);
            if (ret != IPGUI_ERR_OK) break;

            row_remain = inf->w - img_x;
            if (pix_num < row_remain) row_remain = pix_num;
            btr = px_sz * row_remain;/* size of the real pixels data */
            ret = ipgui_vfs_fread(f, buffer, btr, &br);
            if (ret != IPGUI_ERR_OK) break;

            if (btr != br) {
                ret = IPGUI_ERR_NOK;
                break;
            }
            
            buffer = (void *)((s8_t *)buffer + br);
            off += (skip_sz + br);/* get next line offset */
            pix_num -= row_remain;
            img_x = 0;
        }
    } else {
        off = inf->pixs_off + (inf->h - 1 - img_y) * line_sz + img_x * px_sz;

        while (pix_num) {
            ret = ipgui_vfs_fseek(f, IPGUI_FILE_SEEK_SET, off);
            if (ret != IPGUI_ERR_OK) break;

            row_remain = inf->w - img_x;
            if (pix_num < row_remain) row_remain = pix_num;
            btr = px_sz * row_remain;/* size of the real pixels data */
            ret = ipgui_vfs_fread(f, buffer, btr, &br);
            if (ret != IPGUI_ERR_OK) break;

            if (btr != br) {
                ret = IPGUI_ERR_NOK;
                break;
            }
            
            buffer = (void *)((s8_t *)buffer + br);
            off -= (2 * line_sz - br - skip_sz);/* get next line offset */
            pix_num -= row_remain;
            img_x = 0;
        }
    }

    return ret;
}

/* read 16 bits pixels */
static ipgui_err_t read_rgb16(ipgui_file_t * f,
    struct bmp_inf * inf, 
    ipgui_coord_t img_x, ipgui_coord_t img_y, /* rel x,y coord of image */
    void * buffer, ipgui_coord_t pix_num)
{
    return read_true_color(f, inf, img_x, img_y, buffer, pix_num);
}

/* 4 byte align for low bpp image */
static inline ipgui_coord_t get_shallow_bmp_line_sz(s32_t bpp, ipgui_coord_t w)
{
    if (bpp == 1)
        return ((w + 31) >> 5) << 2;
    else if (bpp == 4)
        return (((w << 2) + 31) >> 5) << 2;
    else if (bpp == 8)
        return ((w +  3) >> 2) << 2;
}

/* read 1 4 8 bits pixels */
static ipgui_err_t read_shallow(ipgui_file_t * f, 
    struct bmp_inf * inf,
    ipgui_coord_t img_x, ipgui_coord_t img_y,
    void * buffer, ipgui_coord_t pix_num)
{
    ipgui_err_t ret;
    ipgui_coord_t line_sz;

    if (!inf->has_pattle) {
        ipgui_dbg_error("err: wtf?! something wrong happened at the decode stage\r\n");
        return IPGUI_ERR_NOK;
    }

    if (!inf->pattle)
        return IPGUI_ERR_BMP_PAT_NULL;

    #define IDX_CACHE_SIZE 1024
    static u8_t idx_cache[IDX_CACHE_SIZE];

    line_sz = get_shallow_bmp_line_sz(inf->bpp, inf->w);

    if (line_sz > IDX_CACHE_SIZE)
        return IPGUI_ERR_BMP_PAT_IDX_CACHE;

    s32_t bytes_to_read, br;
    s32_t pixels_processed = 0;
    u32_t * dest_buffer = (u32_t *)buffer;
    ipgui_coord_t current_x = img_x;
    ipgui_coord_t current_y = img_y;

    while (pixels_processed < pix_num) {
        s32_t pixels_remain_in_row = inf->w - current_x;
        s32_t pixels_to_read = (pix_num - pixels_processed < pixels_remain_in_row) 
                            ? (pix_num - pixels_processed) 
                            : pixels_remain_in_row;

        s32_t start_byte = (current_x * inf->bpp) / 8;
        s32_t end_byte = ((current_x + pixels_to_read) * inf->bpp + 7) / 8;
        bytes_to_read = end_byte - start_byte;

        s32_t current_off;
        if (inf->flip == 0) {
            current_off = inf->pixs_off + current_y * line_sz + start_byte;
        } else {
            current_off = inf->pixs_off + (inf->h - 1 - current_y) * line_sz + start_byte;
        }

        ret = ipgui_vfs_fseek(f, IPGUI_FILE_SEEK_SET, current_off);
        if (ret != IPGUI_ERR_OK) 
            return ret;

        ret = ipgui_vfs_fread(f, idx_cache, bytes_to_read, &br);
        if (ret != IPGUI_ERR_OK) 
            return ret;
        if (bytes_to_read != br) 
            return IPGUI_ERR_NOK;

        for (s32_t i = 0; i < pixels_to_read; i ++) {
            u32_t pattle_index = 0;
            s32_t pixel_x = current_x + i;
            s32_t pixel_bit_pos = pixel_x * inf->bpp;
            s32_t byte_pos = pixel_bit_pos / 8 - start_byte;
            s32_t bit_offset = pixel_bit_pos % 8;

            if (inf->bpp == 8) {
                pattle_index = idx_cache[byte_pos];
            } else if (inf->bpp == 4) {
                u8_t byte = idx_cache[byte_pos];
                if (bit_offset == 0) {
                    pattle_index = (byte >> 4) & 0x0F;
                } else {
                    pattle_index = byte & 0x0F;
                }
            } else {
                u8_t byte = idx_cache[byte_pos];
                pattle_index = (byte >> (7 - bit_offset)) & 0x01;
            }

            dest_buffer[pixels_processed] = inf->pattle[pattle_index];
            pixels_processed ++;
        }

        current_x += pixels_to_read;
        if (current_x >= inf->w) {
            current_x = 0;
            current_y ++; 
            if (current_y >= inf->h) {
                break;
            }
        }
    }

    return IPGUI_ERR_OK;
}

/* 读取bmp数据，不是读取一行，而是按行顺序读取，一行数据读完继续读下一行，直到满足pix_num */
ipgui_err_t ipgui_bmp_read_linebyline(struct bmp_inf * inf, 
    ipgui_coord_t img_x, ipgui_coord_t img_y, /* rel x,y coord of image */
    void * buffer, ipgui_coord_t pix_num, ipgui_coord_t * pix_nr/* how many pixels read actually */)
{
    ipgui_err_t ret;
    ipgui_file_t file;
    const s8_t * path;
    if (((img_x >= inf->w) || (img_y >= inf->h)) ||
        ((img_x < 0) || (img_y < 0)))
        return IPGUI_ERR_OUT_OF_BOUNDS;
    ipgui_memset(&file, 0, sizeof(ipgui_file_t));
    path = inf->path;
    ipgui_link_fs_auto(&file);
    ret = ipgui_vfs_fopen(&file, path, IPGUI_FILE_MODE_READ, 0, 0);
    if (ret != IPGUI_ERR_OK)
        return ret;

    /* get left size, compare with pix_num */
    s32_t max_pixs = (inf->w - img_x) + (inf->h - img_y - 1) * inf->w;
    if (pix_num > max_pixs) {
        pix_num = max_pixs;
    }

    if ((inf->bpp == 24) || (inf->bpp == 32)) {
        ret = read_true_color(&file, inf, img_x, img_y, buffer, pix_num);
    } else if (inf->bpp == 16) {
        ret = read_rgb16(&file, inf, img_x, img_y, buffer, pix_num);
    } else {
        ret = read_shallow(&file, inf, img_x, img_y, buffer, pix_num);
    }
    if (pix_nr && (ret == IPGUI_ERR_OK))
        * pix_nr = pix_num;
    else if (pix_nr)
        * pix_nr = 0;
_return:
    ipgui_vfs_fclose(&file);
    return ret;
}

struct bmp_inf * ipgui_bmp_dec_create(void)
{
    struct bmp_inf * inf;
    inf = (struct bmp_inf *)ipgui_mem_alloc_def(sizeof(struct bmp_inf));
    return inf;
}

void ipgui_bmp_dec_destroy(struct bmp_inf * inf)
{
    if (!inf) return;

    if (inf->pattle)
        ipgui_mem_free_def((void *)inf->pattle);
    
    ipgui_mem_free_def((void *)inf);
}

ipgui_err_t ipgui_bmp_dec_module_init(void)
{
    if (BFH_SZ != 14) return IPGUI_ERR_BMP_FIL_HDR_STRUCT_LEN;
    else return IPGUI_ERR_OK;
}

//test_bmp不要删，有内存管理或虚拟文件系统bug，将once_read_len改为1可以复现
#include "ipgui_screen.h"
#include "ipgui_angle.h"
#include "ipgui_graphic2.h"
#include "ipgui_image_geometry_transform.h"
extern ipgui_scr_t * sdl_scr;

extern void clear_fucking_screen(ipgui_scr_t * scr);
void test_bmp_rotate(u8_t * buffer, s32_t w, s32_t h, s32_t pix_sz, ipgui_angle_t angle);
int test_bmp(const s8_t * path, ipgui_img_dsc_t * image) 
{
    static u8_t buffer[1024*1024*6];
    static u8_t mask_buffer[1024*1024];
    ipgui_memset(mask_buffer, 255, sizeof(mask_buffer));
    struct bmp_inf inf;
    ipgui_err_t ret;
    ret = ipgui_bmp_dec(path, &inf);
    // ret = ipgui_bmp_dec("C:/Users/9999/Desktop/linghua.bmp", &inf);
    if (ret != IPGUI_ERR_OK) {
        ipgui_dbg_error("decode bmp error\r\n");
        return -1;
    }
//     s32_t cnt0 = 0,cnt = 0;
//     s32_t cnt255 = 0;
//     s32_t once_read_len = 100*100;
//     s32_t img_len = inf.w * inf.h;
//     s32_t y_off = 0, x_off = 0;
//     while(img_len > 0) {
//         s32_t ol = once_read_len;
//         if (ol > img_len) ol = img_len;
//         ipgui_bmp_read_linebyline(&inf, x_off, y_off, buffer, ol, NULL);
//         ipgui_color_t color;

//         s32_t x_off_bk = x_off;
//         s32_t y_off_bk = y_off;
//         for (s32_t i = 0; i < ol; i ++) {
//             if (inf.bpp == 24) {//24
//                 color.r = buffer[i * 3 + 2];
//                 color.g = buffer[i * 3 + 1];
//                 color.b = buffer[i * 3];
//             } else if (inf.bpp != 16) { // 1 4 8 32
//                 color.r = buffer[i * 4 + 2];
//                 color.g = buffer[i * 4 + 1];
//                 color.b = buffer[i * 4];
//             } else {// 16
// /*总结：compress为0和为3时的情况
// * 为0 → 必是555（标准规定）,无掩码
// * 为3 → 5551或555或565,有掩码
// */                   
//                 /* 下面这一大坨逻辑可以用下面的函数封装 
//                     static void extract_16bit_pixel(uint16_t pixel, 
//                             uint32_t r_mask, uint32_t g_mask, uint32_t b_mask,
//                             uint8_t *r, uint8_t *g, uint8_t *b) 
//                     {
//                         // 通用掩码提取
//                         uint32_t r_shift = get_mask_shift(r_mask);
//                         uint32_t g_shift = get_mask_shift(g_mask);
//                         uint32_t b_shift = get_mask_shift(b_mask);
                        
//                         *r = ((pixel & r_mask) >> r_shift);
//                         *g = ((pixel & g_mask) >> g_shift);
//                         *b = ((pixel & b_mask) >> b_shift);
                        
//                         // 根据位数扩展到8位
//                         *r = expand_to_8bit(*r, get_mask_bits(r_mask));
//                         *g = expand_to_8bit(*g, get_mask_bits(g_mask));
//                         *b = expand_to_8bit(*b, get_mask_bits(b_mask));
//                     }
//                 */


//                 unsigned short cr = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
//                 u8_t r,g,b;
//                 if (!inf.has_mask) {
//                     /*cr:内存排列（X1R5G5B5 XRRRRRGG GGGBBBBB）X是无效位 */
//                     r = (cr & 0x7c00) >> 10;
//                     g = (cr & 0x03e0) >> 5;
//                     b = (cr & 0x001f);
//                     color.r = r << 3;
//                     color.g = g << 3;
//                     color.b = b << 3;
//                 } else {
//                     if (inf.r_mask == 0xf800) {
//                         /* case 1 rgb565 */
//                         /* cr的内存排列 
//                         * 第1字节（低地址）：GGGBBBBB
//                         * 第2字节（高地址）：RRRRRGGG
//                         */
//                         r = (cr & inf.r_mask) >> 11;
//                         g = (cr & inf.g_mask) >> 5;
//                         b = (cr & inf.b_mask);
//                         color.r = r << 3;
//                         color.g = g << 2;
//                         color.b = b << 3;
//                     } else if (inf.r_mask == 0x7c00) {
//                         /* case 2 argb1555 */
//                         /* cr的内存排列 
//                         * 第1字节（低地址）：GGGBBBBB
//                         * 第2字节（高地址）：ARRRRRGG
//                         */
//                         r = (cr & inf.r_mask) >> 10;
//                         g = (cr & inf.g_mask) >> 5;
//                         b = (cr & inf.b_mask);
//                         color.r = r << 3;
//                         color.g = g << 3;
//                         color.b = b << 3;
//                     } else {
//                         /* wtf */
//                     }
//                 }
//             }
//             sdl_scr->drv->put_pixel(sdl_scr, x_off_bk, y_off_bk, color);
//             x_off_bk ++;
//             if (x_off_bk >= inf.w) {
//                 y_off_bk ++;
//                 x_off_bk = 0;
//             }
//         }
//         sdl_scr->drv->flush(sdl_scr);
//         x_off = (x_off + ol);
//         if (x_off >= inf.w) {
//             y_off += x_off / inf.w;
//             x_off = x_off % inf.w;
//         }
//         img_len -= once_read_len;
//     }
//     sdl_scr->drv->flush(sdl_scr);


    ipgui_bmp_read_linebyline(&inf, 0, 0, buffer, inf.h * inf.w, NULL);
    image->h = inf.h;
    image->w = inf.w;
    image->stride = ((inf.bpp == 24) ? 3 : 4) * inf.w;
    image->pixmap = buffer;
    image->fmt = (inf.bpp == 24) ? IPGUI_IMG_FMT_RGB888:  IPGUI_IMG_FMT_ARGB8888;
    image->mask = mask_buffer;
    // s32_t pixel_size = ((inf.bpp == 24) ? 3 : (inf.bpp == 16) ? 2 : 4);
    // // while(1) {
    // //     clear_fucking_screen(sdl_scr);
    //     static s32_t angle = 30;
    //     test_bmp_rotate(buffer, inf.w, inf.h, pixel_size, IPGUI_ANGLE_PRECISION * angle ++);
    // // }

    return 0;
}
