/*
* Slamtec LIDAR SDK
*
* sl_lidar.h
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
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO,
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
// 本文件是 Slamtec LIDAR SDK 的顶层入口头文件，定义了 SDK 的版本号
// 宏及版本号字符串拼接工具宏。用户应用程序只需 #include "sl_lidar.h"
// 即可同时引入 sl_lidar_driver.h（驱动抽象接口）及其传递依赖的全部
// 协议结构体与常量定义。
//
// 版本号采用语义化版本(SemVer)三段式：主版本号.次版本号.修订号。
// 主版本号变更表示不兼容的 API 变更；次版本号变更表示向下兼容的功能
// 新增；修订号变更表示向下兼容的问题修复。
//
// 【SDK手册 LR002 P.12】sl_lidar.h 为 SDK 的版本信息头文件，
//   同时包含了 sl_lidar_driver.h 驱动接口头文件。
// ===================================================================

#pragma once

// 引入驱动抽象接口头文件，其中定义了 ILidarDriver 驱动接口、
// IChannel 通信通道接口、LidarScanMode 扫描模式结构体、以及
// createLidarDriver()/createSerialPortChannel() 等工厂函数。
// 该头文件又会传递引入 sl_lidar_cmd.h（命令/应答协议结构体）和
// sl_lidar_protocol.h（请求/应答报文头结构体）等协议层头文件。
#include "sl_lidar_driver.h"

// SDK 主版本号。不兼容的 API 变更时递增。
// 当前为 2，表示第二代 SDK 接口（相比早期 RPLIDAR SDK 有重大重构）。
#define SL_LIDAR_SDK_VERSION_MAJOR  2

// SDK 次版本号。向下兼容的功能新增时递增。
#define SL_LIDAR_SDK_VERSION_MINOR  1

// SDK 修订号。向下兼容的问题修复时递增。
#define SL_LIDAR_SDK_VERSION_PATCH  0

// 将三段版本号打包为一个 32 位整数的组合版本序列号。
// 布局：[主版本号(16bits) | 次版本号(8bits) | 修订号(8bits)]
// 可用于运行时通过整数比较判断 SDK 版本高低，例如：
//   #if SL_LIDAR_SDK_VERSION_SEQ >= 0x00020100 // >= 2.1.0
// 这样设计是因为主版本号变化范围最大（放在高位），次版本号和修订号
// 范围较小（放在低位），用一个 32 位整数即可完整表示版本层级关系。
#define SL_LIDAR_SDK_VERSION_SEQ    ((SL_LIDAR_SDK_VERSION_MAJOR << 16) | (SL_LIDAR_SDK_VERSION_MINOR << 8) | SL_LIDAR_SDK_VERSION_PATCH)


// -------------------------------------------------------------------
// 以下两个宏实现"字符串化"(stringification)功能，用于将版本号宏的
// 数值转换为字符串字面量，最终拼接成形如 "2.1.0" 的版本字符串。
//
// 为什么要分两层（INDIR + 外层）？
// 因为 C 预处理器宏展开时，# 运算符作用于宏参数时不会进行二次展开。
// 如果直接使用 #x，当 x 本身是一个宏(如 SL_LIDAR_SDK_VERSION_MAJOR)
// 时，#x 会产生字符串 "SL_LIDAR_SDK_VERSION_MAJOR" 而非其值 "2"。
// 通过先经过 INDIR 这一层强制宏展开，外层宏传入已展开的值再进行
// 字符串化，从而正确得到 "2"。
// -------------------------------------------------------------------

// 字符串化辅助宏（内层）：对参数 x 执行 # 运算将其转换为字符串字面量。
// 此宏必须经由另一层宏调用，以确保 x 在被 # 之前已完全展开。
#define SL_LIDAR_SDK_VERSION_MK_STR_INDIR(x)  #x

// 字符串化辅助宏（外层）：先展开参数 x（因 x 可能本身是宏定义），
// 再调用内层宏 SL_LIDAR_SDK_VERSION_MK_STR_INDIR 完成字符串化。
// 例如 SL_LIDAR_SDK_VERSION_MK_STR(SL_LIDAR_SDK_VERSION_MAJOR)
//   → SL_LIDAR_SDK_VERSION_MK_STR_INDIR(2) → "2"
#define SL_LIDAR_SDK_VERSION_MK_STR(x)        SL_LIDAR_SDK_VERSION_MK_STR_INDIR(x)

// SDK 完整版本号字符串，形如 "2.1.0"。
// 通过字符串拼接将三段版本号用 "." 连接，可在运行时打印/日志输出，
// 或在协议握手时上报给上层应用用于版本兼容性检查。
// 展开过程：
//   SL_LIDAR_SDK_VERSION_MK_STR(SL_LIDAR_SDK_VERSION_MAJOR) → "2"
//   SL_LIDAR_SDK_VERSION_MK_STR(SL_LIDAR_SDK_VERSION_MINOR) → "1"
//   SL_LIDAR_SDK_VERSION_MK_STR(SL_LIDAR_SDK_VERSION_PATCH) → "0"
//   "2" "." "1" "." "0" → "2.1.0"
#define SL_LIDAR_SDK_VERSION        (SL_LIDAR_SDK_VERSION_MK_STR(SL_LIDAR_SDK_VERSION_MAJOR) "." SL_LIDAR_SDK_VERSION_MK_STR(SL_LIDAR_SDK_VERSION_MINOR) "." SL_LIDAR_SDK_VERSION_MK_STR(SL_LIDAR_SDK_VERSION_PATCH))
