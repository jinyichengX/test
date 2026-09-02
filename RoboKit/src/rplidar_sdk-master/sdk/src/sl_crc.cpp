/*
 * Slamtec LIDAR SDK
 *
 *  Copyright (c) 2014 - 2020 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */
 /*
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  */

// ===================================================================
// 本文件实现 CRC32 校验函数，用于 HQ 扫描模式应答报文的校验。
//
// 背景：RPLIDAR 通讯协议的普通报文使用“按字节异或”校验和，
//   【通信协议 LR001 P.6】请求报文校验和：
//     checksum = 0 ⨁ 0xA5 ⨁ CmdType ⨁ PayloadSize ⨁ Payload[0] ⨁ … ⨁ Payload[n]
// 而 HQ 扫描模式(应答数据类型 SL_LIDAR_ANS_TYPE_MEASUREMENT_HQ=0x83)的应答报文
//   使用更强的 CRC32 校验。HQ 报文结构 sl_lidar_response_hq_capsule_measurement_nodes_t
//   末尾的 crc32 字段即由本文件 getResult 计算并校验。
// 本实现采用 IEEE 802.3 标准 CRC32，生成多项式为 0x4C11DB7。
// ===================================================================

#include "sl_crc.h"  

namespace sl {namespace crc32 {
    
    // CRC32 查找表（256 项），由 init() 按多项式生成，cal() 查表逐字节计算。
    static sl_u32 table[256];//crc32_table

    // 位反序(reflect)：将 input 的低 bw 位二进制位反序后返回。
    // 用于将标准多项式 0x4C11DB7 反序为 reflected 多项式，以配合
    // “反向(reflected)”CRC 算法（即 cal 中右移、查表的方向）。
    sl_u32 bitrev(sl_u32 input, sl_u16 bw)
    {
        sl_u16 i;
        sl_u32 var;
        var = 0;
        // 逐位扫描：若 input 当前最低位为1，则在结果中对应最高位处置1。
        for (i = 0; i < bw; i++) {
            if (input & 0x01) {
                var |= 1 << (bw - 1 - i);
            }
            input >>= 1;
        }
        return var;
    }

    // 初始化：按多项式 poly 生成 256 项的 CRC32 查找表 table[]。
    // 使用 reflected 多项式（对 poly 做 32 位反序），与 cal() 的右移方向一致。
    void init(sl_u32 poly)
    {
        sl_u16 i;
        sl_u16 j;
        sl_u32 c;

        // 将多项式按 32 位反序，得到 reflected 多项式。
        poly = bitrev(poly, 32);
        // 为 0~255 每个可能的字节值预计算其 CRC 变换结果。
        for (i = 0; i < 256; i++) {
            c = i;
            // 每个字节做 8 次移位/异或，得到该字节对应的表项。
            for (j = 0; j < 8; j++) {
                if (c & 1)
                    c = poly ^ (c >> 1);
                else
                    c = c >> 1;
            }
            table[i] = c;
        }
    }

    // 计算：以 crc 为初值，对 input 指向的 len 字节数据计算 CRC32。
    // 采用查表法：每取一字节，用 (crc^byte) 作为索引查表，
    //   新 crc = (crc>>8) ^ table[index]。最后做 4 字节零填充并对结果取反。
    sl_u32 cal(sl_u32 crc, void* input, sl_u16 len)
    {
        sl_u16 i;
        sl_u8 index;
        sl_u8* pch;
        pch = (unsigned char*)input;
        // 计算需要补多少个零字节才能使总长度对齐到 4 字节边界。
        // HQ 报文要求按 4 字节对齐进行 CRC 计算。
        sl_u8 leftBytes = 4 - (len & 0x3);

        // 对实际数据逐字节查表计算。
        for (i = 0; i < len; i++) {
            index = (unsigned char)(crc^*pch);
            crc = (crc >> 8) ^ table[index];
            pch++;
        }

        // 零填充：对不足 4 字节对齐的部分补零字节继续计算。
        for (i = 0; i < leftBytes; i++) {//zero padding
            index = (unsigned char)(crc ^ 0);
            crc = (crc >> 8) ^ table[index];
        }
        // 最终结果取反（与 0xFFFFFFFF 异或），符合 IEEE 802.3 CRC32 规范。
        return crc ^ 0xffffffff;
    }

    // 对外接口：对 ptr 指向的 len 字节数据计算 CRC32。
    // 仅在首次调用时以 IEEE 802.3 标准多项式 0x4C11DB7 初始化查找表，
    // 后续调用直接复用。初值固定为 0xFFFFFFFF。
    // HQ 扫描报文(sl_lidar_response_hq_capsule_measurement_nodes_t)的 crc32
    // 字段即用本函数对报文（除末尾 crc32 字段本身外）计算得到。
    sl_result getResult(sl_u8 *ptr, sl_u32 len) 
    {
        // 静态标志，保证查找表只初始化一次。
        static sl_u8 tmp;
        if (tmp != 1) {
            // 使用 IEEE 802.3 标准 CRC32 多项式 0x4C11DB7 初始化查找表。
            init(0x4C11DB7);
            tmp = 1;
        }

        // 初值 0xFFFFFFFF，对 ptr 指向的 len 字节计算 CRC32。
        return cal(0xFFFFFFFF, ptr, len);
    }
}}
