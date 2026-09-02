/*
* Slamtec LIDAR SDK
*
* sl_lidar_protocol.h
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
// 本文件定义了 RPLIDAR 通讯协议文档中描述的底层相关数据结构和常量。
// 包含请求报文(sl_lidar_cmd_packet_t)和起始应答报文头
// (sl_lidar_ans_header_t)的内存布局，以及协议用到的同步字节等常量。
// 【SDK手册 LR002 P.12】头文件介绍：sl_lidar_protocol.h 定义了
//   RPLIDAR 通讯协议文档中描述的底层相关数据结构和常量定义。
// 通讯协议采用二进制数据报文进行，每个数据报文均具有统一的报头数据格式，
// 字节发送顺序采用小字端(little endian)模式。
// 【通信协议 LR001 P.4】基本通讯模式：与 RPLIDAR 进行的通讯采用非文本形式的
//   二进制数据报文进行，且每个数据报文均具有统一的报头数据格式。
// ===================================================================

#pragma once


// MSVC 编译器下，本文件使用了 C 语言的“零长度数组”（柔性数组 data[0]），
// 这是 GCC 扩展语法，MSVC 默认会发出 C4200 警告。这里临时关闭该警告，
// 以保证 Windows 平台下编译通过。
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4200)
#endif

// 引入平台无关的基本数据类型定义（sl_u8/sl_u16/sl_u32 等），
// 这些类型对应协议文档第36页定义的 u8/u16/u32/u64 等基本数据类型。
// 【通信协议 LR001 P.36】基本数据类型定义：u8/u16/u32/u64、s8/s16/s32/s64、
//   String、Float、Double，所有数据采用小字端(little-endian)存储。
#include "sl_types.h"

// -------------------------------------------------------------------
// 以下为请求报文相关常量
// 【通信协议 LR001 P.6】请求报文格式：
//   起始标志(1byte, 0xA5) + 请求命令(1byte) + 负载长度(1byte)
//   + 请求负载数据(0-255 bytes) + 校验和(1byte)
// -------------------------------------------------------------------

// 请求报文的起始标志（同步字节）。
// 每个请求报文均以固定的 0xA5 作为开始字节，RPLIDAR 将以此识别一个新的
// 请求报文的开头。
// 【通信协议 LR001 P.6】每个请求报文均以固定的 0xA5 作为开始字节，
//   RPLIDAR 将以此识别一个新的请求报文的开头。
#define SL_LIDAR_CMD_SYNC_BYTE              0xA5

// 命令标志位：当该位被置位时，表示当前请求报文带有负载数据。
// 该位与具体的命令码按位或运算后填入请求报文的“请求命令”字段。
// 例如 EXPRESS_SCAN 命令码为 0x02，实际发送的命令字节为 0x82(0x80|0x02)；
// GET_LIDAR_CONF 命令码为 0x04，实际发送字节为 0x84(0x80|0x04)。
// 【通信协议 LR001 P.6】如果该请求命令需要额外附带有其他数据，则请求报文
//   还需要附带一个字节的负载数据长度信息、负载数据本身以及一个字节的
//   校验和作为结尾。
#define SL_LIDAR_CMDFLAG_HAS_PAYLOAD        0x80

// -------------------------------------------------------------------
// 以下为起始应答报文相关常量
// 【通信协议 LR001 P.8】起始应答报文结构：
//   起始标志1(1byte, 0xA5) + 起始标志2(1byte, 0x5A)
//   + 数据应答报文长度(30bits) + 应答模式(2bits) + 数据类型(1byte)
// -------------------------------------------------------------------

// 起始应答报文的起始标志1，固定为 0xA5。
// 【通信协议 LR001 P.8】起始标志为 2 个字节的固定数据：0xA5 0x5A。
//   外部系统可以以此判断起始应答报文的开始部分。
#define SL_LIDAR_ANS_SYNC_BYTE1             0xA5

// 起始应答报文的起始标志2，固定为 0x5A。
// 与 SL_LIDAR_ANS_SYNC_BYTE1 一起构成 2 字节的固定同步序列 0xA5 0x5A，
// 用于外部系统判断一个起始应答报文的开始。
// 【通信协议 LR001 P.8】起始标志为 2 个字节的固定数据：0xA5 0x5A。
#define SL_LIDAR_ANS_SYNC_BYTE2             0x5A

// 应答模式标志位：多次应答模式。
// 起始应答报文中的 2bits 应答模式字段取值如下：
//   0x0 = 单次应答模式，RPLIDAR 只发送一次数据应答报文
//   0x1 = 多次应答模式，RPLIDAR 将会发送一个或者多个应答报文
//   0x2/0x3 = 保留，暂未定义
// 当应答模式为 0x1(LOOP)时，表示当前请求采用“单次请求-多次应答”通讯模式，
// 即扫描测距模式：RPLIDAR 在收到开始扫描请求后，将连续发送多个数据应答
// 报文（每个测距采样点一个报文），直到外部系统发送新的请求为止。
// 【通信协议 LR001 P.5】单次请求-多次应答模式：该通讯模式用于 RPLIDAR
//   进行扫描测距的模式下。外部系统只需要发送单次的请求，并开始连续接收
//   来自 RPLIDAR 的多个应答数据报文。
// 【通信协议 LR001 P.8】应答模式取值：0x0 单次应答模式；0x1 多次应答模式。
#define SL_LIDAR_ANS_PKTFLAG_LOOP           0x1

// 起始应答报文中 size_q30_subtype 字段的“长度”位掩码。
// size_q30_subtype 是一个 32 位字段，低 30 位存放数据应答报文的长度
// （单位：字节），高 2 位存放应答模式（见 SL_LIDAR_ANS_PKTFLAG_LOOP）。
// 该掩码用于从 size_q30_subtype 中取出低 30 位的长度值。
// 【通信协议 LR001 P.8】数据应答报文长度为 30bits 的数据，记录了随后发送
//   的单个数据应答报文的长度。
#define SL_LIDAR_ANS_HEADER_SIZE_MASK       0x3FFFFFFF

// 起始应答报文中 size_q30_subtype 字段的“应答模式”位移位数。
// 应答模式占 size_q30_subtype 的高 2 位（即第 30~31 位），
// 需要将 size_q30_subtype 右移 30 位才能得到应答模式的值。
// 【通信协议 LR001 P.8】2bits 的应答模式字段描述了接下来的数据应答报文
//   的发送模式。
#define SL_LIDAR_ANS_HEADER_SUBTYPE_SHIFT   (30)

// Windows 平台下，使用 #pragma pack(1) 取消结构体的默认对齐填充，
// 使结构体成员按 1 字节边界紧凑排列，确保协议报文的内存布局与线上传输的
// 字节序严格一致（协议要求小字端、无填充字节）。
#if defined(_WIN32)
#pragma pack(1)
#endif



// -------------------------------------------------------------------
// 请求报文结构定义
// 对应通信协议 PDF 第6页“请求报文格式”：
//   起始标志(1byte,0xA5) + 请求命令(1byte) + 负载长度(1byte)
//   + 请求负载数据(0-255bytes) + 校验和(1byte)
// 注意：本结构体仅描述请求报文除“校验和”外的部分（校验和由编解码层
// 计算并附加在报文末尾）。当命令带有负载时，cmd_flag 会被设置
// SL_LIDAR_CMDFLAG_HAS_PAYLOAD(0x80) 标志位。
// 【通信协议 LR001 P.6】请求报文发送格式图表 2-4。
// -------------------------------------------------------------------
typedef struct sl_lidar_cmd_packet_t
{
    // 起始标志，必须填入 SL_LIDAR_CMD_SYNC_BYTE(0xA5)。
    // RPLIDAR 以此识别一个新请求报文的开始。
    // 【通信协议 LR001 P.6】每个请求报文均以固定的 0xA5 作为开始字节。
    sl_u8 syncByte; //must be SL_LIDAR_CMD_SYNC_BYTE

    // 请求命令字段。低 7 位为命令码，最高位为 SL_LIDAR_CMDFLAG_HAS_PAYLOAD
    // 标志（带负载时置1）。例如 STOP=0x25、SCAN=0x20、
    // EXPRESS_SCAN=0x82(0x80|0x02)、GET_LIDAR_CONF=0x84(0x80|0x04)。
    // 【通信协议 LR001 P.6】所有请求报文都必须包含一个字节长度的请求命令字段。
    sl_u8 cmd_flag;

    // 负载数据长度（字节数），范围 0~255。
    // 仅当 cmd_flag 带有 SL_LIDAR_CMDFLAG_HAS_PAYLOAD 标志时本字段有效。
    // 【通信协议 LR001 P.6】负载长度 1byte，负载数据 0-255 bytes。
    sl_u8 size;

    // 负载数据（柔性数组/零长度数组），实际长度由 size 字段决定。
    // 当请求不需要附带数据时（如 STOP/SCAN/GET_INFO），该字段不占用空间。
    // 注意：完整的请求报文末尾还需追加 1 字节校验和，其计算公式为：
    //   checksum = 0 XOR 0xA5 XOR CmdType XOR PayloadSize
    //              XOR Payload[0] XOR ... XOR Payload[n]
    // 【通信协议 LR001 P.6】校验和的值按照如下公式计算得出：
    //   checksum = 0 ⨁ 0xA5 ⨁ CmdType ⨁ PayloadSize ⨁ Payload[0] ⨁ … ⨁ Payload[n]
    sl_u8 data[0];
} __attribute__((packed)) sl_lidar_cmd_packet_t;


// -------------------------------------------------------------------
// 起始应答报文头结构定义
// 对应通信协议 PDF 第8页“起始应答报文结构”图表 2-7：
//   起始标志1(1byte,0xA5) + 起始标志2(1byte,0x5A)
//   + 数据应答报文长度(30bits) + 应答模式(2bits) + 数据类型(1byte)
// 起始应答报文用以描述后续数据应答报文的相关信息（长度、模式、类型），
// 在一次请求/应答的通讯过程中，起始应答报文只会发送一次。
// 【通信协议 LR001 P.7】应答报文分为起始应答报文和数据应答报文两类。
//   如果当前接收到的请求报文需要发送应答报文，则 RPLIDAR 首先发送起始
//   应答报文，随后按照通讯模式，发送一次或者任意多次的数据应答报文。
// -------------------------------------------------------------------
typedef struct sl_lidar_ans_header_t
{
    // 起始标志1，必须为 SL_LIDAR_ANS_SYNC_BYTE1(0xA5)。
    // 外部系统据此判断起始应答报文的开始部分。
    // 【通信协议 LR001 P.8】起始标志为 2 个字节的固定数据：0xA5 0x5A。
    sl_u8  syncByte1; // must be SL_LIDAR_ANS_SYNC_BYTE1

    // 起始标志2，必须为 SL_LIDAR_ANS_SYNC_BYTE2(0x5A)。
    // 与 syncByte1 共同构成 2 字节的固定同步序列。
    // 【通信协议 LR001 P.8】外部系统可以以此判断起始应答报文的开始部分。
    sl_u8  syncByte2; // must be SL_LIDAR_ANS_SYNC_BYTE2

    // 复合字段：低 30 位为数据应答报文长度（字节），高 2 位为应答模式。
    //   - 长度：后续单个数据应答报文的字节数（使用 SL_LIDAR_ANS_HEADER_SIZE_MASK 取低30位）
    //   - 应答模式：使用 (size_q30_subtype >> SL_LIDAR_ANS_HEADER_SUBTYPE_SHIFT) 取高2位
    //     取值 0x0=单次应答；0x1=多次应答（扫描数据用）；0x2/0x3=保留
    // 【通信协议 LR001 P.8】数据应答报文长度为 30bits，应答模式字段为 2bits。
    sl_u32 size_q30_subtype; // see _u32 size:30; _u32 subType:2;

    // 数据类型，1 字节。表示数据应答报文发送内容的类型，与 RPLIDAR 接收
    // 到的请求报文类型相对应。外部系统可通过该字段确定后续数据应答报文
    // 的接收与解析策略。典型取值如：
    //   0x81 = SCAN 命令对应的标准测距数据（5字节/点，见 measurement_node_t）
    //   0x82 = EXPRESS_SCAN 传统版本（84字节/包）
    //   0x83 = EXPRESS_SCAN 扩展版本（132字节/包，Ultra Cabin）
    //   0x84 = EXPRESS_SCAN 扩展版本（132字节/包）
    //   0x85 = EXPRESS_SCAN 密实版本（84字节/包）
    //   0x04 = GET_INFO 设备信息（20字节）
    //   0x06 = GET_HEALTH 健康状态（3字节）
    //   0x15 = GET_SAMPLERATE 测距用时（4字节）
    //   0x20 = GET_LIDAR_CONF 设备配置（可变长）
    // 【通信协议 LR001 P.8】数据类型表示了数据应答报文发送内容的类型，
    //   它与 RPLIDAR 接收到的请求报文类型所对应。
    sl_u8  type;
} __attribute__((packed)) sl_lidar_ans_header_t;

// 结束 1 字节对齐设置，恢复默认对齐方式。
#if defined(_WIN32)
#pragma pack()
#endif


// 恢复此前 push 的编译器警告状态。
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
