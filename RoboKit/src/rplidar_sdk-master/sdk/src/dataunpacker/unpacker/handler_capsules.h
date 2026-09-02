/*
 *  Slamtec LIDAR SDK
 *
 *  Copyright (c) 2014 - 2023 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */

 /*
  *  Sample Data Unpacker System
  *  Capsule Style Sample Node Handlers
  *
  *  本文件声明了 4 种 Capsule（胶囊）格式的高速扫描数据处理器类。
  *  这些 handler 分别处理 EXPRESS_SCAN 命令的不同版本应答数据格式
  *  【通信协议 LR001 P.17-28】。
  *
  *  Capsule 格式概述：
  *  "胶囊"格式是 RPLIDAR 为在有限带宽（如115200bps UART）下传输高采样率
  *  （4kHz及以上）数据而设计的压缩报文格式。通过将多个采样点打包到
  *  一条报文中，并使用角度差值推算各采样点角度，大幅减少了数据量。
  *
  *  4 种 Capsule 格式对应不同的压缩级别和报文结构：
  *
  *  1. CapsuleNode (0x82) —— 传统版 Express
  *     【通信协议 LR001 P.17-20】
  *     报文长度: 84字节
  *     结构: 2字节sync+checksum + 2字节start_angle_q6 + 16个cabin(5字节each)
  *     每个cabin含2个采样点的distance1/distance2和角度补偿dθ1/dθ2
  *     一条报文含32个采样点
  *     角度公式: θk = ωi + AngleDiff(ωi,ωi+1)/32 * k - dθk
  *
  *  2. UltraCapsuleNode (0x84) —— 扩展版 Express
  *     【通信协议 LR001 P.22-23】
  *     报文长度: 132字节
  *     结构: 2字节sync+checksum + 2字节start_angle_q6 + 32个ultra_cabin(4字节each)
  *     每个ultra_cabin含3个采样点的距离（12bit major可变比例编码 + 10bit predict1 + 10bit predict2）
  *     一条报文含96个采样点
  *     使用SLAMTEC专利压缩编码（CN 108306649 A）
  *
  *  3. DenseCapsuleNode (0x85) —— 密实版 Express
  *     【通信协议 LR001 P.24-25】
  *     报文长度: 84字节
  *     结构: 2字节sync+checksum + 2字节start_angle_q6 + 40个cabin(2字节each)
  *     每个cabin只含1个采样点的distance(16bit)
  *     一条报文含40个采样点
  *     角度公式: θk = ωi + AngleDiff(ωi,ωi+1)/40 * k
  *
  *  4. UltraDenseCapsuleNode (0x86) —— 超密实版 Express
  *     更高级的压缩格式，包含时间戳和设备状态
  *     每条报文含64个采样点（每个cabin含2个采样点）
  *     使用可变比例编码的quality+distance压缩
  *
  *  通用报文格式（所有版本共享的前4字节）：
  *    字节0: sync1(高4位=0xA) | ChkSum[3:0](低4位)
  *    字节1: sync2(高4位=0x5) | ChkSum[7:4](低4位)
  *    字节2: start_angle_q6[7:0]
  *    字节3: S(1bit) | start_angle_q6[14:8]
  *    sync1和sync2的值用于识别报文起始，S为起始标志位
  *    ChkSum为对报文数据（不含sync字段）的按字节异或校验和
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

namespace unpacker {

// =============================================================================
// UnpackerHandler_CapsuleNode —— 传统版 Express 数据处理器
// -----------------------------------------------------------------------------
// 处理传统版 EXPRESS_SCAN 命令（应答类型 0x82）的数据应答报文。
// 报文长度84字节，包含16个cabin（每个5字节），共32个采样点。
// 每个cabin含2个采样点的distance和角度补偿dθ。
// 角度计算公式: θk = ωi + AngleDiff(ωi,ωi+1)/32 * k - dθk
// 【通信协议 LR001 P.17-20】
//
// 关键：角度计算需要前后两条报文的start_angle差值，
// 因此handler必须缓存上一条报文数据（_cached_previous_capsuledata）。
// =============================================================================
class UnpackerHandler_CapsuleNode : public IDataUnpackerHandler {
public:
	// 构造函数
	UnpackerHandler_CapsuleNode();
	// 析构函数
	virtual ~UnpackerHandler_CapsuleNode();

	// 返回应答类型 0x82 (RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED)
	virtual _u8 getSampleAnswerType() const;
	// 逐字节接收并解析传统版Express报文（核心解码入口）
	virtual void onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t size);
	// 重置handler状态
	virtual void reset();
	// 接收时序参数上下文
	virtual void onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size);
protected:

	// _onScanNodeCapsuleData: 解码传统版capsule报文数据
	// 从cabin结构提取distance和angle_offset，利用前后报文start_angle差值
	// 计算每个采样点的角度，发布32个HQ节点
	void _onScanNodeCapsuleData(rplidar_response_capsule_measurement_nodes_t &, LIDARSampleDataUnpackerInner* engine);

	// 接收缓存：逐字节组装完整的传统版capsule报文（84字节）
	std::vector<_u8> _cached_scan_node_buf;
	// 当前接收位置索引
	int              _cached_scan_node_buf_pos;
	// 上一条capsule报文是否已就绪（角度计算需要前后两条报文）
	bool             _is_previous_capsuledataRdy;

	// 缓存的上一条capsule报文数据，用于角度差值计算
	rplidar_response_capsule_measurement_nodes_t _cached_previous_capsuledata;
	// 上一条报文的时间戳缓存
	_u64             _cached_last_data_timestamp_us;

	// 雷达时序参数缓存，用于采样延迟补偿
	SlamtecLidarTimingDesc _cachedTimingDesc;
};

// =============================================================================
// UnpackerHandler_UltraCapsuleNode —— 扩展版 Express 数据处理器
// -----------------------------------------------------------------------------
// 处理扩展版 EXPRESS_SCAN 命令（应答类型 0x84）的数据应答报文。
// 报文长度132字节，包含32个ultra_cabin（每个4字节），共96个采样点。
// 每个ultra_cabin含3个采样点的距离数据，使用SLAMTEC专利压缩编码
// （CN 108306649 A）：12bit major可变比例编码 + 10bit predict1 + 10bit predict2
// 【通信协议 LR001 P.22-23】
//
// 与传统版不同，扩展版不编码角度补偿量dθ，而是需要结合RPLIDAR光学模型
// 在外部系统按公式计算角度。角度补偿使用经验公式计算。
// =============================================================================
class UnpackerHandler_UltraCapsuleNode : public IDataUnpackerHandler {
public:
	// 构造函数
	UnpackerHandler_UltraCapsuleNode();
	// 析构函数
	virtual ~UnpackerHandler_UltraCapsuleNode();

	// 返回应答类型 0x84 (RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED_ULTRA)
	virtual _u8 getSampleAnswerType() const;
	// 逐字节接收并解析扩展版Express报文（核心解码入口）
	virtual void onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t size);
	// 重置handler状态
	virtual void reset();
	// 接收时序参数上下文
	virtual void onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size);
protected:
	// _onScanNodeUltraCapsuleData: 解码扩展版ultra capsule报文数据
	// 使用_varbitscale_decode解码可变比例编码的major距离，
	// 结合predict1/predict2预测编码恢复3个采样点的距离，
	// 利用光学模型公式计算角度补偿
	void _onScanNodeUltraCapsuleData(rplidar_response_ultra_capsule_measurement_nodes_t&, LIDARSampleDataUnpackerInner* engine);


	// 接收缓存：逐字节组装完整的扩展版ultra capsule报文（132字节）
	std::vector<_u8> _cached_scan_node_buf;
	// 当前接收位置索引
	int              _cached_scan_node_buf_pos;
	// 上一条ultra capsule报文是否已就绪
	bool             _is_previous_capsuledataRdy;

	// 缓存的上一条ultra capsule报文数据，用于角度差值计算
	rplidar_response_ultra_capsule_measurement_nodes_t _cached_previous_ultracapsuledata;
	// 上一条报文的时间戳缓存
	_u64             _cached_last_data_timestamp_us;

	// 雷达时序参数缓存
	SlamtecLidarTimingDesc _cachedTimingDesc;

};



// =============================================================================
// UnpackerHandler_DenseCapsuleNode —— 密实版 Express 数据处理器
// -----------------------------------------------------------------------------
// 处理密实版 EXPRESS_SCAN 命令（应答类型 0x85）的数据应答报文。
// 报文长度84字节，包含40个cabin（每个2字节），共40个采样点。
// 每个cabin只含1个采样点的distance(16bit)，不包含角度补偿量。
// 角度计算公式: θk = ωi + AngleDiff(ωi,ωi+1)/40 * k（无dθ补偿项）
// 【通信协议 LR001 P.24-25】
//
// 密实版的特点是每个采样点只保留距离数据，角度完全由线性插值得到，
// 适用于高密度采样场景（如 S1/S2 的 DenseBoost 模式）。
// =============================================================================
class UnpackerHandler_DenseCapsuleNode : public IDataUnpackerHandler {
public:
	// 构造函数
	UnpackerHandler_DenseCapsuleNode();
	// 析构函数
	virtual ~UnpackerHandler_DenseCapsuleNode();

	// 返回应答类型 0x85 (RPLIDAR_ANS_TYPE_MEASUREMENT_DENSE_CAPSULED)
	virtual _u8 getSampleAnswerType() const;
	// 逐字节接收并解析密实版Express报文（核心解码入口）
	virtual void onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t size);
	// 重置handler状态
	virtual void reset();
	// 接收时序参数上下文
	virtual void onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size);
protected:
	// _onScanNodeDenseCapsuleData: 解码密实版dense capsule报文数据
	// 每个cabin只含2字节distance(16bit)，角度由线性插值计算
	void _onScanNodeDenseCapsuleData(rplidar_response_dense_capsule_measurement_nodes_t&, LIDARSampleDataUnpackerInner* engine);


	// 接收缓存：逐字节组装完整的密实版dense capsule报文（84字节）
	std::vector<_u8> _cached_scan_node_buf;
	// 当前接收位置索引
	int              _cached_scan_node_buf_pos;
	// 上一条dense capsule报文是否已就绪
	bool             _is_previous_capsuledataRdy;

	// 缓存的上一条dense capsule报文数据
	rplidar_response_dense_capsule_measurement_nodes_t _cached_previous_dense_capsuledata;
	// 上一条报文的时间戳缓存
	_u64             _cached_last_data_timestamp_us;

	// 雷达时序参数缓存
	SlamtecLidarTimingDesc _cachedTimingDesc;

};


// =============================================================================
// UnpackerHandler_UltraDenseCapsuleNode —— 超密实版 Express 数据处理器
// -----------------------------------------------------------------------------
// 处理超密实版 EXPRESS_SCAN 命令（应答类型 0x86）的数据应答报文。
// 这是最高级的压缩格式，包含时间戳和设备状态信息。
// 每条报文包含64个采样点（32个cabin，每个cabin含2个采样点）。
// 使用可变比例编码的quality+distance压缩方案（4级比例编码）。
//
// 报文结构与密实版类似，但增加了：
// - 时间戳字段
// - 每个采样点额外编码quality（信号质量）
// - 使用4级可变比例编码压缩距离数据
// =============================================================================
class UnpackerHandler_UltraDenseCapsuleNode : public IDataUnpackerHandler {
public:
	// 构造函数
	UnpackerHandler_UltraDenseCapsuleNode();
	// 析构函数
	virtual ~UnpackerHandler_UltraDenseCapsuleNode();

	// 返回应答类型 0x86 (RPLIDAR_ANS_TYPE_MEASUREMENT_ULTRA_DENSE_CAPSULED)
	virtual _u8 getSampleAnswerType() const;
	// 逐字节接收并解析超密实版Express报文（核心解码入口）
	virtual void onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t size);
	// 重置handler状态
	virtual void reset();
	// 接收时序参数上下文
	virtual void onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size);
protected:
	// _onScanNodeUltraDenseCapsuleData: 解码超密实版ultra dense capsule报文数据
	// 使用4级可变比例编码解码quality和distance，角度由线性插值计算
	void _onScanNodeUltraDenseCapsuleData(rplidar_response_ultra_dense_capsule_measurement_nodes_t&, LIDARSampleDataUnpackerInner* engine);

	// 接收缓存
	std::vector<_u8> _cached_scan_node_buf;
	// 当前接收位置索引
	int              _cached_scan_node_buf_pos;
	// 上一条ultra dense capsule报文是否已就绪
	bool             _is_previous_capsuledataRdy;

	// 缓存的上一条ultra dense capsule报文数据
	rplidar_response_ultra_dense_capsule_measurement_nodes_t _cached_previous_ultra_dense_capsuledata;
	// 上一条报文的时间戳缓存
	_u64             _cached_last_data_timestamp_us;



	// 上一个节点的同步标志位，用于确保syncBit精确检测
	int              _last_node_sync_bit;
	// 上一个节点的距离值（q2格式），用于近距离平滑滤波
	int              _last_dist_q2;

	// 雷达时序参数缓存
	SlamtecLidarTimingDesc _cachedTimingDesc;
};


}

END_DATAUNPACKER_NS()