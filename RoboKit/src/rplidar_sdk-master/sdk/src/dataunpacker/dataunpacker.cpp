/*
 *  Slamtec LIDAR SDK
 *
 *  Copyright (c) 2014 - 2023 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */

 /*
  *  Sample Data Unpacker System
  *  数据解包器系统 - 实现文件
  *
  *  本文件实现了数据解包器的具体类 LIDARSampleDataUnpackerImpl，
  *  并注册了 6 种扫描模式对应的 handler。
  *
  *  LIDARSampleDataUnpackerImpl 的核心职责：
  *  1. 管理 ansType → handler 的映射关系
  *  2. 收到原始数据后根据 ansType 分发给对应 handler
  *  3. 当应答类型切换时自动重置旧 handler 并激活新 handler
  *  4. 实现 LIDARSampleDataUnpackerInner 的发布方法，将结果回调给监听器
  *
  *  6 种 handler 对应通信协议中的不同应答数据类型：
  *  - NormalNode(0x81):       标准SCAN模式，5字节/采样点【通信协议 LR001 P.14-16】
  *  - HQNode(0x83):           HQ扫描模式，CRC32校验，96个采样点/报文
  *  - CapsuleNode(0x82):      传统版Express，84字节/32点【通信协议 LR001 P.17-20】
  *  - UltraCapsuleNode(0x84): 扩展版Express，132字节/96点【通信协议 LR001 P.22-23】
  *  - DenseCapsuleNode(0x85): 密实版Express，84字节/40点【通信协议 LR001 P.24-25】
  *  - UltraDenseCapsuleNode(0x86): 超密实版Express，更高级压缩格式
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

// 引入数据解包器公共依赖、对外接口定义和内部接口定义
#include "dataunnpacker_commondef.h"
#include "dataunpacker.h"
#include "dataunnpacker_internal.h"


// C++ 标准库 map 容器：用于建立 ansType → handler 的映射关系
#include <map>


// =============================================================================
// REGISTER_HANDLER 宏：动态创建并注册一个 handler 实例
// -----------------------------------------------------------------------------
// 参数 _c_ 为 handler 类名（位于 unpacker 命名空间内）。
// 宏展开后会在堆上 new 一个 handler 实例并加入 handlerList 列表。
// 如果分配失败则返回 false 终止注册过程。
// =============================================================================
#define REGISTER_HANDLER(_c_) {     \
		auto newBorn = new unpacker::_c_();   \
		if (!newBorn) return false; \
		handlerList.push_back(newBorn); \
	}

// -----------------------------------------------------------------------------
// 如何添加新的 handler？
// 1. 在下方添加新 handler 的头文件包含
// 2. 在 _registerDataUnpackerHandlers 函数中添加对应的 REGISTER_HANDLER 调用
// -----------------------------------------------------------------------------

// 引入各 handler 的头文件声明：
// handler_capsules.h: 声明 CapsuleNode/UltraCapsuleNode/DenseCapsuleNode/UltraDenseCapsuleNode 四种 handler
#include "unpacker/handler_capsules.h"
// handler_hqnode.h: 声明 HQNode handler（0x83 HQ扫描模式）
#include "unpacker/handler_hqnode.h"
// handler_normalnode.h: 声明 NormalNode handler（0x81 标准SCAN模式）
#include "unpacker/handler_normalnode.h"


// 标记：handler 列表注册宏定义段开始
#define  DEF_REGISTER_HANDLER_LIST


BEGIN_DATAUNPACKER_NS()


// -----------------------------------------------------------------------------
// _registerDataUnpackerHandlers: 注册所有支持的扫描模式 handler
// -----------------------------------------------------------------------------
// 此函数在 CreateInstance 中被调用，创建并注册所有 6 种 handler 实例。
// 每个 handler 通过 getSampleAnswerType() 返回其处理的应答类型编号，
// Impl 据此建立 ansType → handler 映射表。
//
// 注册的 handler 列表及其对应的通信协议应答类型：
//   NormalNode         → 0x81 (RPLIDAR_ANS_TYPE_MEASUREMENT)        标准SCAN 5字节/点【通信协议 LR001 P.14-16】
//   HQNode             → 0x83 (RPLIDAR_ANS_TYPE_MEASUREMENT_HQ)    HQ扫描模式
//   CapsuleNode        → 0x82 (RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED)       传统Express 84字节/32点【通信协议 LR001 P.17-20】
//   UltraCapsuleNode   → 0x84 (RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED_ULTRA) 扩展Express 132字节/96点【通信协议 LR001 P.22-23】
//   DenseCapsuleNode  → 0x85 (RPLIDAR_ANS_TYPE_MEASUREMENT_DENSE_CAPSULED)  密实Express 84字节/40点【通信协议 LR001 P.24-25】
//   UltraDenseCapsuleNode → 0x86 (RPLIDAR_ANS_TYPE_MEASUREMENT_ULTRA_DENSE_CAPSULED) 超密实Express
// -----------------------------------------------------------------------------
static bool _registerDataUnpackerHandlers(std::vector<IDataUnpackerHandler *> & handlerList)
{
	// 注册标准SCAN模式handler（0x81，5字节/采样点）
	REGISTER_HANDLER(UnpackerHandler_NormalNode);
	// 注册HQ扫描模式handler（0x83，CRC32校验，96个采样点/报文）
	REGISTER_HANDLER(UnpackerHandler_HQNode);
	// 注册传统版Express模式handler（0x82，84字节/32点）
	REGISTER_HANDLER(UnpackerHandler_CapsuleNode);
	// 注册扩展版Express模式handler（0x84，132字节/96点，专利压缩编码）
	REGISTER_HANDLER(UnpackerHandler_UltraCapsuleNode);
	// 注册密实版Express模式handler（0x85，84字节/40点）
	REGISTER_HANDLER(UnpackerHandler_DenseCapsuleNode);
	// 注册超密实版Express模式handler（0x86，更高级压缩格式）
	REGISTER_HANDLER(UnpackerHandler_UltraDenseCapsuleNode);
	return true;
}


// =============================================================================
// LIDARSampleDataUnpackerImpl —— 数据解包器具体实现类
// -----------------------------------------------------------------------------
// 继承自 LIDARSampleDataUnpackerInner，实现了数据分发和结果发布的全部逻辑。
// 它是整个解包器子系统的核心调度器：
//
//   外部数据流  →  onSampleData(ansType, data)  →  handler->onData(engine, data)
//                                                          ↓
//   listener.onHQNodeDecoded()  ←  publishHQNode()  ←  解码完成回调
//
// 该类维护一个 _handlerMap（ansType → handler 映射表），根据收到的应答类型
// 选择对应的 handler 处理数据。当应答类型切换时（如从标准SCAN切换到
// Express模式），会自动重置旧 handler 并激活新 handler。
// =============================================================================
class LIDARSampleDataUnpackerImpl : public LIDARSampleDataUnpackerInner
{
public:

	// -------------------------------------------------------------------------
	// registerHandler: 注册一个 handler 到映射表
	// 建立 ansType → handler 的对应关系，使得收到对应类型的应答数据时
	// 能够找到正确的 handler 进行处理。
	// -------------------------------------------------------------------------
	void registerHandler(_u8 ansType, IDataUnpackerHandler* handler)
	{
		_handlerMap[ansType] = handler;
	}


	// -------------------------------------------------------------------------
	// unregisterAllHandlers: 注销并释放所有 handler
	// 遍历映射表，delete 每个 handler 实例并清空映射表。
	// 在析构函数中调用，确保没有内存泄漏。
	// -------------------------------------------------------------------------
	void unregisterAllHandlers()
	{
		for (auto itr = _handlerMap.begin(); itr != _handlerMap.end(); ++itr)
		{
			delete itr->second;
		}
		_handlerMap.clear();
	}

	// -------------------------------------------------------------------------
	// 构造函数：初始化成员变量
	// _enabled 初始为 false，需调用 enable() 后才能接收数据
	// _lastActiveAnsType 初始为 0，表示尚未有任何 handler 被激活
	// _lastActiveHandler 初始为 nullptr
	// -------------------------------------------------------------------------
	LIDARSampleDataUnpackerImpl(LIDARSampleDataListener& l)
		: LIDARSampleDataUnpackerInner(l)
		, _enabled(false)
		, _lastActiveAnsType(0)
		, _lastActiveHandler(nullptr)
	{

	}

	// -------------------------------------------------------------------------
	// 析构函数：注销所有 handler 释放内存
	// -------------------------------------------------------------------------
	virtual ~LIDARSampleDataUnpackerImpl()
	{
		unregisterAllHandlers();
	}


	// -------------------------------------------------------------------------
	// updateUnpackerContext: 更新解包器上下文（实现基类纯虚方法）
	// 遍历所有已注册的 handler，将上下文信息（如时序参数、光学因子）传递
	// 给每个 handler，使其能够精确计算采样延迟补偿。
	// -------------------------------------------------------------------------
	virtual void updateUnpackerContext(UnpackerContextType type, const void* data, size_t size)
	{

		// 通知所有 handler 更新上下文参数
		for (auto itr = _handlerMap.begin(); itr != _handlerMap.end(); ++itr)
		{
			itr->second->onUnpackerContextSet(type, data, size);
		}
	}

	// -------------------------------------------------------------------------
	// onSampleData: 接收原始采样数据并分发给对应 handler（实现基类纯虚方法）
	// 这是驱动层向解包器喂入数据的核心入口。
	//
	// 工作流程：
	//   1. 检查是否已启用，未启用则直接返回 false
	//   2. 如果应答类型与上次不同（说明扫描模式切换），
	//      先重置旧 handler 状态，再在映射表中查找新类型对应的 handler
	//   3. 找到 handler 后调用其 onData 方法进行具体解码
	//   4. 找不到对应 handler 则返回 false
	// -------------------------------------------------------------------------
	virtual bool onSampleData(_u8 ansType, const void* buffer, size_t size) {
		// 解包器未启用，拒绝处理数据
		if (!_enabled) return false;


		// 应答类型发生变化：需要切换活跃 handler
		// 这通常发生在雷达从标准SCAN模式切换到Express模式等场景
		if (_lastActiveAnsType != ansType) {
			// 先重置当前 handler 状态，丢弃半截的报文数据
			onDeselectHandler();

			// 在映射表中查找新应答类型对应的 handler
			auto itr = _handlerMap.find(ansType);
			if (itr != _handlerMap.end()) {
				// 找到对应 handler，设为当前活跃 handler
				onSelectHandler(ansType, itr->second);
			}
			else {
				// 未找到对应 handler（不支持该应答类型）
				onSelectHandler(ansType, nullptr);
			}

		}

		// 如果存在活跃 handler，将数据分发给它处理
		if (_lastActiveHandler) {
			_lastActiveHandler->onData(this, reinterpret_cast<const _u8 *>(buffer), size);
			return true;
		}
		else {
			// 无活跃 handler，无法处理数据
			return false;
		}
	}

	// -------------------------------------------------------------------------
	// reset: 重置解包器状态（实现基类纯虚方法）
	// 清空当前 handler 缓存并重置活跃 handler 选择状态，
	// 使解包器回到"等待新数据流"的初始状态。
	// -------------------------------------------------------------------------
	virtual void reset()
	{
		clearCache();
		_lastActiveHandler = nullptr;
		_lastActiveAnsType = 0;

	}

	// -------------------------------------------------------------------------
	// enable: 启用解包器（实现基类纯虚方法）
	// 设置启用标志并重置状态，此后 onSampleData 调用将被接受处理。
	// -------------------------------------------------------------------------
	virtual void enable()
	{
		_enabled = true;
		reset();
	}

	// -------------------------------------------------------------------------
	// disable: 禁用解包器（实现基类纯虚方法）
	// 清除启用标志并重置状态，此后 onSampleData 调用将被拒绝。
	// -------------------------------------------------------------------------
	virtual void disable()
	{
		_enabled = false;
		reset();

	}

	// -------------------------------------------------------------------------
	// clearCache: 清除当前活跃 handler 的缓存（实现基类纯虚方法）
	// 调用当前 handler 的 reset() 方法，丢弃尚未组装完整的半截报文数据。
	// 不改变 handler 选择状态（下次同类型数据无需重新查找 handler）。
	// -------------------------------------------------------------------------
	virtual void clearCache()
	{
		if (_lastActiveHandler) {
			_lastActiveHandler->reset();
		}
	}

	// -------------------------------------------------------------------------
	// getCurrentTimestamp_uS: 获取当前时间戳（实现内部接口纯虚方法）
	// 调用 HAL 层 getus() 获取当前系统时间的微秒级时间戳。
	// handler 在发布测量节点时通过此方法获取时间戳。
	// -------------------------------------------------------------------------
	virtual _u64 getCurrentTimestamp_uS() {
		return getus();
	}

	// -------------------------------------------------------------------------
	// publishHQNode: 发布解包完成的高精度测量节点（实现内部接口纯虚方法）
	// 将 handler 解码出的测量节点通过监听器回调通知给使用方。
	// -------------------------------------------------------------------------
	virtual void publishHQNode(_u64 timestamp_uS, const rplidar_response_measurement_node_hq_t* node)
	{
		_listener.onHQNodeDecoded(timestamp_uS, node);
	}


	// -------------------------------------------------------------------------
	// publishDecodingErrorMsg: 发布解码错误消息（实现内部接口纯虚方法）
	// 将 handler 检测到的错误（校验失败、编码器重置等）通知给使用方。
	// -------------------------------------------------------------------------
	virtual void publishDecodingErrorMsg(int errorType, _u8 ansType, const void* payload, size_t size)
	{
		_listener.onDecodingError(errorType, ansType, payload, size);

	}

	// -------------------------------------------------------------------------
	// publishCustomData: 发布自定义格式数据（实现内部接口纯虚方法）
	// -------------------------------------------------------------------------
	virtual void publishCustomData(_u8 ansType, _u32 customCode, const void* payload, size_t size)
	{
		_listener.onCustomSampleDataDecoded(ansType, customCode, payload, size);
	}


	// -------------------------------------------------------------------------
	// publishNewScanReset: 发布新扫描圈重置信号（实现内部接口纯虚方法）
	// 当 handler 检测到 S=1 标志位时调用（【通信协议 LR001 P.27-28】），
	// 通知使用方重新开始一轮新的扫描数据组装。
	// -------------------------------------------------------------------------
	virtual void publishNewScanReset()
	{
		_listener.onHQNodeScanResetReq();
	}
protected:

	// -------------------------------------------------------------------------
	// onSelectHandler: 激活指定的 handler
	// 记录当前活跃的应答类型和 handler 指针，
	// 后续 onSampleData 调用将直接使用此 handler（无需重复查找）。
	// -------------------------------------------------------------------------
	void onSelectHandler(_u8 ansType, IDataUnpackerHandler* handler)
	{
		_lastActiveHandler = handler;
		_lastActiveAnsType = ansType;
	}

	// -------------------------------------------------------------------------
	// onDeselectHandler: 取消激活当前 handler
	// 调用 reset() 清空状态，使解包器回到等待新 handler 选择的状态。
	// -------------------------------------------------------------------------
	void onDeselectHandler()
	{
		reset();
	}


protected:
	// 是否已启用（enable() 后为 true，disable() 后为 false）
	bool _enabled;
	// 应答类型 → handler 的映射表，在 CreateInstance 时通过 registerHandler 建立
	std::map<_u8, IDataUnpackerHandler*> _handlerMap;

	// 上次处理的应答类型编号，用于检测应答类型是否发生变化
	_u8 _lastActiveAnsType;
	// 当前活跃的 handler 指针，数据直接分发给它处理
	IDataUnpackerHandler* _lastActiveHandler;
};

// -----------------------------------------------------------------------------
// CreateInstance: 工厂方法 —— 创建数据解包器实例（实现基类静态方法）
// -----------------------------------------------------------------------------
// 工作流程：
//   1. 创建 LIDARSampleDataUnpackerImpl 实例
//   2. 调用 _registerDataUnpackerHandlers 创建所有 6 种 handler 实例
//   3. 如果注册失败，清理已创建的 handler 和 impl 实例，返回 nullptr
//   4. 成功则遍历 handler 列表，通过 getSampleAnswerType() 获取每个 handler
//      对应的应答类型，调用 registerHandler 建立映射关系
//   5. 返回创建好的解包器实例
//
// 使用方在使用完毕后必须调用 ReleaseInstance 释放内存。
// -----------------------------------------------------------------------------
LIDARSampleDataUnpacker* LIDARSampleDataUnpacker::CreateInstance(LIDARSampleDataListener& listener)
{
	// 创建解包器实现实例
	LIDARSampleDataUnpackerImpl* impl = new LIDARSampleDataUnpackerImpl(listener);

	// 创建并注册所有 handler 实例
	std::vector<IDataUnpackerHandler*> list;
	if (!_registerDataUnpackerHandlers(list)) {
		// 注册失败：清理已创建的 handler 和 impl 实例
		delete  impl;
		for (auto itr = list.begin(); itr != list.end(); ++itr) {
			delete* itr;
		}
		impl = nullptr;
	}

	// 遍历 handler 列表，建立 ansType → handler 映射关系
	// 每个 handler 的 getSampleAnswerType() 返回其处理的应答类型编号
	for (auto itr = list.begin(); itr != list.end(); ++itr) {
		impl->registerHandler((*itr)->getSampleAnswerType(), (*itr));
	}
	return impl;
}

// -----------------------------------------------------------------------------
// ReleaseInstance: 释放数据解包器实例（实现基类静态方法）
// 直接 delete 解包器指针，触发 LIDARSampleDataUnpackerImpl 析构函数
// 释放所有 handler 内存。
// -----------------------------------------------------------------------------
void LIDARSampleDataUnpacker::ReleaseInstance(LIDARSampleDataUnpacker* unpacker) {
	delete unpacker;
}

// 基类虚析构函数实现（空函数体，实际清理在派生类 Impl 析构中完成）
LIDARSampleDataUnpacker::~LIDARSampleDataUnpacker() {

}

// 基类构造函数实现：保存监听器引用
LIDARSampleDataUnpacker::LIDARSampleDataUnpacker(LIDARSampleDataListener& l)
	: _listener(l)
{

}


END_DATAUNPACKER_NS()