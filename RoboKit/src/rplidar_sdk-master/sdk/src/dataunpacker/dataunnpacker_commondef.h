/*
 *  Slamtec LIDAR SDK
 *
 *  Copyright (c) 2014 - 2023 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */

 /*
  *  Sample Data Unpacker System
  *  External Reference and dependencies
  *
  *  本文件是数据解包器(dataunpacker)子系统的公共依赖头文件，
  *  负责统一引入 SDK 各层所需的基础头文件（平台抽象层 HAL、通用类型、
  *  LIDAR 驱动接口、CRC 校验等），并定义解包器模块级别的编译开关宏。
  *  所有 dataunpacker 模块的 .cpp 文件都会首先包含此文件。
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

#pragma once

// -----------------------------------------------------------------------------
// 以下为数据解包器模块统一依赖的头文件集合
// -----------------------------------------------------------------------------

// SDK 通用定义：基本数据类型别名（_u8, _u16, _u32, _u64, _s32 等）、
// 宏定义和通用工具函数。是所有模块的基础依赖。
#include "sdkcommon.h"

// HAL 串口/网络收发抽象层：定义跨平台的串口、TCP/UDP 等数据收发接口。
// 解包器虽然不直接操作串口，但需要使用其定义的 getus() 等时间函数。
#include "hal/abs_rxtx.h"

// HAL 线程抽象层：提供跨平台的线程、互斥锁等同步原语。
#include "hal/thread.h"

// HAL 基础类型定义：包括 LIDARInterfaceType 枚举等接口类型定义。
#include "hal/types.h"

// HAL 断言宏：提供跨平台的 assert 实现。
#include "hal/assert.h"

// HAL 互斥锁封装：用于多线程环境下的资源保护。
#include "hal/locker.h"

// HAL 网络套接字抽象层：提供跨平台的 socket 操作封装。
#include "hal/socket.h"

// HAL 事件抽象层：提供跨平台的事件/信号机制。
#include "hal/event.h"

// HAL 等待器抽象层：提供跨平台的条件变量/等待原语。
#include "hal/waiter.h"

// HAL 字节序转换工具：提供大小端字节序转换函数
// （le16_to_cpu, le32_to_cpu 等），用于处理 RPLIDAR 小端数据。
// RPLIDAR 协议采用小字端(little endian)模式发送数据（【通信协议 LR001 P.6】）。
#include "hal/byteorder.h"

// SLAMTEC LIDAR 驱动接口头文件：定义了 rplidar 系列数据结构
// （rplidar_response_measurement_node_t、rplidar_response_measurement_node_hq_t、
//  rplidar_response_capsule_measurement_nodes_t 等应答报文结构体），
// 以及应答类型枚举（RPLIDAR_ANS_TYPE_MEASUREMENT 等）。
// 这些结构直接对应通信协议中定义的各种应答报文格式。
#include "sl_lidar_driver.h"

// SLAMTEC CRC 校验库：提供 CRC32 校验算法实现。
// 用于 HQ 报文（0x83 应答）的 CRC32 校验验证。
#include "sl_crc.h"

// C++ 标准库算法头文件：使用 std::min/std::max 等工具。
#include <algorithm>

// C++ 智能指针头文件：使用 std::shared_ptr 等智能指针管理资源。
#include <memory>

// -----------------------------------------------------------------------------
// 模块级编译开关宏定义
// -----------------------------------------------------------------------------

// 编译开关：不使用 Boost 库的 CRC 实现。
// 定义此宏后，HQ 报文的 CRC32 校验将使用 SDK 自带的 sl_crc 实现，
// 而非 boost::crc_optimal。这减少了对 Boost 库的依赖，便于在嵌入式
// 平台上编译部署。
#define CONF_NO_BOOST_CRC_SUPPORT

// 引入数据解包器命名空间宏定义（BEGIN_DATAUNPACKER_NS / END_DATAUNPACKER_NS）
#include "dataupacker_namespace.h"


