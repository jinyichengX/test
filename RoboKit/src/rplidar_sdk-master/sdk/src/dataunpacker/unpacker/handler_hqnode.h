/*
 *  Slamtec LIDAR SDK
 *
 *  Copyright (c) 2014 - 2023 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */

 /*
  *  Sample Data Unpacker System
  *  HQNode Sample Node Handler
  *
  *  本文件声明了 UnpackerHandler_HQNode 类，负责处理 HQ 扫描模式
  *  （应答类型 0x83，RPLIDAR_ANS_TYPE_MEASUREMENT_HQ）的数据应答报文。
  *
  *  HQ（High Quality）模式是 RPLIDAR 的高精度扫描模式，具有以下特点：
  *  - 使用 CRC32 校验保证数据完整性（其他模式使用简单的异或校验和）
  *  - 每条报文包含 96 个采样点（远多于标准SCAN的每报文1个点）
  *  - 报文自带 8 字节时间戳，无需外部计算时间
  *  - 角度和距离直接使用高精度定点数格式，无需额外转换
  *
  *  HQ 报文结构（rplidar_response_hq_capsule_measurement_nodes_t）：
  *    - sync_byte (1字节): 同步标志，固定为 0xA5 (RPLIDAR_RESP_MEASUREMENT_HQ_SYNC)
  *    - time_stamp (8字节): 设备时间戳（微秒级）
  *    - node_hq[96] (96 * sizeof(hq_node) 字节): 96个高精度测量节点
  *    - crc32 (4字节): CRC32校验值（覆盖除CRC自身外的所有数据）
  *
  *  解码流程：
  *    位置0: 等待同步字节 0xA5
  *    位置1~(N-5): 接收报文数据
  *    位置(N-1): 完整报文，验证CRC32，通过则逐个发布96个节点
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
	// UnpackerHandler_HQNode —— HQ 扫描模式数据处理器
	// -----------------------------------------------------------------------------
	// 处理 HQ 扫描模式（应答类型 0x83）的数据应答报文。
	// HQ 报文使用 CRC32 校验，每条报文包含 96 个高精度测量节点。
	//
	// 解码状态机：
	//   - 位置0: 等待同步字节 0xA5（RPLIDAR_RESP_MEASUREMENT_HQ_SYNC）
	//   - 位置1到(N-5): 逐字节接收报文数据体
	//   - 位置(N-1): 完整报文组装完毕，执行CRC32校验
	//     校验通过: 逐个发布96个HQ节点
	//     校验失败: 发布解码错误消息
	// =============================================================================
	class UnpackerHandler_HQNode : public IDataUnpackerHandler {
	public:
		// 构造函数：初始化接收缓存为HQ报文大小
		UnpackerHandler_HQNode();
		// 析构函数
		virtual ~UnpackerHandler_HQNode();

		// 返回应答类型 0x83 (RPLIDAR_ANS_TYPE_MEASUREMENT_HQ)
		virtual _u8 getSampleAnswerType() const;
		// 逐字节接收并解析HQ报文数据（核心解码入口）
		virtual void onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t size);
		// 重置接收状态机到初始位置
		virtual void reset();
		// 接收时序参数上下文，用于计算采样延迟补偿
		virtual void onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size);

	protected:
		// 接收缓存：用于逐字节组装完整的HQ报文
		// 大小为 sizeof(rplidar_response_hq_capsule_measurement_nodes_t)
		std::vector<_u8> _cached_scan_node_buf;
		// 当前接收位置索引，指示下一个字节存入缓存的哪个位置
		int              _cached_scan_node_buf_pos;
		// 缓存的雷达时序参数描述符，用于计算采样延迟补偿
		SlamtecLidarTimingDesc _cachedTimingDesc;
	};

}

END_DATAUNPACKER_NS()