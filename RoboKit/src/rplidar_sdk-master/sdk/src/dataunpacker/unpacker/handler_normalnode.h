/*
 *  Slamtec LIDAR SDK
 *
 *  Copyright (c) 2014 - 2023 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */

 /*
  *  Sample Data Unpacker System
  *  Normal Sample Node Handler
  *
  *  本文件声明了 UnpackerHandler_NormalNode 类，负责处理标准 SCAN 命令
  *  （应答类型 0x81）的数据应答报文。
  *
  *  标准 SCAN 模式下，每个测距采样点通过一个 5 字节的数据应答报文发送
  *  【通信协议 LR001 P.14-16】。报文结构如下：
  *
  *    字节0: Quality(6bit) | S_bar(1bit) | S(1bit)
  *           - S=扫描起始标志，S=1表示新的一圈360度扫描的开始
  *           - S_bar=S的取反，始终S_bar=!S，用于数据校验
  *           - Quality=采样点信号质量
  *
  *    字节1: angle_q6[6:0] | C(1bit校验位)
  *           - C=校验位，永远为1，用于起始字节判断和数据校验
  *           - angle_q6低7位
  *
  *    字节2: angle_q6[14:7]
  *           - angle_q6高8位，实际角度 = angle_q6 / 64.0 度
  *
  *    字节3: distance_q2[7:0]
  *           - distance_q2低8位
  *
  *    字节4: distance_q2[15:8]
  *           - distance_q2高8位，实际距离 = distance_q2 / 4.0 毫米
  *           - distance_q2=0表示无效点
  *
  *  本 handler 逐字节接收数据，通过检查 S/S_bar 互反关系和校验位 C
  *  来识别报文边界，组装完整的 5 字节报文后解码为 HQ 格式测量节点。
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

namespace unpacker{

// =============================================================================
// UnpackerHandler_NormalNode —— 标准 SCAN 模式数据处理器
// -----------------------------------------------------------------------------
// 处理标准 SCAN 命令（0x20）对应的数据应答报文（应答类型 0x81）。
// 每个测距采样点使用 5 字节的数据应答报文发送（【通信协议 LR001 P.14-16】）。
//
// 该 handler 采用逐字节状态机方式接收数据：
//   - 位置0: 检查 S/S_bar 互反关系验证起始字节有效性
//   - 位置1: 检查校验位 C（最高位必须为1）
//   - 位置2-3: 接收剩余数据字节
//   - 位置4: 完整报文组装完毕，执行解码并发布 HQ 节点
// =============================================================================
class UnpackerHandler_NormalNode : public IDataUnpackerHandler {
public:
	// 构造函数：初始化接收缓存为5字节（rplidar_response_measurement_node_t 大小）
	UnpackerHandler_NormalNode();
	// 析构函数
	virtual ~UnpackerHandler_NormalNode();

	// 返回应答类型 0x81 (RPLIDAR_ANS_TYPE_MEASUREMENT)
	virtual _u8 getSampleAnswerType() const;
	// 逐字节接收并解析标准SCAN数据应答报文（核心解码入口）
	virtual void onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t size);
	// 重置接收状态机到初始位置
	virtual void reset();
	// 接收时序参数上下文，用于计算采样延迟补偿
	virtual void onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size);
protected:
	// 接收缓存：用于逐字节组装完整的5字节标准SCAN数据应答报文
	std::vector<_u8> _cached_scan_node_buf;
	// 当前接收位置索引（0-4），指示下一个字节存入缓存的哪个位置
	int              _cached_scan_node_buf_pos;

	// 缓存的雷达时序参数描述符，包含采样持续时间、传输波特率等
	// 用于在发布节点时计算采样延迟补偿（_getSampleDelayOffsetInLegacyMode）
	SlamtecLidarTimingDesc _cachedTimingDesc;
};

}

END_DATAUNPACKER_NS()