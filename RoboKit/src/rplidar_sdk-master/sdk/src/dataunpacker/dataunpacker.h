/*
 *  Slamtec LIDAR SDK
 *
 *  Copyright (c) 2014 - 2023 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */

 /*
  *  Sample Data Unpacker System
  *  数据解包器系统 - 对外公共接口定义
  *
  *  本文件定义了数据解包器子系统的两个核心对外接口类：
  *  1. LIDARSampleDataListener  —— 解包结果监听器接口（由使用方实现，接收解包结果）
  *  2. LIDARSampleDataUnpacker  —— 数据解包器抽象接口（由 SDK 实现，接收并解码原始报文）
  *
  *  RPLIDAR 测距核心在扫描时，会持续发送原始数据应答报文。
  *  不同的扫描模式（Standard/Express/Boost/Sensitivity/Stability/DenseBoost）
  *  使用不同的报文格式（【通信协议 LR001 P.11】）。
  *  数据解包器负责将这些不同格式的压缩报文解码为统一的高精度测量节点
  *  (rplidar_response_measurement_node_hq_t)，并回调通知监听器。
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

// 引入数据解包器命名空间宏定义
#include "dataupacker_namespace.h"

BEGIN_DATAUNPACKER_NS()

// =============================================================================
// LIDARSampleDataListener —— 解包结果监听器接口（回调接口）
// -----------------------------------------------------------------------------
// 这是一个纯虚接口类，由使用方（通常是 LIDAR 驱动层）实现。
// 数据解包器在完成对原始报文的解码后，会通过此接口的方法将解包结果
// 回调通知给使用方。使用方据此获取到统一的 HQ 格式测量节点数据。
//
// 设计说明：采用回调（观察者）模式，使数据解包器与上层驱动解耦，
// 解包器只负责"解码"这一职责，不关心数据如何被消费。
// =============================================================================
class LIDARSampleDataListener
{


public:
	// -------------------------------------------------------------------------
	// onHQNodeScanResetReq: 新扫描圈重置请求回调
	// 当解包器检测到标志位 S=1（新的一圈 360 度扫描开始，或转速不均衡
	// 导致角度无法计算需要重新解析【通信协议 LR001 P.27-28】）时调用。
	// 使用方收到此回调后应当重新开始新一轮扫描数据的组装。
	// -------------------------------------------------------------------------
	virtual void onHQNodeScanResetReq() = 0;

	// -------------------------------------------------------------------------
	// onHQNodeDecoded: 单个测量节点解包完成回调
	// 参数:
	//   timestamp_uS - 该采样点的时间戳（微秒），已扣除采样延迟补偿
	//   node         - 指向解包后的高精度测量节点数据（角度+距离+质量+同步标志）
	//
	// 每解出一个测量点，解包器都会调用此方法。
	// 这是最核心的回调方法，使用方通过它逐点获取扫描数据。
	// -------------------------------------------------------------------------
	virtual void onHQNodeDecoded(_u64 timestamp_uS, const rplidar_response_measurement_node_hq_t* node) = 0;

	// -------------------------------------------------------------------------
	// onCustomSampleDataDecoded: 自定义采样数据解包完成回调（可选实现）
	// 用于处理非标准的自定义格式数据应答。
	// 参数:
	//   ansType    - 应答类型编号
	//   customCode - 自定义数据编码标识
	//   data       - 解码后的数据指针
	//   size       - 数据大小
	// 默认空实现，使用方按需覆写。
	// -------------------------------------------------------------------------
	virtual void onCustomSampleDataDecoded(_u8 ansType, _u32 customCode, const void* data, size_t size) {}

	// -------------------------------------------------------------------------
	// onDecodingError: 解码错误回调（可选实现）
	// 当解包器在处理数据时遇到校验和错误、编码器重置等异常情况时调用。
	// 参数:
	//   errMsg  - 错误码（见 LIDARSampleDataUnpacker::ERR_EVENT_ON_EXP_* 枚举）
	//   ansType - 出错时的应答类型
	//   payload - 出错时的原始报文数据指针
	//   size    - 原始报文数据大小
	// 默认空实现，使用方按需覆写以进行错误日志记录或恢复处理。
	// -------------------------------------------------------------------------
	virtual void onDecodingError(int errMsg, _u8 ansType, const void* payload, size_t size) {}
};

// =============================================================================
// LIDARSampleDataUnpacker —— 数据解包器抽象接口
// -----------------------------------------------------------------------------
// 这是数据解包器子系统的对外抽象接口。SDK 在内部提供了具体实现
// （LIDARSampleDataUnpackerImpl，见 dataunpacker.cpp），使用方通过
// 静态工厂方法 CreateInstance 创建实例。
//
// 工作流程：
//   1. 使用方实现 LIDARSampleDataListener 接口
//   2. 调用 CreateInstance(listener) 创建解包器实例
//   3. 调用 enable() 启用解包器
//   4. 驱动层收到原始应答数据后调用 onSampleData() 喂入数据
//   5. 解包器解码后通过 listener 回调输出结果
//   6. 使用完毕调用 ReleaseInstance() 释放
// =============================================================================
class LIDARSampleDataUnpacker
{
public:
	// -------------------------------------------------------------------------
	// 错误事件枚举：定义解包过程中可能发生的错误类型
	// 这些错误码通过 LIDARSampleDataListener::onDecodingError 回调上报
	// -------------------------------------------------------------------------
	enum {
		// 扩展扫描编码器重置事件：
		// 当解包器在数据流中检测到非首帧的 S=1 标志位时触发。
		// 表示雷达转速不均衡或其他原因导致角度值无法通过公式计算，
		// 外部系统应重新开始数据解析（【通信协议 LR001 P.27-28】）
		ERR_EVENT_ON_EXP_ENCODER_RESET = 0x8001,

		// 校验和错误事件：
		// 当接收到的报文校验和不匹配时触发。
		// 表示报文在传输过程中可能发生了数据损坏。
		ERR_EVENT_ON_EXP_CHECKSUM_ERR = 0x8002,
	};

	// -------------------------------------------------------------------------
	// UnpackerContextType: 解包器上下文类型枚举
	// 通过 updateUnpackerContext() 方法向解包器及其内部 handler 传递
	// 设备相关的上下文信息（如时间参数、光学参数等），用于精确计算
	// 采样延迟补偿和角度补偿。
	// -------------------------------------------------------------------------
	enum UnpackerContextType {
		// 未知类型（默认占位值）
		UNPACKER_CONTEXT_TYPE_LIDAR_UNKNOWN = 0,
		// 雷达时序参数：传递 SlamtecLidarTimingDesc 结构，
		// 包含采样持续时间、传输波特率、链路延迟等，用于计算时间戳补偿
		UNPACKER_CONTEXT_TYPE_LIDAR_TIMING = 1,
		// 三角测距光学因子：传递光学模型参数，
		// 用于 UltraCapsule 等模式的角度补偿计算
		UNPACKER_CONTEXT_TYPE_TRIANGULATION_OPTICAL_FACTOR = 2,
	};

	// 析构函数（虚函数，确保通过基类指针正确析构派生类）
	virtual ~LIDARSampleDataUnpacker();

	// -------------------------------------------------------------------------
	// CreateInstance: 工厂方法 —— 创建数据解包器实例
	// 参数:
	//   listener - 解包结果监听器引用（解包器持有此引用的生命周期内有效）
	// 返回: 新创建的解包器实例指针，使用方需通过 ReleaseInstance 释放
	// -------------------------------------------------------------------------
	static LIDARSampleDataUnpacker* CreateInstance(LIDARSampleDataListener& listener);

	// -------------------------------------------------------------------------
	// ReleaseInstance: 释放数据解包器实例
	// 参数: 通过 CreateInstance 创建的解包器实例指针
	// -------------------------------------------------------------------------
	static void ReleaseInstance(LIDARSampleDataUnpacker*);

	// -------------------------------------------------------------------------
	// updateUnpackerContext: 更新解包器上下文信息
	// 将设备相关参数（时序、光学因子等）传递给所有内部 handler，
	// 使其能够精确计算采样延迟补偿和角度补偿。
	// 参数:
	//   type - 上下文类型（见 UnpackerContextType 枚举）
	//   data - 上下文数据指针
	//   size - 数据大小
	// -------------------------------------------------------------------------
	virtual void updateUnpackerContext(UnpackerContextType type, const void* data, size_t size) = 0;

	// -------------------------------------------------------------------------
	// enable / disable: 启用/禁用解包器
	// enable() 会重置解包器状态并开始接收数据；
	// disable() 会停止接收数据并清理缓存。
	// 解包器在 disable 状态下 onSampleData() 调用将直接返回 false。
	// -------------------------------------------------------------------------
	virtual void enable() = 0;
	virtual void disable() = 0;

	// -------------------------------------------------------------------------
	// onSampleData: 接收原始采样数据并解包
	// 这是驱动层向解包器喂入原始应答报文数据的入口。
	// 解包器根据 ansType 选择对应的 handler 处理数据。
	// 参数:
	//   ansType - 应答类型，区分不同扫描模式的报文格式：
	//             0x81 = NormalNode  (标准SCAN，5字节/点【通信协议 LR001 P.14-16】)
	//             0x82 = CapsuleNode  (传统版Express，84字节/32点【通信协议 LR001 P.17-20】)
	//             0x83 = HQNode      (HQ扫描模式，CRC32校验【通信协议 LR001 P.11】)
	//             0x84 = UltraCapsule (扩展版Express，132字节/96点【通信协议 LR001 P.22-23】)
	//             0x85 = DenseCapsule (密实版Express，84字节/40点【通信协议 LR001 P.24-25】)
	//             0x86 = UltraDenseCapsule (超密实版Express)
	//   buffer  - 原始报文数据缓冲区指针
	//   size    - 数据大小（字节数）
	// 返回: true 表示数据被接受处理，false 表示解包器未启用或无对应 handler
	// -------------------------------------------------------------------------
	virtual bool onSampleData(_u8 ansType, const void* buffer, size_t size) = 0;

	// -------------------------------------------------------------------------
	// reset: 重置解包器状态
	// 清空所有内部缓存和状态，相当于重新开始接收数据。
	// -------------------------------------------------------------------------
	virtual void reset() = 0;

	// -------------------------------------------------------------------------
	// clearCache: 清除当前 handler 的缓存
	// 仅清除当前活跃 handler 的接收缓存（半截的报文数据），
	// 不改变 handler 选择状态。用于处理数据流中断后的残留数据。
	// -------------------------------------------------------------------------
	virtual void clearCache() = 0;

protected:
	// 构造函数（protected，防止直接实例化，只能通过 CreateInstance 创建）
	LIDARSampleDataUnpacker(LIDARSampleDataListener&);

	// 监听器引用：指向使用方实现的结果回调对象
	// 生命周期由使用方管理，解包器仅持有引用
	LIDARSampleDataListener& _listener;

};

END_DATAUNPACKER_NS()