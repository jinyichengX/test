/*
 * 轻量级 inflate 实现，适合嵌入式
 * 支持 deflate 格式（PNG使用的zlib压缩）
 */

#include "ipgui_png_dec.h"
#include "ipgui_memory.h"

/* 位流读取器 */
typedef struct {
    const unsigned char *data;
    unsigned int size;
    unsigned int pos;      /* 字节位置 */
    unsigned int bit_pos;  /* 位位置 0-7 */
    unsigned int bit_buf;  /* 位缓冲 */
    unsigned int bit_cnt;  /* 缓冲中的位数 */
} bitstream_t;

static void bs_init(bitstream_t *bs, const unsigned char *data, unsigned int size)
{
    bs->data = data;
    bs->size = size;
    bs->pos = 0;
    bs->bit_pos = 0;
    bs->bit_buf = 0;
    bs->bit_cnt = 0;
}

static inline void bs_refill(bitstream_t *bs)
{
    while (bs->bit_cnt <= 24 && bs->pos < bs->size) {
        bs->bit_buf |= (unsigned int)bs->data[bs->pos++] << bs->bit_cnt;
        bs->bit_cnt += 8;
    }
}

static inline unsigned int bs_read(bitstream_t *bs, int bits)
{
    bs_refill(bs);
    unsigned int val = bs->bit_buf & ((1u << bits) - 1);
    bs->bit_buf >>= bits;
    bs->bit_cnt -= bits;
    return val;
}

static inline unsigned int bs_peek(bitstream_t *bs, int bits)
{
    bs_refill(bs);
    return bs->bit_buf & ((1u << bits) - 1);
}

static inline void bs_drop(bitstream_t *bs, int bits)
{
    bs->bit_buf >>= bits;
    bs->bit_cnt -= bits;
}

/* Huffman解码表 */
#define HUFF_MAX_BITS 15
#define HUFF_LOOKUP_BITS 9

typedef struct {
    unsigned short counts[HUFF_MAX_BITS + 1];
    unsigned short symbols[288];
    unsigned short lookup[1 << HUFF_LOOKUP_BITS];
} huffman_t;

/* 构建Huffman表 */
static int huff_build(huffman_t *h, const unsigned char *lengths, int num)
{
    int i, j, code, cnt;
    unsigned short offsets[HUFF_MAX_BITS + 1];
    
    for (i = 0; i <= HUFF_MAX_BITS; i++) h->counts[i] = 0;
    for (i = 0; i < num; i++) h->counts[lengths[i]]++;
    
    h->counts[0] = 0;
    offsets[0] = 0;
    for (i = 1; i <= HUFF_MAX_BITS; i++) {
        offsets[i] = offsets[i-1] + h->counts[i-1];
    }
    
    for (i = 0; i < num; i++) {
        if (lengths[i]) {
            h->symbols[offsets[lengths[i]]++] = i;
        }
    }
    
    /* 构建快速查找表 */
    for (i = 0; i < (1 << HUFF_LOOKUP_BITS); i++) h->lookup[i] = 0;
    
    code = 0;
    cnt = 0;
    for (i = 1; i <= HUFF_MAX_BITS && i <= HUFF_LOOKUP_BITS; i++) {
        for (j = 0; j < h->counts[i]; j++) {
            int sym = h->symbols[cnt++];
            int fill = 1 << (HUFF_LOOKUP_BITS - i);
            int idx = code << (HUFF_LOOKUP_BITS - i);
            while (fill--) {
                h->lookup[idx++] = (sym << 4) | i;
            }
            code++;
        }
        code <<= 1;
    }
    
    return 0;
}

/* Huffman解码 */
static int huff_decode(bitstream_t *bs, huffman_t *h)
{
    bs_refill(bs);
    unsigned int bits = bs_peek(bs, HUFF_LOOKUP_BITS);
    unsigned short entry = h->lookup[bits];
    
    if (entry) {
        int len = entry & 0xF;
        bs_drop(bs, len);
        return entry >> 4;
    }
    
    /* 慢速路径：超过查找表位数 */
    int code = bits;
    int len = HUFF_LOOKUP_BITS;
    int cnt = 0;
    
    for (int i = 1; i < HUFF_LOOKUP_BITS; i++) cnt += h->counts[i];
    
    for (len = HUFF_LOOKUP_BITS + 1; len <= HUFF_MAX_BITS; len++) {
        code = bs_peek(bs, len);
        /* 反转位序 */
        int rev = 0;
        for (int i = 0; i < len; i++) {
            rev = (rev << 1) | ((code >> i) & 1);
        }
        int first = 0;
        for (int i = 1; i < len; i++) {
            first = (first + h->counts[i]) << 1;
        }
        if (rev - first < h->counts[len]) {
            bs_drop(bs, len);
            return h->symbols[cnt + rev - first];
        }
        cnt += h->counts[len];
    }
    
    return -1;
}

/* 固定Huffman表 */
static huffman_t fixed_lit, fixed_dist;
static int fixed_init = 0;

static void init_fixed_huffman(void)
{
    if (fixed_init) return;
    
    unsigned char lit_lens[288], dist_lens[32];
    int i;
    
    for (i = 0; i < 144; i++) lit_lens[i] = 8;
    for (i = 144; i < 256; i++) lit_lens[i] = 9;
    for (i = 256; i < 280; i++) lit_lens[i] = 7;
    for (i = 280; i < 288; i++) lit_lens[i] = 8;
    huff_build(&fixed_lit, lit_lens, 288);
    
    for (i = 0; i < 32; i++) dist_lens[i] = 5;
    huff_build(&fixed_dist, dist_lens, 32);
    
    fixed_init = 1;
}

/* 长度和距离额外位表 */
static const unsigned short len_base[29] = {
    3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258
};
static const unsigned char len_extra[29] = {
    0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0
};
static const unsigned short dist_base[30] = {
    1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,
    1025,1537,2049,3073,4097,6145,8193,12289,16385,24577
};
static const unsigned char dist_extra[30] = {
    0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
};

/* 解压单个块 */
static int inflate_block(bitstream_t *bs, unsigned char *out, 
                         unsigned int *out_pos, unsigned int out_size,
                         huffman_t *lit_huff, huffman_t *dist_huff)
{
    while (1) {
        int sym = huff_decode(bs, lit_huff);
        if (sym < 0) return -1;
        
        if (sym < 256) {
            /* 字面量 */
            if (*out_pos >= out_size) return -1;
            out[(*out_pos)++] = (unsigned char)sym;
        } else if (sym == 256) {
            /* 块结束 */
            break;
        } else {
            /* 长度-距离对 */
            sym -= 257;
            if (sym >= 29) return -1;
            
            int length = len_base[sym] + bs_read(bs, len_extra[sym]);
            
            int dist_sym = huff_decode(bs, dist_huff);
            if (dist_sym < 0 || dist_sym >= 30) return -1;
            
            int distance = dist_base[dist_sym] + bs_read(bs, dist_extra[dist_sym]);
            
            if (*out_pos < (unsigned int)distance) return -1;
            if (*out_pos + length > out_size) return -1;
            
            /* 复制匹配 */
            unsigned char *src = out + *out_pos - distance;
            for (int i = 0; i < length; i++) {
                out[(*out_pos)++] = *src++;
            }
        }
    }
    return 0;
}

/* 动态Huffman表解码 */
static int decode_dynamic_huffman(bitstream_t *bs, huffman_t *lit, huffman_t *dist)
{
    static const unsigned char order[19] = {
        16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
    };
    
    int hlit = bs_read(bs, 5) + 257;
    int hdist = bs_read(bs, 5) + 1;
    int hclen = bs_read(bs, 4) + 4;
    
    unsigned char code_lens[19] = {0};
    for (int i = 0; i < hclen; i++) {
        code_lens[order[i]] = bs_read(bs, 3);
    }
    
    huffman_t code_huff;
    huff_build(&code_huff, code_lens, 19);
    
    unsigned char lens[288 + 32];
    int n = 0, total = hlit + hdist;
    
    while (n < total) {
        int sym = huff_decode(bs, &code_huff);
        if (sym < 0) return -1;
        
        if (sym < 16) {
            lens[n++] = sym;
        } else if (sym == 16) {
            int rep = bs_read(bs, 2) + 3;
            if (n == 0) return -1;
            while (rep-- && n < total) lens[n] = lens[n-1], n++;
        } else if (sym == 17) {
            int rep = bs_read(bs, 3) + 3;
            while (rep-- && n < total) lens[n++] = 0;
        } else {
            int rep = bs_read(bs, 7) + 11;
            while (rep-- && n < total) lens[n++] = 0;
        }
    }
    
    huff_build(lit, lens, hlit);
    huff_build(dist, lens + hlit, hdist);
    
    return 0;
}

/* 主解压函数 */
int ipgui_inflate(const unsigned char *in, unsigned int in_size,
                  unsigned char *out, unsigned int out_size,
                  unsigned int *out_len)
{
    bitstream_t bs;
    unsigned int out_pos = 0;
    int bfinal, btype;
    
    /* 跳过zlib头 (2字节) */
    if (in_size < 2) return -1;
    bs_init(&bs, in + 2, in_size - 2);
    
    init_fixed_huffman();
    
    do {
        bfinal = bs_read(&bs, 1);
        btype = bs_read(&bs, 2);
        
        if (btype == 0) {
            /* 无压缩块 */
            bs.bit_buf = 0;
            bs.bit_cnt = 0;
            
            if (bs.pos + 4 > bs.size) return -1;
            unsigned int len = bs.data[bs.pos] | (bs.data[bs.pos+1] << 8);
            bs.pos += 4;
            
            if (bs.pos + len > bs.size) return -1;
            if (out_pos + len > out_size) return -1;
            
            for (unsigned int i = 0; i < len; i++) {
                out[out_pos++] = bs.data[bs.pos++];
            }
        } else if (btype == 1) {
            /* 固定Huffman */
            if (inflate_block(&bs, out, &out_pos, out_size, 
                              &fixed_lit, &fixed_dist) < 0)
                return -1;
        } else if (btype == 2) {
            /* 动态Huffman */
            huffman_t dyn_lit, dyn_dist;
            if (decode_dynamic_huffman(&bs, &dyn_lit, &dyn_dist) < 0)
                return -1;
            if (inflate_block(&bs, out, &out_pos, out_size,
                              &dyn_lit, &dyn_dist) < 0)
                return -1;
        } else {
            return -1;
        }
    } while (!bfinal);
    
    *out_len = out_pos;
    return 0;
}