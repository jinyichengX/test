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
  *  本文件实现 UnpackerHandler_NormalNode 类，处理标准 SCAN 模式
  *  （应答类型 0x81）的数据应答报文解码。
  *
  *  标准 SCAN 报文为 5 字节/采样点（【通信协议 LR001 P.14-16】）：
  *    字节0: Quality(6bit) | S_bar(1bit) | S(1bit)
  *    字节1: angle_q6[6:0] | C(1bit)        C永远为1
  *    字节2: angle_q6[14:7]                 实际角度=angle_q6/64.0度
  *    字节3: distance_q2[7:0]
  *    字节4: distance_q2[15:8]              实际距离=distance_q2/4.0mm
  *
  *  解码流程采用逐字节状态机：
  *    位置0: 验证 S 与 S_bar 互反 → (S ^ S_bar) & 0x1 == 1
  *    位置1: 验证校验位 C → 最高位必须为1 (RPLIDAR_RESP_MEASUREMENT_CHECKBIT)
  *    位置2-4: 直接接收剩余字节
  *    位置4: 报文完整，执行格式转换并发布
  *
  *  格式转换（角度）：
  *    HQ格式使用 q14 定点数表示角度（angle_z_q14），
  *    标准 SCAN 使用 q6 定点数（angle_q6），需要转换：
  *    angle_z_q14 = (angle_q6 << 8) / 90
  *    这等价于 angle_q6 * 256 / 90 ≈ angle_q6 * 2.844
  *    因为 q14 = q6 * 2^(14-6) = q6 * 256，但还需除以90来对齐角度单位
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


// 引入本 handler 的头文件声明
#include "handler_normalnode.h"

BEGIN_DATAUNPACKER_NS()

namespace unpacker{


// -----------------------------------------------------------------------------
// _getSampleDelayOffsetInLegacyMode: 计算标准 SCAN 模式下的采样延迟补偿值
// -----------------------------------------------------------------------------
// RPLIDAR 从激光发射到数据被外部系统接收之间存在多种延迟。
// 为了获得准确的采样时间戳，需要从当前时间戳中扣除这些延迟。
//
// 总延迟 = 采样滤波延迟 + 采样持续时间的一半 + 传输延迟 + 链路延迟
//
// 各分量说明：
//   - sampleFilterDelay (sample_duration_uS): 采样滤波处理耗时
//   - sampleDelay (sample_duration_uS / 2): 采样持续时间的中心点
//     （因为采样时间戳应指向采样窗口的中心而非结束时刻）
//   - tranmissionDelay: 数据从雷达通过串口/网络传输到主机的时间
//   - linkage_delay_uS: 链路额外延迟（如 USB 转接等）
//
// 参数: timing - 雷达时序参数描述符
// 返回: 总延迟补偿值（微秒）
// -----------------------------------------------------------------------------
static _u64 _getSampleDelayOffsetInLegacyMode(const SlamtecLidarTimingDesc& timing)
{
    // 根据雷达型号猜测通信波特率，默认115200（标准UART）
    const _u64 channelBaudRate = timing.native_baudrate? timing.native_baudrate:115200;

    // 传输延迟：5字节报文 * 10bit/byte(含起始停止位) / 波特率 * 1000000(转微秒)
    _u64 tranmissionDelay = 1000000ULL * sizeof(rplidar_response_measurement_node_t) * 10 / channelBaudRate;

    // 以太网接口使用固定值（以太网延迟远小于串口且不易精确计算）
    if (timing.native_interface_type == LIDARInterfaceType::LIDAR_INTERFACE_ETHERNET)
    {
        tranmissionDelay = 100; //dummy value
    }

    // 采样持续时间的一半：取采样窗口中心点作为时间戳参考
    const _u64 sampleDelay = (timing.sample_duration_uS >> 1);
    // 采样滤波延迟：信号处理和滤波的耗时
    const _u64 sampleFilterDelay = timing.sample_duration_uS;

    return sampleFilterDelay + sampleDelay + tranmissionDelay + timing.linkage_delay_uS;
}

// -----------------------------------------------------------------------------
// 构造函数：初始化接收缓存和状态
// -----------------------------------------------------------------------------
UnpackerHandler_NormalNode::UnpackerHandler_NormalNode()
    : _cached_scan_node_buf_pos(0)
{
    // 预分配5字节缓存，对应 rplidar_response_measurement_node_t 结构大小
    _cached_scan_node_buf.resize(sizeof(rplidar_response_measurement_node_t));
    // 清零时序参数缓存
    memset(&_cachedTimingDesc, 0, sizeof(_cachedTimingDesc));
;}

// 析构函数（空实现）
UnpackerHandler_NormalNode::~UnpackerHandler_NormalNode()
{

}

// -----------------------------------------------------------------------------
// getSampleAnswerType: 返回该 handler 处理的应答类型
// RPLIDAR_ANS_TYPE_MEASUREMENT = 0x81，对应标准 SCAN 命令的数据应答
// 【通信协议 LR001 P.14-16】
// -----------------------------------------------------------------------------
_u8 UnpackerHandler_NormalNode::getSampleAnswerType() const
{
    return RPLIDAR_ANS_TYPE_MEASUREMENT;
}

// -----------------------------------------------------------------------------
// onData: 逐字节接收并解析标准 SCAN 数据应答报文（核心解码方法）
// -----------------------------------------------------------------------------
// 采用状态机方式，逐字节处理输入数据流。
// _cached_scan_node_buf_pos 记录当前接收位置（0-4），各位置的含义：
//
//   位置0: 验证起始字节的 S/S_bar 互反关系
//     - 标准 SCAN 报文字节0包含: Quality(6bit) | S_bar(1bit) | S(1bit)
//     - S 是扫描起始标志，S_bar 是 S 的取反，始终 S_bar = !S
//     - 通过检查 S 和 S_bar 互反来验证字节0的有效性
//     - 验证方法: tmp = data >> 1; (tmp ^ data) & 0x1 == 1 表示S和S_bar互反
//     - 如果不互反则丢弃此字节，继续等待有效起始字节
//
//   位置1: 验证校验位 C
//     - 字节1最高位为校验位C，永远为1（RPLIDAR_RESP_MEASUREMENT_CHECKBIT）
//     - 如果最高位为0，说明数据流可能错位，重置到位置0重新同步
//
//   位置2-3: 直接接收剩余字节
//
//   位置4(最后一个字节): 完整报文组装完毕，执行解码和格式转换
//     - 将 q6 格式角度转换为 q14 格式: angle_z_q14 = (angle_q6 << 8) / 90
//     - 提取同步标志位 S 作为 HQ flag
//     - 将 quality 从 6bit(0-63) 扩展到 8bit(0-255)
//     - 计算时间戳（扣除采样延迟补偿）并发布 HQ 节点
// -----------------------------------------------------------------------------
void UnpackerHandler_NormalNode::onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t cnt)
{
    // 逐字节处理输入数据
    for (size_t pos = 0; pos < cnt; ++pos) {
        _u8 current_data = data[pos];
        switch (_cached_scan_node_buf_pos) {
        case 0: // 位置0: 期望起始字节包含互反的同步位 S 和 S_bar
            // 字节0格式: Quality(6bit) | S_bar(1bit) | S(1bit) 【通信协议 LR001 P.14-16】
        {
            // 右移1位取出 S（最低位变为S_bar），S 在 bit0，S_bar 在 bit1
            _u8 tmp = (current_data >> 1);
            // 检查 S 和 S_bar 是否互反: (S ^ S_bar) & 0x1 == 1
            // 如果互反则通过，否则丢弃此字节继续等待
            if ((tmp ^ current_data) & 0x1) {
                // pass: S 和 S_bar 互反，字节有效
            }
            else {
                // S 和 S_bar 不互反，丢弃此字节
                continue;
            }

        }
        break;
        case 1: // 位置1: 期望校验位 C 为 1（字节1最高位）
            // 字节1格式: angle_q6[6:0] | C(1bit)，C 永远为1 【通信协议 LR001 P.15】
        {
            // 检查最高位是否为1（RPLIDAR_RESP_MEASUREMENT_CHECKBIT = 0x80）
            if (current_data & RPLIDAR_RESP_MEASUREMENT_CHECKBIT) {
                // pass: 校验位 C = 1，字节有效
            }
            else {
                // 校验位 C = 0，说明数据流可能错位
                // 重置到位置0重新寻找有效起始字节
                _cached_scan_node_buf_pos = 0;
                continue;
            }
        }
        break;
        case sizeof(rplidar_response_measurement_node_t) - 1: // 位置4: 最后一个字节，报文完整
        {
            // 存入最后一个字节完成5字节报文组装
            _cached_scan_node_buf[sizeof(rplidar_response_measurement_node_t) - 1] = current_data;
            // 重置接收位置，准备接收下一个报文
            _cached_scan_node_buf_pos = 0;

            // 将缓存中的5字节数据解释为标准SCAN测量节点结构
            rplidar_response_measurement_node_t* node = reinterpret_cast<rplidar_response_measurement_node_t*>(&_cached_scan_node_buf[0]);
#ifdef _CPU_ENDIAN_BIG
            // 大端CPU需要将小端数据转换为主机字节序（RPLIDAR使用小端发送【通信协议 LR001 P.6】）
            node->angle_q6_checkbit = le16_to_cpu(node->angle_q6_checkbit);
            node->distance_q2 = le16_to_cpu(node->distance_q2);
#endif
            // 将标准SCAN节点(node)转换为统一的高精度HQ节点(hqNode)格式
            rplidar_response_measurement_node_hq_t hqNode;

            // 角度转换: 从 q6 格式转换为 q14 格式
            // 先右移去掉校验位C(RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT=1)得到纯angle_q6，
            // 再左移8位转换为q14，最后除以90对齐角度单位
            // 公式: angle_z_q14 = (angle_q6 << 8) / 90
            // 实际角度 = angle_z_q14 / (2^14 / 360) = angle_z_q14 * 360 / 16384 度
            hqNode.angle_z_q14 = (((node->angle_q6_checkbit) >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) << 8) / 90;  //transfer to q14 Z-angle

            // 距离直接拷贝（HQ格式也使用q2定点数表示距离，单位mm）
            hqNode.dist_mm_q2 = node->distance_q2;

            // 同步标志: 提取S位作为HQ flag（S=1表示新的一圈扫描开始）
            // RPLIDAR_RESP_MEASUREMENT_SYNCBIT 提取字节0的最低位(S)
            hqNode.flag = (node->sync_quality & RPLIDAR_RESP_MEASUREMENT_SYNCBIT);  // trasfer syncbit to HQ flag field

            // 信号质量: 从6bit(0-63)扩展到8bit(0-255)
            // 先右移去掉低2位(S和S_bar)，再左移2位扩展到8bit范围
            // 这样使得 quality 的 0-63 映射到 0-252
            hqNode.quality = (node->sync_quality >> RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT) << RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT;  //remove the last two bits and then make quality from 0-63 to 0-255


            // 发布解包后的HQ节点，时间戳扣除采样延迟补偿
            engine->publishHQNode(engine->getCurrentTimestamp_uS() - _getSampleDelayOffsetInLegacyMode(_cachedTimingDesc), &hqNode);
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
void UnpackerHandler_NormalNode::onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size)
{
    if (type == LIDARSampleDataUnpacker::UNPACKER_CONTEXT_TYPE_LIDAR_TIMING) {
        // 验证数据大小匹配
        assert(size == sizeof(_cachedTimingDesc));
        // 缓存时序参数
        _cachedTimingDesc = *reinterpret_cast<const SlamtecLidarTimingDesc*>(data);
    }
}

// -----------------------------------------------------------------------------
// reset: 重置接收状态机
// 将接收位置归零，丢弃半截的报文数据，准备接收新的起始字节。
// -----------------------------------------------------------------------------
void UnpackerHandler_NormalNode::reset()
{
    _cached_scan_node_buf_pos = 0;
}
}


END_DATAUNPACKER_NS()