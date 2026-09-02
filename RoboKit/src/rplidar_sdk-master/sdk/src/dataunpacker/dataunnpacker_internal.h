/*
 *  Slamtec LIDAR SDK
 *
 *  Copyright (c) 2014 - 2023 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */

 /*
  *  Sample Data Unpacker System
  *  Internal Definition
  *
  *  本文件定义了数据解包器子系统的内部接口，仅供 SDK 内部使用。
  *  它在对外接口 LIDARSampleDataUnpacker 的基础上扩展了两个内部类：
  *
  *  1. LIDARSampleDataUnpackerInner —— 内部解包器接口
  *     继承自 LIDARSampleDataUnpacker，增加了将解包结果"发布"给监听器
  *     和获取当前时间戳的内部方法。这些方法由具体实现类提供，
  *     供各 handler 在解码过程中调用。
  *
  *  2. IDataUnpackerHandler —— 数据处理器（handler）接口
  *     每种扫描模式（标准/HQ/传统Capsule/扩展UltraCapsule/密实Dense/超密实UltraDense）
  *     对应一个具体的 handler 实现。handler 负责该模式原始报文的
  *     字节级解析和测量数据恢复。
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

BEGIN_DATAUNPACKER_NS()


// =============================================================================
// LIDARSampleDataUnpackerInner —— 内部解包器接口
// -----------------------------------------------------------------------------
// 继承自对外接口 LIDARSampleDataUnpacker，扩展了一组"发布"方法，
// 供各 handler 在解码过程中将结果回传给监听器。
// 具体实现类 LIDARSampleDataUnpackerImpl（见 dataunpacker.cpp）实现了
// 这些纯虚方法，将调用转发到持有的 LIDARSampleDataListener 引用上。
//
// 这种设计将"调度分发"（由 Impl 负责）和"具体解码"（由各 handler 负责）
// 分离：handler 解码出一个节点后调用 engine->publishHQNode()，
// 由 Impl 统一回调给监听器。
// =============================================================================
class LIDARSampleDataUnpackerInner: public LIDARSampleDataUnpacker
{
public:
	// 构造函数：转发给基类 LIDARSampleDataUnpacker
	LIDARSampleDataUnpackerInner(LIDARSampleDataListener& l): LIDARSampleDataUnpacker(l){}

	// -------------------------------------------------------------------------
	// publishHQNode: 发布一个解包完成的高精度测量节点
	// handler 在成功解码一个采样点后调用此方法，由 Impl 转发给监听器的
	// onHQNodeDecoded 回调。
	// 参数:
	//   timestamp_uS - 采样点时间戳（微秒），通常已扣除采样延迟补偿
	//   node         - 指向解包后的 HQ 测量节点（角度+距离+质量+同步标志）
	// -------------------------------------------------------------------------
	virtual void publishHQNode(_u64 timestamp_uS, const rplidar_response_measurement_node_hq_t* node) = 0;

	// -------------------------------------------------------------------------
	// publishDecodingErrorMsg: 发布解码错误消息
	// handler 在检测到校验和错误、编码器重置等异常时调用，
	// 由 Impl 转发给监听器的 onDecodingError 回调。
	// 参数:
	//   errorType - 错误类型码（见 LIDARSampleDataUnpacker::ERR_EVENT_ON_EXP_*）
	//   ansType   - 出错时的应答类型
	//   payload   - 出错时的原始报文数据指针
	//   size      - 原始报文数据大小
	// -------------------------------------------------------------------------
	virtual void publishDecodingErrorMsg(int errorType, _u8 ansType, const void* payload, size_t size) = 0;

	// -------------------------------------------------------------------------
	// publishCustomData: 发布自定义格式数据
	// 用于处理非标准格式的应答数据。
	// 参数:
	//   ansType    - 应答类型编号
	//   customCode - 自定义数据编码标识
	//   payload    - 解码后的数据指针
	//   size       - 数据大小
	// -------------------------------------------------------------------------
	virtual void publishCustomData(_u8 ansType, _u32 customCode, const void* payload, size_t size) = 0;

	// -------------------------------------------------------------------------
	// publishNewScanReset: 发布新扫描圈重置信号
	// handler 在检测到 S=1 标志位（新的一圈扫描开始或编码器重置
	// 【通信协议 LR001 P.27-28】）时调用，
	// 由 Impl 转发给监听器的 onHQNodeScanResetReq 回调。
	// -------------------------------------------------------------------------
	virtual void publishNewScanReset() = 0;

	// -------------------------------------------------------------------------
	// getCurrentTimestamp_uS: 获取当前时间戳（微秒）
	// handler 在发布测量节点时需要附带时间戳，通过此方法获取。
	// 通常由 Impl 调用 HAL 层的 getus() 函数实现。
	// -------------------------------------------------------------------------
	virtual _u64 getCurrentTimestamp_uS() = 0;

};

// =============================================================================
// IDataUnpackerHandler —— 数据处理器（handler）抽象接口
// -----------------------------------------------------------------------------
// 每种扫描模式的报文格式都由一个具体的 handler 实现类处理。
// 解包器 Impl 持有一组 handler，根据收到的应答类型(ansType)分发数据
// 给对应的 handler。
//
// 目前 SDK 注册了以下 6 种 handler（见 dataunpacker.cpp）：
//   UnpackerHandler_NormalNode         → 0x81 标准SCAN  5字节/点【通信协议 LR001 P.14-16】
//   UnpackerHandler_HQNode             → 0x83 HQ扫描    CRC32校验
//   UnpackerHandler_CapsuleNode        → 0x82 传统Express 84字节/32点【通信协议 LR001 P.17-20】
//   UnpackerHandler_UltraCapsuleNode   → 0x84 扩展Express 132字节/96点【通信协议 LR001 P.22-23】
//   UnpackerHandler_DenseCapsuleNode  → 0x85 密实Express 84字节/40点【通信协议 LR001 P.24-25】
//   UnpackerHandler_UltraDenseCapsuleNode → 0x86 超密实Express
// =============================================================================
class IDataUnpackerHandler
{
public:
	// 默认构造函数
	IDataUnpackerHandler() {}
	// 虚析构函数，确保通过基类指针正确析构派生 handler
	virtual ~IDataUnpackerHandler() {}

	// -------------------------------------------------------------------------
	// onUnpackerContextSet: 接收解包器上下文更新
	// 当 Impl 收到 updateUnpackerContext() 调用时，会遍历所有 handler
	// 调用此方法，将设备参数（时序、光学因子等）传递给每个 handler。
	// handler 缓存这些参数用于后续的延迟补偿和角度补偿计算。
	// 参数:
	//   type - 上下文类型（LIDAR_TIMING / TRIANGULATION_OPTICAL_FACTOR）
	//   data - 上下文数据指针
	//   size - 数据大小
	// -------------------------------------------------------------------------
	virtual void onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size) = 0;

	// -------------------------------------------------------------------------
	// getSampleAnswerType: 返回该 handler 处理的应答类型编号
	// Impl 在注册 handler 时调用此方法获取应答类型，
	// 据此建立 ansType → handler 的映射关系。
	// 返回值: 如 RPLIDAR_ANS_TYPE_MEASUREMENT (0x81) 等
	// -------------------------------------------------------------------------
	virtual _u8 getSampleAnswerType() const = 0;

	// -------------------------------------------------------------------------
	// onData: 处理接收到的原始数据（核心解码入口）
	// 驱动层每收到一段原始数据就调用此方法喂入 handler。
	// handler 在内部维护接收状态机，逐字节/逐字段解析报文，
	// 当完整报文组装完成后执行校验和解码，并通过 engine 回调发布结果。
	// 参数:
	//   engine - 内部解包器引擎指针，用于调用 publishHQNode 等方法
	//   data   - 原始数据字节流指针
	//   size   - 数据长度
	// -------------------------------------------------------------------------
	virtual void onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t size) = 0;

	// -------------------------------------------------------------------------
	// reset: 重置 handler 状态
	// 清空内部接收缓存和解析状态，丢弃半截的报文数据。
	// 在解包器 enable/disable/切换 handler 时调用。
	// -------------------------------------------------------------------------
	virtual void reset() = 0;

};

END_DATAUNPACKER_NS()