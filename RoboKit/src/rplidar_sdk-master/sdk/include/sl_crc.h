/*
* Slamtec LIDAR SDK
*
* sl_crc.h
*
* Copyright (c) 2020 Shanghai Slamtec Co., Ltd.
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
// 本文件声明了 CRC32 校验函数。
//
// 说明：RPLIDAR 通讯协议有两套校验机制：
//   1) 普通请求/应答报文使用“按字节异或”校验和。
//      【通信协议 LR001 P.6】请求报文校验和公式：
//        checksum = 0 ⨁ 0xA5 ⨁ CmdType ⨁ PayloadSize ⨁ Payload[0] ⨁ … ⨁ Payload[n]
//      【通信协议 LR001 P.20】EXPRESS_SCAN 报文的 ChkSum 亦为按字节依次异或得到。
//   2) HQ 扫描模式(ANS_TYPE_MEASUREMENT_HQ=0x83)的应答报文使用 CRC32 校验
//      （见 sl_lidar_cmd.h 中 sl_lidar_response_hq_capsule_measurement_nodes_t.crc32）。
//      CRC32 采用 IEEE 802.3 标准多项式 0x4C11DB7（见 sl_crc.cpp 的 init 调用）。
// ===================================================================

#pragma once

// 引入命令定义，获得 sl_u8/sl_u32/sl_result 等基本类型与 HQ 报文结构等定义。
#include "sl_lidar_cmd.h"

// CRC32 校验命名空间。HQ 扫描模式应答报文使用此处的 CRC32 进行校验。
namespace sl {namespace crc32 {
    // 位反序(reflect)：将 input 的低 bw 位按二进制位反序输出。
    // 用于将 CRC 多项式按位反序，以适应“反向(reflected)”CRC 算法实现。
    sl_u32 bitrev(sl_u32 input, sl_u16 bw);//reflect
    // 初始化：按给定多项式 poly 生成 256 项的 CRC32 查找表(table)。
    // HQ 模式使用 IEEE 802.3 标准多项式 0x4C11DB7。
    void init(sl_u32 poly); // table init
    // 计算：以 crc 为初值，对 input 指向的 len 字节数据计算 CRC32。
    // 采用查表法逐字节处理，并对数据做 4 字节对齐的零填充。
    sl_u32 cal(sl_u32 crc, void* input, sl_u16 len);
    // 对外接口：对 ptr 指向的 len 字节数据计算 CRC32 结果。
    // 内部会按需(仅一次)以多项式 0x4C11DB7 初始化查找表，初值为 0xFFFFFFFF。
    sl_result getResult(sl_u8 *ptr, sl_u32 len);
}}
