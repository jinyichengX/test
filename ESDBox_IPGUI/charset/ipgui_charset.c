// #include "ipgui_charset.h"
// #include "ipgui_debug.h"

// char ipgui_is_ascii(const char c)
// {
//     return ((c >= 0) && (c <= 0x7f));
// }

// /* 二分查表 */
// /* map：表起始地址 */
// /* tbl_max：单元数量即表共多少行 */
// /* uni_size：每单元大小表每一行字节数*/
// /* uni_off：从每单元偏移多少开始查*/
// /* len：被查单元大小，就是一次查几个字节*/
// /* mode：查询的表是大端还是小端 */
// static int ipgui_charset_mapping_table_search(unsigned int tar, unsigned char len, const unsigned char * map, unsigned int tbl_max, unsigned char uni_size, unsigned char uni_off)
// {
//     int b1 = 0;
//     int b2 = tbl_max - 1;
//     unsigned int index, k = 0;
//     unsigned int b_off, idx_v;

//     if(uni_off + len > uni_size) 
//         return -1;
//     if((len < 1) || (len > 4))
//         return -2;

//     while(b1 <= b2)
//     {
//         if(k > 32)
//             return -3;
//         k ++;

//         index = b1 + (b2 - b1) / 2;

//         b_off = index * uni_size + uni_off;
//         //idx_v = b2val_table[len]( (unsigned char *)map + b_off );/* 计算索引值 */
//         if(idx_v < tar) 
//             b1 = index + 1;
//         else if(idx_v > tar)
//             b2 = index - 1;
//         if(idx_v == tar)
//             return index;
//     }
//     return -4;
// }

// /* if len < ret, then false */
// char ipgui_unicode2utf8(unsigned int uni, void * ret_buf, int len)
// {
//     unsigned char *p = ret_buf;
//     if((ret_buf == NULL) || (len < 1))
//         return -1;

//     if( (uni > 0x00000000) && (uni <= 0x0000007f) )
//     {
//         *p = uni & 0x7f;
//         return 1;
//     }
//     else if( (uni >= 0x00000080) && (uni <= 0x000007ff) )
//     {
// 		*p = (uni >> 6) & 0x1f;
//         *p = *p | 0xc0;
// 		*(p+1) = uni & 0x3f;
//         *(p+1) = *(p + 1) | 0x80;
//         return 2;
//     }
//     else if( (uni >= 0x00000800) && (uni <= 0x0000ffff) )
//     {
// 		*p = (uni >> 12) & 0x0f;
//         *p = *p | 0xe0;
//         *(p+1) =  (uni >> 6) & 0x3f;
//         *(p+1) = *(p + 1) | 0x80;
// 		*(p+2) = uni & 0x3f;
//         *(p+2) = *(p + 2) | 0x80;
//         return 3;
//     }
//     else if( (uni >= 0x00010000) && (uni <= 0x0010ffff) )
//     {
// 		*p = (uni >> 18) & 0x07;
//         *p = *p | 0xf0;
// 		*(p+1) = (uni >> 12) & 0x3f;
//         *(p+1) = *(p + 1) | 0x80;
//         *(p+2) = (uni >> 6) & 0x3f;
//         *(p+2) = *(p + 2) | 0x80;
//         *(p+3) = uni & 0x3f;
//         *(p+3) = *(p + 3) | 0x80;
//         return 4;
//     }
//     else
//         return 0;
// }

// char ipgui_utf82unicode(unsigned char * unic, unsigned int * uecd)
// {
//     * uecd = 0;
//     unsigned char * ufst = unic;
//     unsigned char * rp = (unsigned char *)uecd;
//     /* 解码逻辑看字符编码相关的逻辑图 */
//     /* 判断第一个字节是否可能为Utf-8编码 */
//     if(*ufst >= 0xF0)
//     {
//         * rp = ( ( *(ufst + 2) << 6 ) & 0xc0 ) | ( *(ufst + 3) & 0x3f );
// 		*(rp+1) = ((*(ufst + 2) >> 2) & 0x0f) | ( (*(ufst + 1) << 4) & 0xf0 );
//         *(rp+2) = ( (*ufst << 2) & 0x1c ) | ( (*(ufst + 1) >> 4) & 0x03 );
// 		return 4;
//     }
//     else if(*ufst >= 0xe0)
//     {
//         * rp = ( ( *(ufst + 1) << 6 ) & 0xc0 ) | ( *(ufst + 2) & 0x3f );
//         *(rp+1) = ((*(ufst + 1) >> 2) & 0x0f) | (*ufst << 4);
// 		return 2;
//     }
//     else if(*ufst >= 0xc0)
//     {
//         *(rp + 1) = (*ufst >> 2) & 0x07;
//         *rp = ( (*ufst << 6) & 0xc0 ) | ( *(ufst + 1) & 0x3f );
//         return 2;
//     }
//     else if(*ufst <= 0x7f)
//     {
//         *rp = *ufst;
//         return 1;
//     }
//     /* 这里应该加一个逆向的过程，防止错码 */
//     else return 0;
// }

// // __IPGUI_API__ ipgui_charset_is