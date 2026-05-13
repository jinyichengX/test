typedef unsigned char byte;
typedef unsigned int uint;
#define nullptr NULL
/* start of frame markers */
const byte SOF0 = 0xC0;//baseline DCT
const byte SOF1 = 0xC1;
const byte SOF2 = 0xC2;
const byte SOF3 = 0xC3;

const byte SOF5 = 0xC5;
const byte SOF6 = 0xC6;
const byte SOF7 = 0xC7;

const byte SOF9 = 0xC9;
const byte SOF10 = 0xCA;
const byte SOF11 = 0xCB;

const byte SOF13 = 0xCD;
const byte SOF14 = 0xCE;
const byte SOF15 = 0xCF;

/* huffman tables */
const byte DHT = 0XC4;

/* JPEG extensions */
const byte JPG = 0XC8;

/* define arithmetic coding conditioning(s) */
const byte DAC = 0XCC;

/* restart interval markers */
const byte RST0 = 0xD0;
const byte RST1 = 0xD1;
const byte RST2 = 0xD2;
const byte RST3 = 0xD3;
const byte RST4 = 0xD4;
const byte RST5 = 0xD5;
const byte RST6 = 0xD6;
const byte RST7 = 0xD7;

/* other markers */
const byte SOI = 0xD8;
const byte EOI = 0xD9;
const byte SOS = 0xDA;
const byte DQT = 0xDB;
const byte DNL = 0xDC;
const byte DRI = 0xDD;
const byte DHP = 0xDE;
const byte EXP = 0xDF;

/* appN markers */
const byte APP0 = 0XE0;
const byte APP1 = 0XE1;
const byte APP2 = 0XE2;
const byte APP3 = 0XE3;
const byte APP4 = 0XE4;
const byte APP5 = 0XE5;
const byte APP6 = 0XE6;
const byte APP7 = 0XE7;
const byte APP8 = 0XE8;
const byte APP9 = 0XE9;
const byte APP10 = 0XEA;
const byte APP11 = 0XEB;
const byte APP12 = 0XEC;
const byte APP13 = 0XED;
const byte APP14 = 0XEE;
const byte APP15 = 0XEF;

/* Misc markers */
const byte JPG0 = 0XF0;
const byte JPG1 = 0XF1;
const byte JPG2 = 0XF2;
const byte JPG3 = 0XF3;
const byte JPG4 = 0XF4;
const byte JPG5 = 0XF5;
const byte JPG6 = 0XF6;
const byte JPG7 = 0XF7;
const byte JPG8 = 0XF8;
const byte JPG9 = 0XF9;
const byte JPG10 = 0XFA;
const byte JPG11 = 0XFB;
const byte JPG12 = 0XFC;
const byte JPG13 = 0XFD;
const byte COM = 0XFE;
const byte TEM = 0X01;

typedef struct QuantizationTableEntry {
    uint  quantization_table[64];
}jdqt_t;

typedef struct lookup_table{
    byte code_len;
    byte symbol;
    //byte code;
}lookup_tbl_t;

typedef struct slow_table
{
    uint16_t code;
    byte symbol;
    byte code_len;
}slow_tbl_t;

typedef struct haffman_table{
    byte offsets[17];
    byte symbols[162];

    /* 慢表 */
    slow_tbl_t slow_tbl[162];
    /* 快表 */
    lookup_tbl_t lookup_tbl[256];
}huff_tbl_t;

typedef struct color_component {
    byte  horizontal_sampling_factor;
    byte  vertical_sampling_factor;
    byte  quantization_table_id;//1~3共3个，定义在SOF
    byte  in_use;
}jcr_cmpn_t;

typedef struct ipgui_jpg_header_st{
    jdqt_t qtable[4];//量化表id：0~3共4个
    huff_tbl_t huff_tbl_dc[4];//Destination ID 0~3
    huff_tbl_t huff_tbl_ac[4];

    /* SOF */
    byte frame_type;//baseline or progressive
    uint height;
    uint width;
    byte num_components;//Y:1,Cb:1,Cr:1
    jcr_cmpn_t jcr_cmpn[3];

    /* DRI */
    uint16_t restart_interval;

    byte valid;
}jhdr_t;

typedef struct byte_stream_st {
    byte data;
    byte offset;
}byte_stream_t;

typedef struct img_mcu{
    union{
        int y[64];
        int r[64];
    };
    union{
        int cb[64];
        int g[64];
    };
    union{
        int cr[64];
        int b[64];
    };
}img_mcu_t;
/* DQT remap */
uint8_t zigZagMap[64] = {
   0,  1,  8, 16,  9,  2,  3, 10,
  17, 24, 32, 25, 18, 11,  4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13,  6,  7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};

static float cosines[8][8] = {
	{ 0.35355339059,0.35355339059,0.35355339059,0.35355339059,0.35355339059,0.35355339059,0.35355339059,0.35355339059 },
	{ 0.490392640202,0.415734806151,0.27778511651,0.0975451610081,-0.0975451610081,-0.27778511651,-0.415734806151,-0.490392640202 },
	{ 0.461939766256,0.191341716183,-0.191341716183,-0.461939766256,-0.461939766256,-0.191341716183,0.191341716183,0.461939766256 },
	{ 0.415734806151,-0.0975451610081,-0.490392640202,-0.27778511651,0.27778511651,0.490392640202,0.0975451610081,-0.415734806151 },
	{ 0.353553390593,-0.353553390593,-0.353553390593,0.353553390593,0.353553390593,-0.353553390593,-0.353553390593,0.353553390593 },
	{ 0.27778511651,-0.490392640202,0.0975451610081,0.415734806151,-0.415734806151,-0.0975451610081,0.490392640202,-0.27778511651 },
	{ 0.191341716183,-0.461939766256,0.461939766256,-0.191341716183,-0.191341716183,0.461939766256,-0.461939766256,0.191341716183 },
	{ 0.0975451610081,-0.27778511651,0.415734806151,-0.490392640202,0.490392640202,-0.415734806151,0.27778511651,-0.0975451610081 },
};

/* generate standard table and lookup table */
__IPGUI_STATIC__ int ipgui_jhuff_lookup_tbl_gen(jhdr_t * jhdr, huff_tbl_t * huff_tbl, byte * std_encode)
{
    int i, j, symbol_num, off = 0;

    plat_memset(huff_tbl, 0, sizeof(huff_tbl_t));

    /* generate standard table */
    for(i = 1; i < 17; i++)
        huff_tbl->offsets[i] = *(std_encode++);

    for(i = 1; i < 17; i++)
    {
        symbol_num = huff_tbl->offsets[i];
        plat_memcpy(huff_tbl->symbols + off, std_encode, symbol_num);
        std_encode += symbol_num;
        off += symbol_num;
    }

    int bitN_width_num = 0;
    uint16_t recode = 0;
    uint16_t recode_extra = 0;
    byte section_len;
    off = 0;

    /* generate lookup table */
    /* 将编码长度为1-8bit的字符放入快表 */
    for( i = 1; i < 9; i ++ )
    {
        bitN_width_num = huff_tbl->offsets[i];

        if(( bitN_width_num == 0 ) && ( recode == 0 ))
            continue;
        else if( bitN_width_num == 0 )
        {
            recode = recode << 1;
            continue;
        }
        section_len = 1 << (8 - i);
        recode_extra = recode << (8 - i);
        while(bitN_width_num --)
        {
            for( j = 0; j < section_len; j ++ )
            {
                huff_tbl->lookup_tbl[recode_extra].code_len = i;
                huff_tbl->lookup_tbl[recode_extra ++].symbol = huff_tbl->symbols[off];
            }
            if( bitN_width_num )
                recode ++;
            off ++;
        }
        recode = (++ recode) << 1;
    }
    
    int next = 0;

    /* generate slow table */
    /* 将编码长度为9-16bit的字符放入慢表 */
    for( i = 9; i < 17; i++ )
    {
        bitN_width_num = huff_tbl->offsets[i];
        if(( bitN_width_num == 0 ) && ( recode == 0 ))
            continue;
        else if( bitN_width_num == 0 )
        {
            recode = recode << 1;
            continue;
        }
        while(bitN_width_num --)
        {
            huff_tbl->slow_tbl[next].code = recode;
            huff_tbl->slow_tbl[next].code_len = i;
            huff_tbl->slow_tbl[next ++].symbol = huff_tbl->symbols[off];
            if( bitN_width_num )
                recode ++;
            off ++;
        }
        recode = (++ recode) << 1;
    }

    return 0;
}

__IPGUI_STATIC__ int ipgui_jcode_match( uint16_t code, huff_tbl_t * huff_tbl, byte * code_len )
{
    if( huff_tbl->lookup_tbl[(byte)code].code_len != 0 ){
        * code_len = huff_tbl->lookup_tbl[(byte)code].code_len;
        return huff_tbl->lookup_tbl[(byte)code].symbol;
    }
    
    return -1;
}

__IPGUI_STATIC__ int ipgui_jcode_match_slow( uint16_t code, huff_tbl_t * huff_tbl, byte * code_len )
{
    for(int i = 0; i < 162; i ++ )
    {
        if( huff_tbl->slow_tbl[i].code == code ){
            * code_len = huff_tbl->slow_tbl[i].code_len;
            return huff_tbl->slow_tbl[i].symbol;
        }
    }
    return -1;
}

__IPGUI_STATIC__ void get_nbits_shift(byte_stream_t * stream, int bitN, byte ** code_stream, int * code)
{
    int next_code = * code = 0;
    int bit;
    while( bitN -- )
    { 
        bit = (stream->data & (1 << stream->offset)) ? 1 : 0;
        next_code = (next_code << 1) + bit;
        if( !stream->offset ) {
            stream->offset = 7;
            *code_stream += 1;
            stream->data = *(* code_stream);
        }else {
            stream->offset --;
        }
    }
    * code = next_code;
}

__IPGUI_STATIC__ void get_nbits_shift_and_skip(byte_stream_t * stream, int bitN, byte ** code_stream, int * code, int skip_len)
{
    int next_code = * code = 0;
    int bit;
    while( bitN -- )
    { 
        bit = (stream->data & (1 << stream->offset)) ? 1 : 0;
        next_code = (next_code << 1) + bit;
        if( !stream->offset ) {
            stream->offset = 7;
            *code_stream += 1;
            *code_stream += (skip_len > 0) ? skip_len : 0;
            skip_len --;
            stream->data = *(* code_stream);
        }else {
            stream->offset --;
        }
    }
    * code = next_code;
}

__IPGUI_STATIC__ void peek_nbits(byte_stream_t * stream, int bitN, byte * code_stream, int * code, int skip_len)
{
    int next_code = * code = 0;
    int bit;
    byte_stream_t stream1 = {.data = stream->data, .offset = stream->offset};
    while(bitN --)
    {
        bit = (stream->data & (1 << stream->offset)) ? 1 : 0;
        next_code = (next_code << 1) + bit;
        if( !stream->offset ) {
            stream->offset = 7;
            code_stream ++;
            code_stream += skip_len;
            stream->data = * code_stream;
        }else {
            stream->offset --;
        }
    }
    stream->data = stream1.data;
    stream->offset = stream1.offset;
    * code = next_code;
}

int lum[1];

struct idct {
	float base[64];
};

__IPGUI_STATIC__ __IPGUI_INLINE__ int gen_mask(int len)
{
    int mask = 0;
    for(int i = 0; i < len; i ++)
        mask |= 1 << i;
    return mask;
}

#define PI 3.1416
float DCT_Mat[100][100];
float DctMap[100][100];
float DctMapTmp[100][100];
#define MAT_SIZE 8

void InitTransMat(void)
{

    int i,j;
    float a;

    for(i=0;i<MAT_SIZE;i++)
    {
        for(j=0;j<MAT_SIZE;j++)
        {
            a = 0;
            if(i==0)
            {
                a=sqrt((float)1/MAT_SIZE);
            }
            else
            {
                a=sqrt((float)2/MAT_SIZE);
            }
            DCT_Mat[i][j]= a*cos((j+0.5)*PI*i/MAT_SIZE);
        }
    }

}

void IDCT2(float *block)
{
    float t=0;
    int i,j,k;
    for(i=0;i<8;i++)  //相当于A'*I
    {
        for(j=0;j<8;j++)
        {
            t=0;
            for(k=0;k<8;k++)
            {
                t += DCT_Mat[k][i] * block[k*8+j]; //矩阵的乘法，DCT_Mat的第i列乘DctMap的第j列
            }
            DctMapTmp[i][j]=t;
        }
    }
    for(i=0;i<8;i++)  //相当于（A*I）后再*A‘
    {
        for(j=0;j<8;j++)
        {
            t=0;
            for(k=0;k<8;k++)
            {
                t += DctMapTmp[i][k] * DCT_Mat[k][j];
            }
            block[i*8+j]=t;
        }
    }
}

__IPGUI_STATIC__ int clamp(int col) {
	if (col > 255) return 255;
	if (col < 0) return 0;
	return col;
}

/* YCbCr to RGB conversion */
__IPGUI_STATIC__ void color_conversion(float Y, float Cb, float Cr, int *R, int *G, int *B) 
{
	float r = (Cr*(2.0-2.0*0.299) + Y);
	float b = (Cb*(2.0-2.0*0.114) + Y);
	float g = (Y - 0.144 * b - 0.229 * r) / 0.587;

	*R = clamp(r + 128);
	*G = clamp(g + 128);
	*B = clamp(b + 128);
}
void test(void);
/* decode sos */
__IPGUI_API__ int ipgui_jpeg_decode(jhdr_t * jhdr, byte * code_stream)
{
    int code = 0;
    int symbol;
    byte code_len;

    byte_stream_t byte_stream = {.data = (*code_stream)};
    byte_stream.offset = 7;

    byte code_len_inc = 8;

    byte skip_zero;

    byte l;
		struct idct matL[4], matCr, matCb, temp;

    int lum_loop_cnt = 4;
    int cr_loop_cnt = 1;
    int cb_loop_cnt = 1;

    plat_memset(&matL, 0, sizeof(struct idct) * 4);
    plat_memset(&matCr, 0, sizeof(matCr));
    plat_memset(&matCb, 0, sizeof(matCb));

    float last_lum_dc_coefficient = 0;
    float last_cb_dc_coefficient = 0;
    float last_cr_dc_coefficient = 0;
    
    /* splice image to MCU */
    volatile int xx = ((jhdr->width + 7) >> 4);
    volatile int yy = ((jhdr->height + 7) >> 4);

    int xx_idx, yy_idx;

    int wtf_happened = 0;

    int ctrol_inc = 0;

    for(yy_idx = 0; yy_idx < yy; yy_idx ++)
    {
        for( xx_idx = 0; xx_idx < xx; xx_idx ++ )
        {
            plat_memset(&matL, 0, sizeof(struct idct) * 4);
            plat_memset(&matCr, 0, sizeof(struct idct));
            plat_memset(&matCb, 0, sizeof(struct idct));
            for(int i = 0,l = 0; i < lum_loop_cnt; l = 0, i++)
            {
                code_len_inc=8;
                skip_zero = 0;
                /* 先取直流分量 */
                while(l < 1)
                {
                    while(1){
                        peek_nbits(&byte_stream, code_len_inc, code_stream, &code,0);
                        if( -1 != (symbol = (code_len_inc > 8) ? (ipgui_jcode_match_slow( code, &jhdr->huff_tbl_dc[0], &code_len )) : (ipgui_jcode_match( code, &jhdr->huff_tbl_dc[0], &code_len ))) )
                        {
                            if( symbol == 0 )
                            {
                                matL[i].base[l] = (code & (1 << ((symbol & 0x0f) - 1))) ? (float)(code) : (float)(-((~code) & (gen_mask(symbol & 0x0f))));

                                /* restore dc_coefficient based on DPCM */
                                matL[i].base[l ++] += last_lum_dc_coefficient;
                                last_lum_dc_coefficient = matL[i].base[l - 1];
                                break;
                            }
                            break;
                        }
                        code_len_inc ++;
                    }
                    if( symbol == 0 )
                    {
                        plat_printf("the end of block \r\n");
                        get_nbits_shift(&byte_stream, code_len, &code_stream, &code);
                        break;
                    }
                    get_nbits_shift(&byte_stream, code_len, &code_stream, &code);
                    get_nbits_shift(&byte_stream, symbol & 0x0f, &code_stream, &code);
                    matL[i].base[l] = (code & (1 << ((symbol & 0x0f) - 1))) ? (float)(code) : (float)(-((~code) & (gen_mask(symbol & 0x0f))));

                    /* restore dc_coefficient based on DPCM */
                    matL[i].base[l ++] += last_lum_dc_coefficient;
                    last_lum_dc_coefficient = matL[i].base[l - 1];
                }
                    if( symbol == 0 ) 
                        continue;

                /* 取交流分量 */
                while(l < 64)
                {
                    code_len_inc = 8;
                    skip_zero = 0;
                    while(1){
                        peek_nbits(&byte_stream, code_len_inc, code_stream, &code, wtf_happened);
                        if( ( code == 0xff ) && (byte_stream.offset == 7) )
                        {
                            wtf_happened = 1;
                            ctrol_inc = 1;
                            code_len_inc ++;
                            continue;
                        }
                        if( -1 != (symbol = (code_len_inc > 8) ? (ipgui_jcode_match_slow( code, &jhdr->huff_tbl_ac[0], &code_len )) : (ipgui_jcode_match( code, &jhdr->huff_tbl_ac[0], &code_len ))) )
                        {
                            if( symbol == 0xf0 )
                            {
                                skip_zero += 16;
                                get_nbits_shift(&byte_stream, code_len_inc, &code_stream, &code);
                                continue;
                            }
                            wtf_happened = 0;
                            break;
                        }
                        code_len_inc ++; 
                    }
                    if( symbol == 0 )
                    {
                        plat_printf("the end of block \r\n");
                        get_nbits_shift(&byte_stream, wtf_happened * 8 + code_len, &code_stream, &code);
                        break;
                        //EOB
                    }
                    skip_zero += (symbol & 0xf0) >> 4;
                    get_nbits_shift_and_skip(&byte_stream, code_len, &code_stream, &code , ctrol_inc);
                    ctrol_inc = 0;
                    get_nbits_shift(&byte_stream, symbol & 0x0f, &code_stream, &code);
                    matL[i].base[l += skip_zero] = (code & (1 << ((symbol & 0x0f) - 1))) ? (float)(code) : (float)(-((~code) & (gen_mask(symbol & 0x0f))));
                    l ++;
                }
            }

            //Cb 分量
            for(int i = 0,l = 0; i < cb_loop_cnt; l = 0, i++)
            {
                code_len_inc=8;
                skip_zero = 0;
                /* 先取直流分量 */
                while(l < 1)
                {
                    while(1){
                        peek_nbits(&byte_stream, code_len_inc, code_stream, &code,0);
                        if( -1 != (symbol = (code_len_inc > 8) ? (ipgui_jcode_match_slow( code, &jhdr->huff_tbl_dc[1], &code_len )) : (ipgui_jcode_match( code, &jhdr->huff_tbl_dc[1], &code_len ))) )
                        {
                            if( symbol == 0 )
                            {
                                matCb.base[l] = (code & (1 << ((symbol & 0x0f) - 1))) ? (float)(code) : (float)(-((~code) & (gen_mask(symbol & 0x0f))));

                                /* restore dc_coefficient based on DPCM */
                                matCb.base[l ++] += last_cb_dc_coefficient;
                                last_cb_dc_coefficient = matCb.base[l - 1];
                                break;
                            }
                            break;
                        }
                        code_len_inc ++;
                    }
                    if( symbol == 0 )
                    {
                        plat_printf("the end of block \r\n");
                        get_nbits_shift(&byte_stream, code_len, &code_stream, &code);
                        break;
                    }
                    get_nbits_shift(&byte_stream, code_len, &code_stream, &code);
                    get_nbits_shift(&byte_stream, symbol & 0x0f, &code_stream, &code);
                    matCb.base[l] = (code & (1 << ((symbol & 0x0f) - 1))) ? (float)(code) : (float)(-((~code) & (gen_mask(symbol & 0x0f))));

                    /* restore dc_coefficient based on DPCM */
                    matCb.base[l ++] += last_cb_dc_coefficient;
                    last_cb_dc_coefficient = matCb.base[l - 1];
                }
                    if( symbol == 0 ) 
                        continue;

                /* 取交流分量 */
                while(l < 64)
                {
                    code_len_inc = 8;
                    skip_zero = 0;
                    while(1){
                        peek_nbits(&byte_stream, code_len_inc, code_stream, &code, wtf_happened);
                        if( ( code == 0xff ) && (byte_stream.offset == 7) )
                        {
                            wtf_happened = 1;
                            ctrol_inc = 1;
                            code_len_inc ++;
                            continue;
                        }
                        if( -1 != (symbol = (code_len_inc > 8) ? (ipgui_jcode_match_slow( code, &jhdr->huff_tbl_ac[1], &code_len )) : (ipgui_jcode_match( code, &jhdr->huff_tbl_ac[1], &code_len ))) )
                        {
                            if( symbol == 0xf0 )
                            {
                                skip_zero += 16;
                                get_nbits_shift(&byte_stream, code_len_inc, &code_stream, &code);
                                continue;
                            }
                            wtf_happened = 0;
                            break;
                        }
                        code_len_inc ++; 
                    }
                    if( symbol == 0 )
                    {
                        plat_printf("the end of block \r\n");
                        get_nbits_shift(&byte_stream, wtf_happened * 8 + code_len, &code_stream, &code);
                        break;
                        //EOB
                    }
                    skip_zero += (symbol & 0xf0) >> 4;
                    get_nbits_shift_and_skip(&byte_stream, code_len, &code_stream, &code , ctrol_inc);
                    ctrol_inc = 0;
                    get_nbits_shift(&byte_stream, symbol & 0x0f, &code_stream, &code);
                    matCb.base[l += skip_zero] = (code & (1 << ((symbol & 0x0f) - 1))) ? (float)(code) : (float)(-((~code) & (gen_mask(symbol & 0x0f))));
                    l ++;
                }
            }

            //Cr 分量
            for(int i = 0,l = 0; i < cr_loop_cnt; l = 0, i++)
            {
                                code_len_inc=8;
                skip_zero = 0;
                /* 先取直流分量 */
                while(l < 1)
                {
                    while(1){
                        peek_nbits(&byte_stream, code_len_inc, code_stream, &code,0);
                        if( -1 != (symbol = (code_len_inc > 8) ? (ipgui_jcode_match_slow( code, &jhdr->huff_tbl_dc[1], &code_len )) : (ipgui_jcode_match( code, &jhdr->huff_tbl_dc[1], &code_len ))) )
                        {
                            if( symbol == 0 )
                            {
                                matCr.base[l] = (code & (1 << ((symbol & 0x0f) - 1))) ? (float)(code) : (float)(-((~code) & (gen_mask(symbol & 0x0f))));

                                /* restore dc_coefficient based on DPCM */
                                matCr.base[l ++] += last_cr_dc_coefficient;
                                last_cr_dc_coefficient = matCr.base[l - 1];
                                break;
                            }
                            break;
                        }
                        code_len_inc ++;
                    }
                    if( symbol == 0 )
                    {
                        plat_printf("the end of block \r\n");
                        get_nbits_shift(&byte_stream, code_len, &code_stream, &code);
                        break;
                    }
                    get_nbits_shift(&byte_stream, code_len, &code_stream, &code);
                    get_nbits_shift(&byte_stream, symbol & 0x0f, &code_stream, &code);
                    matCr.base[l] = (code & (1 << ((symbol & 0x0f) - 1))) ? (float)(code) : (float)(-((~code) & (gen_mask(symbol & 0x0f))));

                    /* restore dc_coefficient based on DPCM */
                    matCr.base[l ++] += last_cr_dc_coefficient;
                    last_cr_dc_coefficient = matCr.base[l - 1];
                }
                    if( symbol == 0 ) 
                        continue;

                /* 取交流分量 */
                while(l < 64)
                {
                    code_len_inc = 8;
                    skip_zero = 0;
                    while(1){
                        peek_nbits(&byte_stream, code_len_inc, code_stream, &code, wtf_happened);
                        if( ( code == 0xff ) && (byte_stream.offset == 7) )
                        {
                            wtf_happened = 1;
                            ctrol_inc = 1;
                            code_len_inc ++;
                            continue;
                        }
                        if( -1 != (symbol = (code_len_inc > 8) ? (ipgui_jcode_match_slow( code, &jhdr->huff_tbl_ac[1], &code_len )) : (ipgui_jcode_match( code, &jhdr->huff_tbl_ac[1], &code_len ))) )
                        {
                            if( symbol == 0xf0 )
                            {
                                skip_zero += 16;
                                get_nbits_shift(&byte_stream, code_len_inc, &code_stream, &code);
                                continue;
                            }
                            wtf_happened = 0;
                            break;
                        }
                        code_len_inc ++; 
                    }
                    if( symbol == 0 )
                    {
                        plat_printf("the end of block \r\n");
                        get_nbits_shift(&byte_stream, wtf_happened * 8 + code_len, &code_stream, &code);
                        break;
                        //EOB
                    }
                    skip_zero += (symbol & 0xf0) >> 4;
                    get_nbits_shift_and_skip(&byte_stream, code_len, &code_stream, &code , ctrol_inc);
                    ctrol_inc = 0;
                    get_nbits_shift(&byte_stream, symbol & 0x0f, &code_stream, &code);
                    matCr.base[l += skip_zero] = (code & (1 << ((symbol & 0x0f) - 1))) ? (float)(code) : (float)(-((~code) & (gen_mask(symbol & 0x0f))));
                    l ++;
                }
            }

            int _x = 100;
            int _y = 100;

            struct pixel_layout{
                int k, x ,  y;
            } mcu_lay[4] = { [0] = {.k = 0, .x = 0, .y = 0} , [1] = {.k = 1, .x = 8, .y = 0}, [2] = {.k = 2, .x = 0, .y = 8}, [3] = {.k = 3, .x = 8, .y = 8} };

            for(int k = 0; k < lum_loop_cnt; k++)
            {
                memcpy(&temp, &matL[k], sizeof(struct idct));
                /* 反量化 */
                /* 先做zigzag扫描 */
                for(int i = 0; i < 64; i ++)
                {
                    matL[k].base[zigZagMap[i]] = temp.base[i];
                    //matL[k].base[i] = temp.base[zigZagMap[i]];
                }
                /* 再将对应位置相乘 */
                for(int i = 0; i < 64; i ++)
                {
                    matL[k].base[i] *= jhdr->qtable[0].quantization_table[i];
                }

                for (int i = 0; i < MAT_SIZE; i++)
                    for (int j = 0; j < MAT_SIZE; j++)
                        DctMap[i][j] = matL[k].base[i * 8 + j];
                
                /* 2d idct */
                IDCT2(&matL[k].base[0]);

                byte teste[64];
                uint unPixelColor;
                for (int i = 0; i < 64; i ++)
                {
                    /* code */
                    teste[i] = clamp(matL[k].base[i] + 130);
                }
                
                for( uint i = 0; i < 64; i ++ )
                {
                    unPixelColor = teste[i] << 16 | teste[i] << 8 | teste[i];
                    ipgui_put_pixel(i % 8 + mcu_lay[k].x + xx_idx * 16 + _x, i / 8 + mcu_lay[k].y + yy_idx * 16 + _y, unPixelColor);
                }
                FlushBatchDraw();
            }
        }
    }

    return 0;
}

__IPGUI_STATIC__ int ipgui_jmarker_read(jhdr_t * jhdr, const byte * d_marker, int * byte_off)
{
    uint16_t length;
    byte marker_id = *(d_marker + 1);
    uint offset = 0;
    uint idx;
    InitTransMat();
    if( * d_marker != 0xff ){
        return -1;
    }

    /* get marker length */
    length = (d_marker[2] << 8) | d_marker[3];
    (* byte_off) += (length + 2);

    if(( marker_id >= APP0 ) && ( marker_id <= APP15 ))
        ;/* just loop of offset shift */
    else if( marker_id == DQT ){
          length -= 2;
          /* parse DQT */
          while( length > 0 ){
              byte quantization_table_id = d_marker[4] & 0x0f;
              byte table_unit_size = d_marker[4] >> 4;
              length -= 1;
              if( quantization_table_id > 3 ){
                  plat_printf("err: invalid DQT!\r\n");
                  return -1;
              }
              /* record quantization table */
              if( table_unit_size == 0 ){
                  plat_printf("DQT table idx = %d :\r\n", quantization_table_id);
                  for(idx = 0; idx < 64; idx ++){
                      if( idx % 8 == 0 && idx ) plat_printf("\r\n");
                      jhdr->qtable[quantization_table_id].quantization_table[zigZagMap[idx]] = d_marker[5 + idx];
                      plat_printf("%4d ", jhdr->qtable[quantization_table_id].quantization_table[idx]);
                  }
                  plat_printf("\r\n");plat_printf("\r\n");
              }else{
                  for(idx = 0; idx < 64; idx ++){
                      jhdr->qtable[quantization_table_id].quantization_table[zigZagMap[idx]] = d_marker[5 + idx * 2] << 8 | d_marker[5 + idx * 2 + 1];
                  }
              }
              length -= ((table_unit_size == 0) ? 64 : 128);
          }
    }
    else if( marker_id == SOF0 ){
        byte precision = d_marker[4];
        jhdr->height = d_marker[5] << 8 | d_marker[6];
        jhdr->width = d_marker[7] << 8 | d_marker[8];
        if(( precision != 8 ) || ( jhdr->height == 0 ) || ( jhdr->width == 0 ))
            return -1;
        jhdr->num_components = d_marker[9];
        /* color components(颜色分量数), JFIF use YCbCr, instead, 3 components */
        if(( jhdr->num_components == 0 ) || ( jhdr->num_components == 4 ))
        {
            plat_printf("err: num of components invlid!\r\n");
            return -1;
        }
        offset += 10;
        for(idx = 0; idx < jhdr->num_components; idx ++){
            byte componentID = d_marker[offset ++];
            if(( componentID == 4 ) || ( componentID == 5 ))
            {
                plat_printf("err: CMYK and YIQ color mode not support!\r\n");
                return -1;
            }
            if(( componentID == 0 ) || ( componentID > 3 ))
            {
                plat_printf("err: invalid componentID!\r\n");
                return -1;
            }
            jcr_cmpn_t * jcr_cmpn = &jhdr->jcr_cmpn[componentID - 1];
            byte samp_fac = d_marker[offset ++];
            jcr_cmpn->horizontal_sampling_factor = samp_fac >> 4;
            jcr_cmpn->vertical_sampling_factor = samp_fac & 0x0f;
            jcr_cmpn->quantization_table_id = d_marker[offset ++];
        }
    }
    else if( marker_id == DRI ){
        offset += 4;
        jhdr->restart_interval = d_marker[offset] << 8 | d_marker[offset + 1];
        plat_printf("err: jpeg with restart interval not supported!\r\n");
    }
    else if( marker_id == DHT ){
        offset += 4;
        byte table_info = d_marker[offset ++];
        byte table_id = table_info & 0x0f;
        byte table_class = table_info >> 4;
        if( table_id > 3 ){
            return -1;
        }

        huff_tbl_t * huff_tbl = (table_class == 0) ? (&jhdr->huff_tbl_dc[table_id]) : (&jhdr->huff_tbl_ac[table_id]);
        /* generate huffman lookup table */
        ipgui_jhuff_lookup_tbl_gen(jhdr, huff_tbl, (byte *)&d_marker[offset]);
        return 0;
    }
    else if( marker_id == SOS){
        plat_printf("skip sos marker.\r\n");
    }
    else{
        plat_printf("err: invalid marker!\r\n");
        return -1;
    }

    return 0;
}

/* parse jpg header */
__IPGUI_API__ jhdr_t * ipgui_jread(const byte * file)
{
    int byte_off = 0;
    jhdr_t * pstJhdr = (jhdr_t *)kmalloc(sizeof(jhdr_t));

    if( pstJhdr == NULL )
        return NULL;

    if(file[byte_off] != 0xff || file[byte_off + 1] != 0xd8){
        kfree(pstJhdr);
        plat_printf("invalid jpg file!\r\n");
        return NULL;
    }

    pstJhdr->valid = 1;
    byte_off += 2;
    /* appN marker immediately follows SOI,there are actually 16 app markers */
    /* such as photoshop's private data */
    while(pstJhdr->valid == 1)
    {
        if( byte_off == 0x26f )
            break;
        if(0 > ipgui_jmarker_read(pstJhdr, &file[byte_off], &byte_off))
            pstJhdr->valid = 0;
    }

    if( byte_off != 0x26f )
    {
        printf("error! \r\n");
        return NULL;
    }

    ipgui_jpeg_decode(pstJhdr, (byte *)&file[byte_off]);

    return pstJhdr;
}