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
  *  本文件实现 UnpackerHandler_HQNode 类，处理 HQ 扫描模式
  *  （应答类型 0x83）的数据应答报文解码。
  *
  *  HQ 报文结构（rplidar_response_hq_capsule_measurement_nodes_t）：
  *    sync_byte (1字节)  : 同步标志 0xA5
  *    time_stamp (8字节) : 设备时间戳（微秒）
  *    node_hq[96]        : 96个高精度测量节点
  *    crc32 (4字节)      : CRC32校验值
  *
  *  解码流程：
  *    1. 等待同步字节 0xA5
  *    2. 逐字节接收完整报文
  *    3. 对除CRC外的所有数据计算CRC32
  *    4. 比较计算值与报文中的CRC32值
  *    5. 校验通过则逐个发布96个HQ节点，否则发布解码错误
  *
  *  CRC32实现支持两种方式（由编译开关选择）：
  *    - CONF_NO_BOOST_CRC_SUPPORT 定义时: 使用 SDK 自带的 sl_crc 实现
  *    - 否则: 使用 boost::crc_optimal<32, 0x04C11DB7, ...>
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
#include "../dataunnpacker_commondef.h"
#include "../dataunpacker.h"
#include "../dataunnpacker_internal.h"

// 如果定义了 CONF_NO_BOOST_CRC_SUPPORT，则引入 SDK 自带的 CRC32 实现
// 否则使用 Boost CRC 库
#ifdef CONF_NO_BOOST_CRC_SUPPORT
#include "sl_crc.h"
#endif

// 引入本 handler 的头文件声明
#include "handler_hqnode.h"

BEGIN_DATAUNPACKER_NS()

namespace unpacker{


// -----------------------------------------------------------------------------
// _getSampleDelayOffsetInHQMode: 计算HQ模式下的采样延迟补偿值
// -----------------------------------------------------------------------------
// 与标准SCAN模式的延迟计算类似，但HQ报文更大（包含96个节点），
// 传输延迟按单个HQ节点大小计算。
// 注意: 此处存在 FIXME 标记，表示该计算可能需要进一步评估精度。
//
// 总延迟 = 采样滤波延迟 + 采样持续时间的一半 + 传输延迟 + 链路延迟
//
// 参数: timing - 雷达时序参数描述符
// 返回: 总延迟补偿值（微秒）
// -----------------------------------------------------------------------------
static _u64 _getSampleDelayOffsetInHQMode(const SlamtecLidarTimingDesc& timing)
{
    // FIXME: to eval
    //
    // 根据雷达型号猜测通信波特率，HQ模式默认1000000（1Mbps，高于标准模式的115200）
    const _u64 channelBaudRate = timing.native_baudrate? timing.native_baudrate:1000000;

    // 传输延迟：单个HQ节点大小 * 10bit/byte / 波特率 * 1000000(转微秒)
    _u64 tranmissionDelay = 1000000ULL * sizeof(rplidar_response_measurement_node_hq_t) * 10 / channelBaudRate;

    // 以太网接口使用固定值
    if (timing.native_interface_type == LIDARInterfaceType::LIDAR_INTERFACE_ETHERNET)
    {
        tranmissionDelay = 100; //dummy value
    }

    // 采样持续时间的一半：取采样窗口中心点
    const _u64 sampleDelay = (timing.sample_duration_uS >> 1);
    // 采样滤波延迟
    const _u64 sampleFilterDelay = timing.sample_duration_uS;

    return sampleFilterDelay + sampleDelay + tranmissionDelay + timing.linkage_delay_uS;
}

// -----------------------------------------------------------------------------
// 构造函数：初始化接收缓存为HQ报文大小
// -----------------------------------------------------------------------------
UnpackerHandler_HQNode::UnpackerHandler_HQNode()
    : _cached_scan_node_buf_pos(0)
{
    // 预分配缓存，大小为HQ报文结构 rplidar_response_hq_capsule_measurement_nodes_t 的大小
    _cached_scan_node_buf.resize(sizeof(rplidar_response_hq_capsule_measurement_nodes_t));
    // 清零时序参数缓存
    memset(&_cachedTimingDesc, 0, sizeof(_cachedTimingDesc));
}

// 析构函数（空实现）
UnpackerHandler_HQNode::~UnpackerHandler_HQNode()
{

}

// -----------------------------------------------------------------------------
// getSampleAnswerType: 返回该 handler 处理的应答类型
// RPLIDAR_ANS_TYPE_MEASUREMENT_HQ = 0x83，对应 HQ 扫描模式的数据应答
// -----------------------------------------------------------------------------
_u8 UnpackerHandler_HQNode::getSampleAnswerType() const
{
    return RPLIDAR_ANS_TYPE_MEASUREMENT_HQ;
}

// -----------------------------------------------------------------------------
// onData: 逐字节接收并解析 HQ 报文（核心解码方法）
// -----------------------------------------------------------------------------
// HQ报文状态机解码流程：
//
//   位置0: 等待同步字节 0xA5 (RPLIDAR_RESP_MEASUREMENT_HQ_SYNC)
//     - 非同步字节则丢弃，继续等待
//
//   位置1 到 (sizeof-5): 逐字节接收报文数据体
//     - 包括 time_stamp(8字节) 和 node_hq[96]
//
//   位置 (sizeof-5): CRC计算准备阶段（无特殊操作，仅作为状态转移点）
//
//   位置 (sizeof-1): 最后一个字节，完整报文组装完毕
//     1. 计算CRC32（覆盖除CRC字段外的所有数据）
//     2. 比较计算值与报文中的CRC32
//     3. 校验通过: 遍历96个node_hq，逐个发布HQ节点
//     4. 校验失败: 发布 ERR_EVENT_ON_EXP_CHECKSUM_ERR 错误消息
//
// CRC32 计算方式（由编译开关选择）：
//   - CONF_NO_BOOST_CRC_SUPPORT: 使用 SDK 自带 crc32::getResult()
//   - 否则: 使用 boost::crc_optimal<32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true>
//     Boost方式需要将数据补齐到4字节倍数后计算
// -----------------------------------------------------------------------------
void UnpackerHandler_HQNode::onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t cnt)
{

    // 逐字节处理输入数据
    for (size_t pos = 0; pos < cnt; ++pos)
    {
        _u8 current_data = data[pos];

        switch (_cached_scan_node_buf_pos)
        {
        case 0: // 位置0: 等待HQ报文同步字节 0xA5
        {
            // 检查是否为同步字节 0xA5 (RPLIDAR_RESP_MEASUREMENT_HQ_SYNC)
            if (current_data == RPLIDAR_RESP_MEASUREMENT_HQ_SYNC) {
                // pass: 同步字节匹配，开始接收报文
            }
            else {
                // 非同步字节，丢弃，继续等待
                continue;
            }
        }
        break;

        case sizeof(rplidar_response_hq_capsule_measurement_nodes_t) - 1 - 4:    // 位置(N-5): CRC计算准备点
        {
            // 此位置标记报文数据体接收完毕（不含最后4字节CRC32）
            // 可在此处进行CRC计算的预处理，当前无特殊操作
        }
        break;

        case sizeof(rplidar_response_hq_capsule_measurement_nodes_t) - 1: // 位置(N-1): 最后一个字节，完整报文就绪
        {
            // 存入最后一个字节完成报文组装
            _cached_scan_node_buf[sizeof(rplidar_response_hq_capsule_measurement_nodes_t) - 1] = current_data;
            // 重置接收位置
            _cached_scan_node_buf_pos = 0;
            // 将缓存解释为HQ报文结构
            rplidar_response_hq_capsule_measurement_nodes_t* nodesData = reinterpret_cast<rplidar_response_hq_capsule_measurement_nodes_t*>(&_cached_scan_node_buf[0]);

#ifdef CONF_NO_BOOST_CRC_SUPPORT
            // 方式1: 使用SDK自带的CRC32实现
            // 对除最后4字节CRC外的所有数据计算CRC32
            _u32 crcCalc = crc32::getResult(&_cached_scan_node_buf[0], sizeof(sl_lidar_response_hq_capsule_measurement_nodes_t) - 4);


#else
            // 方式2: 使用Boost CRC32实现
            // 多项式 0x04C11DB7，初始值和最终异或值均为 0xFFFFFFFF
            boost::crc_optimal<32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true> mycrc;
            std::vector<_u8> crcInputData;
            crcInputData.resize(sizeof(rplidar_response_hq_capsule_measurement_nodes_t) - 4);
            memcpy(&crcInputData[0], nodesData, sizeof(rplidar_response_hq_capsule_measurement_nodes_t) - 4);
            // Boost CRC要求输入数据长度为4字节倍数，不足则补零
            int leftBytes = 4 - (crcInputData.size() & 3);
            for (int i = 0; i < leftBytes; i++)
                crcInputData.push_back(0);
            mycrc.process_bytes(&crcInputData[0], crcInputData.size());
            _u32 crcCalc = mycrc.checksum();

#endif

            // 获取报文中携带的CRC32值
            _u32 recvCRC = nodesData->crc32;
#ifdef _CPU_ENDIAN_BIG
            // 大端CPU需要将小端数据转换为主机字节序
            recvCRC = le32_to_cpu(recvCRC);
            nodesData->time_stamp = le64_to_cpu(nodesData->time_stamp);
#endif
            // 比较计算CRC与接收CRC
            if (recvCRC == crcCalc)
            {
                // CRC校验通过：遍历96个HQ节点逐个发布
                for (size_t pos = 0; pos < _countof(nodesData->node_hq); ++pos)
                {
                    // 拷贝节点数据（HQ节点已经是高精度格式，无需额外转换）
                    rplidar_response_measurement_node_hq_t hqNode = nodesData->node_hq[pos];
#ifdef _CPU_ENDIAN_BIG
                    // 大端CPU字节序转换
                    hqNode.angle_z_q14 = le16_to_cpu(hqNode.angle_z_q14);
                    hqNode.dist_mm_q2 = le32_to_cpu(hqNode.dist_mm_q2);
#endif
                    // 发布HQ节点，时间戳扣除采样延迟补偿
                    engine->publishHQNode(engine->getCurrentTimestamp_uS() - _getSampleDelayOffsetInHQMode(_cachedTimingDesc), &hqNode);
                }
            }
            else  // CRC校验未通过：报文可能在传输中损坏
            {
                // 发布校验和错误消息
                engine->publishDecodingErrorMsg(LIDARSampleDataUnpacker::ERR_EVENT_ON_EXP_CHECKSUM_ERR
                    , RPLIDAR_ANS_TYPE_MEASUREMENT_HQ, nodesData, sizeof(*nodesData));
            }
            continue;
        }
        break;


        }
        // 将当前字节存入缓存并前进到下一个位置
        _cached_scan_node_buf[_cached_scan_node_buf_pos++] = current_data;
    }

}


// -----------------------------------------------------------------------------
// onUnpackerContextSet: 接收解包器上下文更新
// 当收到时序参数更新时，缓存 SlamtecLidarTimingDesc 用于延迟补偿计算。
// -----------------------------------------------------------------------------
void UnpackerHandler_HQNode::onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size)
{
    if (type == LIDARSampleDataUnpacker::UNPACKER_CONTEXT_TYPE_LIDAR_TIMING) {
        assert(size == sizeof(_cachedTimingDesc));
        _cachedTimingDesc = *reinterpret_cast<const SlamtecLidarTimingDesc*>(data);
    }
}

// -----------------------------------------------------------------------------
// reset: 重置接收状态机
// 将接收位置归零，丢弃半截的报文数据，准备重新等待同步字节。
// -----------------------------------------------------------------------------
void UnpackerHandler_HQNode::reset()
{
    _cached_scan_node_buf_pos = 0;
}
}


END_DATAUNPACKER_NS()