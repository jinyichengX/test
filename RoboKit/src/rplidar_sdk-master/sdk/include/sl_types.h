/*
* Slamtec LIDAR SDK
*
* sl_types.h
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
// 本文件定义了 SDK 使用的平台无关的基本数据类型和错误码。
// 基本数据类型对应通信协议 PDF 第36页定义的 u8/u16/u32/u64、
// s8/s16/s32/s64 等基本数据类型，所有数据采用小字端(little-endian)存储。
// 【SDK手册 LR002 P.12】sl_types.h: 平台无关的结构和常量定义。
// 【通信协议 LR001 P.36】基本数据类型定义：
//   u8/u16/u32/u64(无符号整数)、s8/s16/s32/s64(有符号整数)、String、Float、Double。
//   所有数据采用小字端(little-endian)存储。
// ===================================================================

#pragma once

// C++ 环境下使用 <cstdint> 提供的定宽整数类型(int8_t/uint8_t 等)。
// C 环境下使用 <stdint.h> 提供的定宽整数类型。
#ifdef __cplusplus
#include <cstdint>

// 类型定义宏：在 C++ 下将 std::IntType 重命名为 NewType。
// 例如 SL_DEFINE_TYPE(int8_t, sl_s8) 展开为 typedef std::int8_t sl_s8。
#define SL_DEFINE_TYPE(IntType, NewType)    typedef std::IntType NewType
#else
#include <stdint.h>

// 类型定义宏：在 C 下将 IntType 重命名为 NewType。
#define SL_DEFINE_TYPE(IntType, NewType)    typedef IntType NewType
#endif

// 整数类型对定义宏：一次性定义同一字长的有符号与无符号类型。
// 展开为两个 typedef：sl_s##Bits(有符号) 与 sl_u##Bits(无符号)。
// 对应协议文档第36页定义的 s8/s16/s32/s64 与 u8/u16/u32/u64。
// 【通信协议 LR001 P.36】u8/u16/u32/u64: 无符号整数，字长由数字后缀表示(比特位数量)，
//   长度 1/2/4/8 字节；s8/s16/s32/s64: 有符号整数，长度 1/2/4/8 字节。
#define SL_DEFINE_INT_TYPE(Bits) \
    SL_DEFINE_TYPE(int ## Bits ## _t, sl_s ## Bits); \
    SL_DEFINE_TYPE(uint ## Bits ## _t, sl_u ## Bits); \

// 定义 8 位整数类型：sl_s8(有符号)、sl_u8(无符号)。长度 1 字节。
SL_DEFINE_INT_TYPE(8)
// 定义 16 位整数类型：sl_s16(有符号)、sl_u16(无符号)。长度 2 字节。
SL_DEFINE_INT_TYPE(16)
// 定义 32 位整数类型：sl_s32(有符号)、sl_u32(无符号)。长度 4 字节。
SL_DEFINE_INT_TYPE(32)
// 定义 64 位整数类型：sl_s64(有符号)、sl_u64(无符号)。长度 8 字节。
SL_DEFINE_INT_TYPE(64)

// 非 GCC 编译器下，将 __attribute__ 宏定义为空，以保证使用 __attribute__((packed))
// 的协议结构体在 MSVC 等非 GCC 编译器下也能编译通过（实际对齐由 #pragma pack 控制）。
#if !defined(__GNUC__) && !defined(__attribute__)
#   define __attribute__(x)
#endif

// 平台相关的“机器字长”类型 sl_word_size_t，用于表示指针/句柄大小的整数。
// 64 位 Windows 下为 64 位无符号整数。
#ifdef WIN64
typedef sl_u64          sl_word_size_t;
// 32 位 Windows 下为 32 位无符号整数。
#elif defined(WIN32)
typedef sl_u32          sl_word_size_t;
// GCC 平台下使用 unsigned long（随平台字长变化）。
#elif defined(__GNUC__)
typedef unsigned long   sl_word_size_t;
// IAR ARM 编译器下为 32 位无符号整数。
#elif defined(__ICCARM__)
typedef sl_u32          sl_word_size_t;
#endif

// SDK 操作结果类型。用 32 位无符号整数表示，最高位(bit31)为失败标志位，
// 低 15 位为具体错误码。0 表示成功。
typedef uint32_t sl_result;

// 操作成功。返回值为 0 时表示操作成功完成。
#define SL_RESULT_OK                     (sl_result)0
// 失败标志位(bit31)。所有失败类结果的最高位均置1，成功类结果该位为0。
#define SL_RESULT_FAIL_BIT               (sl_result)0x80000000
// 操作已完成的非错误状态。例如扫描已停止等正常结束情况。
#define SL_RESULT_ALREADY_DONE           (sl_result)0x20
// 数据无效错误。
#define SL_RESULT_INVALID_DATA           (sl_result)(0x8000 | SL_RESULT_FAIL_BIT)
// 操作失败错误。
#define SL_RESULT_OPERATION_FAIL         (sl_result)(0x8001 | SL_RESULT_FAIL_BIT)
// 操作超时错误。例如 grabScanDataHq 等待完整一圈扫描数据超时。
// 对应 SDK 手册所述：如果一圈完整的扫描测距序列尚未接收完毕，该函数将进行等待，
// 直到获得了完整的扫描数据或者超过了等待时间。
// 【SDK手册 LR002 P.15】grabScanDataHq() 将始终返回一个最新的完整的360度的扫描测距序列。
#define SL_RESULT_OPERATION_TIMEOUT      (sl_result)(0x8002 | SL_RESULT_FAIL_BIT)
// 操作被停止错误。例如 stop() 导致的后台扫描线程退出。
#define SL_RESULT_OPERATION_STOP         (sl_result)(0x8003 | SL_RESULT_FAIL_BIT)
// 操作不被支持错误。例如固件版本不支持 EXPRESS_SCAN 模式时 startScanExpress 失败。
// 【SDK手册 LR002 P.14】如果 RPLIDAR 固件不支持 ExpressScan 模式，该函数将执行失败。
#define SL_RESULT_OPERATION_NOT_SUPPORT  (sl_result)(0x8004 | SL_RESULT_FAIL_BIT)
// 数据格式不被支持错误。
#define SL_RESULT_FORMAT_NOT_SUPPORT     (sl_result)(0x8005 | SL_RESULT_FAIL_BIT)
// 内存不足错误。
#define SL_RESULT_INSUFFICIENT_MEMORY    (sl_result)(0x8006 | SL_RESULT_FAIL_BIT)

// 宏：判断操作结果是否为成功（失败标志位为0）。
#define SL_IS_OK(x)    ( ((x) & SL_RESULT_FAIL_BIT) == 0 )
// 宏：判断操作结果是否为失败（失败标志位为1）。
#define SL_IS_FAIL(x)  ( ((x) & SL_RESULT_FAIL_BIT) )
