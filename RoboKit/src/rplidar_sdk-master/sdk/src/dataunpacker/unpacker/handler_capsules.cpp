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
  *  本文件是数据解包器中最大、最重要的实现文件，实现了 4 种 Capsule（胶囊）
  *  格式的高速扫描数据解包器。这些格式对应 EXPRESS_SCAN 命令的不同版本
  *  应答数据，通过压缩冗余数据将 4kHz 及以上采样率的数据在有限带宽链路
  *  （如115200bps UART）上传输【通信协议 LR001 P.17-28】。
  *
  *  4 种 Capsule 格式按代码顺序：
  *
  *  1. CapsuleNode (0x82) —— 传统版 Express
  *     报文84字节，16个cabin(5字节each)，每cabin含2个采样点，共32个采样点
  *     每个cabin含: distance1(14bit) + distance2(14bit) + dθ1(4bit) + dθ2(4bit) + offset_angles(8bit)
  *     角度公式: θk = ωi + AngleDiff(ωi,ωi+1)/32 * k - dθk
  *     AngleDiff(ωi,ωi+1) = ωi+1-ωi (如果ωi<=ωi+1) 或 360+ωi+1-ωi (如果ωi>ωi+1)
  *     【通信协议 LR001 P.17-20, P.27】
  *
  *  2. UltraCapsuleNode (0x84) —— 扩展版 Express
  *     报文132字节，32个ultra_cabin(4字节each)，每ultra_cabin含3个采样点，共96个采样点
  *     ultra_cabin结构: major(12bit可变比例编码) + predict1(10bit预测编码) + predict2(10bit预测编码)
  *     使用SLAMTEC专利压缩编码(CN 108306649 A)
  *     角度补偿使用光学模型经验公式计算
  *     【通信协议 LR001 P.22-23】
  *
  *  3. DenseCapsuleNode (0x85) —— 密实版 Express
  *     报文84字节，40个cabin(2字节each)，每cabin含1个采样点，共40个采样点
  *     每个cabin只含distance(16bit)，不编码角度补偿
  *     角度公式: θk = ωi + AngleDiff(ωi,ωi+1)/40 * k
  *     【通信协议 LR001 P.24-25, P.27-28】
  *
  *  4. UltraDenseCapsuleNode (0x86) —— 超密实版 Express
  *     更高级的压缩格式，包含时间戳和设备状态
  *     每条报文含64个采样点（32个cabin * 2采样点/cabin）
  *     使用4级可变比例编码压缩quality和distance
  *
  *  通用同步与校验机制（所有Capsule版本共享）：
  *    - 报文前2字节高4位为同步标志: sync1=0xA, sync2=0x5
  *    - 前2字节低4位为校验和ChkSum(8bit，拆分为两个4bit半字节)
  *    - ChkSum = 对报文中除sync字段外的所有数据按字节异或
  *    - 第3-4字节为start_angle_q6(15bit) + S(1bit起始标志)
  *    - S=1表示新的一圈扫描开始或编码器重置，外部系统应重新开始解析
  *      【通信协议 LR001 P.27-28】
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

// 引入4种capsule handler的头文件声明
#include "handler_capsules.h"

BEGIN_DATAUNPACKER_NS()

namespace unpacker{


// =============================================================================
// UnpackerHandler_CapsuleNode 实现 —— 传统版 Express 数据处理器
// 报文84字节, 16个cabin(5字节each), 每cabin含2采样点, 共32采样点
// 角度公式: θk = ωi + AngleDiff(ωi,ωi+1)/32 * k - dθk 【通信协议 LR001 P.17-20, P.27】
// =============================================================================

// -----------------------------------------------------------------------------
// _getSampleDelayOffsetInExpressMode: 计算传统版 Express 模式下第 sampleIdx 个
// 采样点相对于报文接收时间戳的延迟补偿量（微秒）
// -----------------------------------------------------------------------------
// 延迟模型说明:
//   报文到达主机时，最后一个字节对应的时间戳为 engine->getCurrentTimestamp_uS()。
//   但报文内 32 个采样点是在不同时刻采集的，越靠前的采样点采集越早，延迟越大。
//   需要补偿的延迟 = 采样滤波延迟 + 采样持续时间/2 + 传输延迟 + 链路延迟 + 分组延迟
//
//   - sampleFilterDelay: 一阶IIR滤波器的群延迟，约等于1个采样周期
//   - sampleDelay: 采样积分时间的中点补偿（采样持续时间/2）
//   - tranmissionDelay: 报文从雷达发出到主机接收的传输延迟
//   - linkage_delay_uS: 链路层固定延迟（如UART驱动缓冲延迟）
//   - groupingDelay: 分组延迟，32个采样点中越靠后的点等待时间越短
//     公式: (31 - sampleIdx) * sample_duration_uS
//     sampleIdx 范围 [0, 31]，第0个采样点延迟最大（等待全部31个后续采样）
// -----------------------------------------------------------------------------
static _u64 _getSampleDelayOffsetInExpressMode(const SlamtecLidarTimingDesc& timing, int sampleIdx)
{
    // FIXME: to eval
    // 根据雷达型号猜测波特率，传统版Express默认115200
    const _u64 channelBaudRate = timing.native_baudrate? timing.native_baudrate:115200;

    // 传输延迟 = 1e6 * 报文字节数 * 10(起始位+8数据+停止位) / 波特率
    // rplidar_response_capsule_measurement_nodes_t = 84 字节
    _u64 tranmissionDelay = 1000000ULL * sizeof(rplidar_response_capsule_measurement_nodes_t) * 10 / channelBaudRate;

    // 以太网接口的传输延迟远小于UART，使用固定值
    if (timing.native_interface_type == LIDARInterfaceType::LIDAR_INTERFACE_ETHERNET)
    {
        tranmissionDelay = 100; //dummy value
    }

    // 采样积分时间的中点补偿（取采样持续时间的一半）
    const _u64 sampleDelay = (timing.sample_duration_uS >> 1);
    // IIR滤波器群延迟，约等于一个完整的采样周期
    const _u64 sampleFilterDelay = timing.sample_duration_uS;
    // 分组延迟: 32个采样点中第sampleIdx个点相对最后一个点的等待时间
    // sampleIdx=0时延迟最大(31个采样周期)，sampleIdx=31时延迟为0
    const _u64 groupingDelay = (31 - sampleIdx) * timing.sample_duration_uS;


    return sampleFilterDelay + sampleDelay + tranmissionDelay + timing.linkage_delay_uS + groupingDelay;
}


// -----------------------------------------------------------------------------
// 构造函数: 初始化接收缓冲区、缓存状态和定时描述
// -----------------------------------------------------------------------------
UnpackerHandler_CapsuleNode::UnpackerHandler_CapsuleNode()
    : _cached_scan_node_buf_pos(0)           // 接收状态机位置归零
    , _is_previous_capsuledataRdy(false)    // 尚无上一条报文缓存（首帧无法解码角度）
    , _cached_last_data_timestamp_us(0)    // 上次报文时间戳归零
{
    // 预分配接收缓冲区，大小为传统版capsule报文长度(84字节)
    _cached_scan_node_buf.resize(sizeof(rplidar_response_capsule_measurement_nodes_t));
    // 清零定时描述结构体
    memset(&_cachedTimingDesc, 0, sizeof(_cachedTimingDesc));
}

// 析构函数: 无需手动释放资源（vector自动析构）
UnpackerHandler_CapsuleNode::~UnpackerHandler_CapsuleNode()
{

}

// -----------------------------------------------------------------------------
// onUnpackerContextSet: 接收外部传入的上下文配置
// 目前仅支持 LIDAR_TIMING 类型，用于设置采样定时参数（采样周期等）
// -----------------------------------------------------------------------------
void UnpackerHandler_CapsuleNode::onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size)
{
    if (type == LIDARSampleDataUnpacker::UNPACKER_CONTEXT_TYPE_LIDAR_TIMING) {
        assert(size == sizeof(_cachedTimingDesc));
        // 将传入的 SlamtecLidarTimingDesc 复制到缓存
        _cachedTimingDesc = *reinterpret_cast<const SlamtecLidarTimingDesc*>(data);
    }
}

// -----------------------------------------------------------------------------
// getSampleAnswerType: 返回此handler处理的数据应答类型
// RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED = 0x82，对应传统版 EXPRESS_SCAN 应答
// 【通信协议 LR001 P.17】起始应答: A5 5A 54 00 00 40 82，数据应答长度 84 bytes
// -----------------------------------------------------------------------------
_u8 UnpackerHandler_CapsuleNode::getSampleAnswerType() const
{
    return RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED;
}

// -----------------------------------------------------------------------------
// onData: 逐字节接收并解析传统版 Express capsule 报文
// -----------------------------------------------------------------------------
// 报文格式【通信协议 LR001 P.17-20, P.27】:
//   字节0: [sync1=0xA(高4位)] [ChkSum[3:0](低4位)]
//   字节1: [sync2=0x5(高4位)] [ChkSum[7:4](低4位)]
//   字节2: [start_angle_q6[7:0]]
//   字节3: [S(1bit)] [start_angle_q6[14:8]]
//   字节4~83: 16个cabin(每个5字节)，共80字节
//   总计: 84字节
//
// 状态机逻辑:
//   _cached_scan_node_buf_pos 跟踪当前接收位置:
//     0: 等待sync1(高4位=0xA)
//     1: 等待sync2(高4位=0x5)
//     2~82: 接收报文体(start_angle + 16个cabin)
//     83: 最后一个字节，报文完整，执行校验和解码
// -----------------------------------------------------------------------------
void UnpackerHandler_CapsuleNode::onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t cnt)
{
    // 逐字节处理输入数据流
    for (size_t pos = 0; pos < cnt; ++pos) {
        _u8 current_data = data[pos];
        switch (_cached_scan_node_buf_pos) {
        case 0: // 状态0: 等待同步标志1（高4位应为0xA）
        {
            _u8 tmp = (current_data >> 4);
            if (tmp == RPLIDAR_RESP_MEASUREMENT_EXP_SYNC_1) {
                // 高4位匹配0xA，继续接收下一字节
            }
            else {
                // 不匹配，丢弃当前字节，标记上一条报文失效
                // （因为连续性被破坏，角度差值计算不再有效）
                _is_previous_capsuledataRdy = false;
                continue;
            }

        }
        break;
        case 1: // 状态1: 等待同步标志2（高4位应为0x5）
        {
            _u8 tmp = (current_data >> 4);
            if (tmp == RPLIDAR_RESP_MEASUREMENT_EXP_SYNC_2) {
                // 高4位匹配0x5，sync1+sync2验证通过，报文起始已确认
            }
            else {
                // sync2不匹配，重置状态机回到等待sync1
                _cached_scan_node_buf_pos = 0;
                _is_previous_capsuledataRdy = false;
                continue;
            }
        }
        break;

        case sizeof(rplidar_response_capsule_measurement_nodes_t) - 1: // 状态83: 收到最后一个字节，报文完整
        {
            _cached_scan_node_buf[sizeof(rplidar_response_capsule_measurement_nodes_t) - 1] = current_data;
            _cached_scan_node_buf_pos = 0;

            // 将缓冲区重解释为capsule报文结构体
            rplidar_response_capsule_measurement_nodes_t* node = reinterpret_cast<rplidar_response_capsule_measurement_nodes_t*>(&_cached_scan_node_buf[0]);

            // 校验和计算:
            // 1. 提取报文中的校验和: s_checksum_1低4位为ChkSum[3:0]，s_checksum_2低4位为ChkSum[7:4]
            _u8 checksum = 0;
            _u8 recvChecksum = ((node->s_checksum_1 & 0xF) | (node->s_checksum_2 << 4));
            // 2. 从start_angle_sync_q6字段开始，对报文中所有数据按字节异或
            //    （跳过sync+checksum的前2字节，因为它们包含的是同步标志和校验和本身）
            for (size_t cpos = offsetof(rplidar_response_capsule_measurement_nodes_t, start_angle_sync_q6);
                cpos < sizeof(rplidar_response_capsule_measurement_nodes_t); ++cpos)
            {
                checksum ^= _cached_scan_node_buf[cpos];
            }

            if (recvChecksum == checksum)
            {
                // 校验和匹配，报文有效

                // 大端CPU需要将小端数据转换为主机字节序
                // 报文中的所有多字节字段均采用小端序（LE）
#ifdef _CPU_ENDIAN_BIG
                node->start_angle_sync_q6 = le16_to_cpu(node->start_angle_sync_q6);
                for (size_t cpos = 0; cpos < _countof(node->cabins); ++cpos) {
                    node->cabins[cpos].distance_angle_1 = le16_to_cpu(node->cabins[cpos].distance_angle_1);
                    node->cabins[cpos].distance_angle_2 = le16_to_cpu(node->cabins[cpos].distance_angle_2);
                }
#endif
                // 检查起始标志位S（bit15 of start_angle_sync_q6）
                // 【通信协议 LR001 P.27-28】S=1表示新的一圈扫描开始或编码器重置
                if (node->start_angle_sync_q6 & RPLIDAR_RESP_MEASUREMENT_EXP_SYNCBIT)
                {
                    // 如果之前已有缓存报文，说明发生意外重置，发布解码错误
                    if (_is_previous_capsuledataRdy) {
                        engine->publishDecodingErrorMsg(LIDARSampleDataUnpacker::ERR_EVENT_ON_EXP_ENCODER_RESET
                            , RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED, node, sizeof(*node));
                    }
                    // 丢弃上一条报文缓存，因为角度连续性被S标志打断
                    _is_previous_capsuledataRdy = false;
                    // 通知外部系统: 新的一圈扫描开始
                    engine->publishNewScanReset();


                }
                // 调用解码函数，将capsule报文解包为HQ节点并发布
                _onScanNodeCapsuleData(*node, engine);
            }
            else {
                // 校验和不匹配，报文损坏，丢弃缓存
                _is_previous_capsuledataRdy = false;

                // 发布解码错误事件，供外部系统记录或处理
                engine->publishDecodingErrorMsg(LIDARSampleDataUnpacker::ERR_EVENT_ON_EXP_CHECKSUM_ERR
                    , RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED, node, sizeof(*node));

            }
            continue;
        }
        break;

        }
        // 将当前字节存入缓冲区并推进位置计数器
        _cached_scan_node_buf[_cached_scan_node_buf_pos++] = current_data;
    }

}

// -----------------------------------------------------------------------------
// reset: 重置handler状态，在停止扫描或切换模式时调用
// 清空接收状态机、丢弃上一条报文缓存、重置时间戳
// -----------------------------------------------------------------------------
void UnpackerHandler_CapsuleNode::reset()
{
    _cached_scan_node_buf_pos = 0;
    _is_previous_capsuledataRdy = false;
    _cached_last_data_timestamp_us = 0;
}

// -----------------------------------------------------------------------------
// _onScanNodeCapsuleData: 解码传统版capsule报文，计算角度并发布HQ节点
// -----------------------------------------------------------------------------
// 角度计算过程(定点数运算)【通信协议 LR001 P.27】:
//
// 1. 计算角度差值 AngleDiff:
//    - currentStartAngle_q8 = (capsule.start_angle_sync_q6 & 0x7FFF) << 2
//      (取15位角度值，从q6左移2位转为q8格式)
//    - prevStartAngle_q8 = 上一条报文的start_angle（同样处理）
//    - diffAngle_q8 = currentStartAngle_q8 - prevStartAngle_q8
//    - 若prevStartAngle > currentStartAngle（跨0度）:
//      diffAngle_q8 += 360<<8 (加上360度的q8值)
//    对应公式: AngleDiff(ωi,ωi+1) = ωi+1-ωi (ωi<=ωi+1)
//                            或 360+ωi+1-ωi (ωi>ωi+1)
//
// 2. 计算角度增量:
//    angleInc_q16 = diffAngle_q8 << 3 (q8->q16，每采样点角度增量)
//    对应公式中的 AngleDiff/32 * k 中的 AngleDiff/32 部分
//
// 3. 遍历16个cabin(每cabin含2采样点，共32点):
//    a. 距离提取: dist_q2 = distance_angle & 0xFFFC
//       (distance_angle字段低14位为距离q2值，低2位用于角度补偿)
//    b. 角度补偿dθ提取(4bit, q3格式):
//       - dθ1 = offset_angles_q3低4位 | (distance_angle_1低2位 << 4)
//       - dθ2 = offset_angles_q3高4位 | (distance_angle_2低2位 << 4)
//    c. 角度计算: angle_q6 = (currentAngle_q16 - (dθ_q3 << 13)) >> 10
//       对应公式: θk = ωi + AngleDiff/32 * k - dθk
//    d. 同步位检测: 当角度跨越360度边界时syncBit=1
//
// 4. HQ格式转换:
//    - angle_z_q14 = (angle_q6 << 8) / 90 (q6->q14)
//    - dist_mm_q2 = dist_q2 (距离保持q2格式)
//    - flag: bit0=syncBit, bit1=!syncBit
//    - quality: 有距离时设为固定值0x2F << shift
// -----------------------------------------------------------------------------
void UnpackerHandler_CapsuleNode::_onScanNodeCapsuleData(rplidar_response_capsule_measurement_nodes_t& capsule, LIDARSampleDataUnpackerInner* engine)
{
    _u64 currentTS = engine->getCurrentTimestamp_uS();
    if (_is_previous_capsuledataRdy) {
        // 有上一条报文缓存，可以计算角度差值
        int diffAngle_q8;
        // 提取当前报文和上一条报文的start_angle，转为q8格式（左移2位）
        // start_angle_sync_q6的bit15是S标志，需要屏蔽（&0x7FFF）
        int currentStartAngle_q8 = ((capsule.start_angle_sync_q6 & 0x7FFF) << 2);
        int prevStartAngle_q8 = ((_cached_previous_capsuledata.start_angle_sync_q6 & 0x7FFF) << 2);

        // 计算角度差值 AngleDiff(ωi, ωi+1)【通信协议 LR001 P.27】
        diffAngle_q8 = (currentStartAngle_q8)-(prevStartAngle_q8);
        // 处理跨0度的情况: 如果上一条报文角度大于当前报文角度，
        // 说明角度经过了0度(360度)边界，需要加上360度
        if (prevStartAngle_q8 > currentStartAngle_q8) {
            diffAngle_q8 += (360 << 8);
        }

        // 计算每个采样点的角度增量(q16格式)
        // angleInc_q16 = diffAngle_q8 << 3 (q8转q16: 乘以8)
        // 对应公式 AngleDiff(ωi,ωi+1)/32 中的分子部分（32个采样点均分角度差）
        int angleInc_q16 = (diffAngle_q8 << 3);
        // 初始角度 = 上一条报文的start_angle(q16格式)
        // currentAngle_raw_q16 跟踪当前采样点的基准角度
        int currentAngle_raw_q16 = (prevStartAngle_q8 << 8);
        // 遍历16个cabin（注意: 使用的是上一条报文_cached_previous_capsuledata的数据）
        // 因为角度差值需要前后两条报文的start_angle，所以解码的是上一条报文的数据
        for (int pos = 0; pos < (int)_countof(_cached_previous_capsuledata.cabins); ++pos)
        {
            int dist_q2[2];   // 2个采样点的距离(q2格式)
            int angle_q6[2];  // 2个采样点的角度(q6格式)
            int syncBit[2];    // 2个采样点的同步标志

            // 提取距离: distance_angle字段低14位为距离(q2)
            // &0xFFFC = 清除低2位(低2位用于存储角度补偿信息)
            dist_q2[0] = (_cached_previous_capsuledata.cabins[pos].distance_angle_1 & 0xFFFC);
            dist_q2[1] = (_cached_previous_capsuledata.cabins[pos].distance_angle_2 & 0xFFFC);

            // 提取角度补偿dθ(4bit, q3格式):
            // dθ1 = offset_angles_q3低4位 | (distance_angle_1低2位 << 4)
            // 组合8bit的q3值: offset_angles提供高4位，distance_angle低2位提供补充位
            // 实际上dθ的有效精度为q3格式(角度/8度)
            int angle_offset1_q3 = ((_cached_previous_capsuledata.cabins[pos].offset_angles_q3 & 0xF) | ((_cached_previous_capsuledata.cabins[pos].distance_angle_1 & 0x3) << 4));
            int angle_offset2_q3 = ((_cached_previous_capsuledata.cabins[pos].offset_angles_q3 >> 4) | ((_cached_previous_capsuledata.cabins[pos].distance_angle_2 & 0x3) << 4));

            // 计算采样点1的角度(q6格式):
            // angle_q6 = (currentAngle_q16 - (dθ_q3 << 13)) >> 10
            // dθ_q3 << 13: q3转q16 (左移13位)
            // >> 10: q16转q6 (右移10位)
            // 对应公式: θk = ωi + AngleDiff/32 * k - dθk
            angle_q6[0] = ((currentAngle_raw_q16 - (angle_offset1_q3 << 13)) >> 10);
            // 检测是否跨越360度边界: 当前角度+增量取模360度后若小于增量，
            // 说明发生了回绕，设置syncBit=1表示新的一圈开始
            syncBit[0] = (((currentAngle_raw_q16 + angleInc_q16) % (360 << 16)) < angleInc_q16) ? 1 : 0;
            // 推进到下一个采样点的基准角度
            currentAngle_raw_q16 += angleInc_q16;


            // 计算采样点2的角度和同步标志（与采样点1相同的方法）
            angle_q6[1] = ((currentAngle_raw_q16 - (angle_offset2_q3 << 13)) >> 10);
            syncBit[1] = (((currentAngle_raw_q16 + angleInc_q16) % (360 << 16)) < angleInc_q16) ? 1 : 0;
            currentAngle_raw_q16 += angleInc_q16;

            // 遍历cabin内的2个采样点，发布HQ节点
            for (int cpos = 0; cpos < 2; ++cpos) {

                // 角度范围归一化到[0, 360)
                if (angle_q6[cpos] < 0) angle_q6[cpos] += (360 << 6);
                if (angle_q6[cpos] >= (360 << 6)) angle_q6[cpos] -= (360 << 6);

                rplidar_response_measurement_node_hq_t hqNode;

                // flag字段: bit0=syncBit(是否新圈起始)，bit1=!syncBit(互补标志)
                hqNode.flag = (syncBit[cpos] | ((!syncBit[cpos]) << 1));
                // quality字段: 有效距离时设固定质量值0x2F，无效距离时为0
                hqNode.quality = dist_q2[cpos] ? (0x2F << RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT) : 0;

                // 角度转换: q6 -> q14 (angle_q6 << 8 / 90)
                // q6格式: 实际角度 = angle_q6 / 64 度
                // q14格式: 实际角度 = angle_z_q14 / 16384 度
                // 转换: angle_z_q14 = angle_q6 * 256 / 90 = angle_q6 * 16384 / 5760
                hqNode.angle_z_q14 = (angle_q6[cpos] << 8) / 90;
                // 距离保持q2格式（实际距离 = dist_q2 / 4 毫米）
                hqNode.dist_mm_q2 = dist_q2[cpos];

                // 发布HQ节点，时间戳减去采样延迟补偿
                // pos*2+cpos: 采样点在32个点中的全局索引[0,31]
                engine->publishHQNode(_cached_last_data_timestamp_us - _getSampleDelayOffsetInExpressMode(_cachedTimingDesc, pos * 2 + cpos), &hqNode);
            }

        }
    }

    // 缓存当前报文，供下一条报文解码时使用
    _cached_previous_capsuledata = capsule;
    _is_previous_capsuledataRdy = true;
    _cached_last_data_timestamp_us = currentTS;

}


// =============================================================================
// UnpackerHandler_UltraCapsuleNode 实现 —— 扩展版 Express 数据处理器
// 报文132字节, 32个ultra_cabin(4字节each), 每ultra_cabin含3采样点, 共96采样点
// 使用SLAMTEC专利压缩编码(CN 108306649 A)压缩距离数据
// 角度补偿使用光学模型经验公式计算（非编码角度补偿量）
// 【通信协议 LR001 P.22-23】
// =============================================================================

// -----------------------------------------------------------------------------
// _getSampleDelayOffsetInUltraBoostMode: 计算扩展版 Express 模式下第 sampleIdx 个
// 采样点相对于报文接收时间戳的延迟补偿量（微秒）
// -----------------------------------------------------------------------------
// 与传统版Express的延迟模型相同，区别:
//   - 默认波特率: 256000（扩展版需要更高带宽）
//   - 报文长度: 132字节（rplidar_response_ultra_capsule_measurement_nodes_t）
//   - 采样点数: 96个（32个ultra_cabin * 3点/cabin）
//   - 分组延迟: (32*3-1 - sampleIdx) * sample_duration_uS
//     sampleIdx 范围 [0, 95]，第0个采样点延迟最大
// -----------------------------------------------------------------------------
static _u64 _getSampleDelayOffsetInUltraBoostMode(const SlamtecLidarTimingDesc& timing, int sampleIdx)
{
    // FIXME: to eval
    // 根据雷达型号猜测波特率，扩展版Express默认256000
    const _u64 channelBaudRate = timing.native_baudrate ? timing.native_baudrate : 256000;

    // 传输延迟 = 1e6 * 报文字节数(132) * 10(起始位+8数据+停止位) / 波特率
    _u64 tranmissionDelay = 1000000ULL * sizeof(rplidar_response_ultra_capsule_measurement_nodes_t) * 10 / channelBaudRate;

    // 以太网接口的传输延迟远小于UART
    if (timing.native_interface_type == LIDARInterfaceType::LIDAR_INTERFACE_ETHERNET)
    {
        tranmissionDelay = 100; //dummy value
    }

    // 采样积分时间的中点补偿
    const _u64 sampleDelay = (timing.sample_duration_uS >> 1);
    // IIR滤波器群延迟
    const _u64 sampleFilterDelay = timing.sample_duration_uS;
    // 分组延迟: 96个采样点中第sampleIdx个点相对最后一个点的等待时间
    // 32*3=96个采样点，(95-sampleIdx)为后续采样点数量
    const _u64 groupingDelay = ((32 * 3 - 1) - sampleIdx) * timing.sample_duration_uS;


    return sampleFilterDelay + sampleDelay + tranmissionDelay + timing.linkage_delay_uS + groupingDelay;
}


// -----------------------------------------------------------------------------
// 构造函数: 初始化接收缓冲区、缓存状态和定时描述
// -----------------------------------------------------------------------------
UnpackerHandler_UltraCapsuleNode::UnpackerHandler_UltraCapsuleNode()
    : _cached_scan_node_buf_pos(0)           // 接收状态机位置归零
    , _is_previous_capsuledataRdy(false)    // 尚无上一条报文缓存
    , _cached_last_data_timestamp_us(0)    // 上次报文时间戳归零
{
    // 预分配接收缓冲区，大小为扩展版ultra capsule报文长度(132字节)
    _cached_scan_node_buf.resize(sizeof(rplidar_response_ultra_capsule_measurement_nodes_t));
    // 清零定时描述结构体
    memset(&_cachedTimingDesc, 0, sizeof(_cachedTimingDesc));
}

// 析构函数: 无需手动释放资源
UnpackerHandler_UltraCapsuleNode::~UnpackerHandler_UltraCapsuleNode()
{

}

// -----------------------------------------------------------------------------
// onUnpackerContextSet: 接收外部传入的上下文配置
// 仅支持 LIDAR_TIMING 类型，设置采样定时参数
// -----------------------------------------------------------------------------
void UnpackerHandler_UltraCapsuleNode::onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size)
{
    if (type == LIDARSampleDataUnpacker::UNPACKER_CONTEXT_TYPE_LIDAR_TIMING) {
        assert(size == sizeof(_cachedTimingDesc));
        _cachedTimingDesc = *reinterpret_cast<const SlamtecLidarTimingDesc*>(data);
    }
}

// -----------------------------------------------------------------------------
// getSampleAnswerType: 返回此handler处理的数据应答类型
// RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED_ULTRA = 0x84，对应扩展版 EXPRESS_SCAN 应答
// 【通信协议 LR001 P.17, P.22】起始应答: A5 5A 84 00 00 40 84，数据应答长度 132 bytes
// -----------------------------------------------------------------------------
_u8 UnpackerHandler_UltraCapsuleNode::getSampleAnswerType() const
{
    return RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED_ULTRA;
}

// -----------------------------------------------------------------------------
// onData: 逐字节接收并解析扩展版 Express capsule 报文
// 报文格式与传统版相同的前4字节结构(sync+checksum+start_angle+S)，
// 区别在于报文体为32个ultra_cabin(4字节each)，共132字节
// 【通信协议 LR001 P.22】
// 状态机逻辑与传统版 CapsuleNode::onData 完全相同，区别仅在使用
// rplidar_response_ultra_capsule_measurement_nodes_t 结构体
// -----------------------------------------------------------------------------
void UnpackerHandler_UltraCapsuleNode::onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t cnt)
{

    for (size_t pos = 0; pos < cnt; ++pos) {
        _u8 current_data = data[pos];
        switch (_cached_scan_node_buf_pos) {
        case 0: // 状态0: 等待sync1(高4位=0xA)
        {
            _u8 tmp = (current_data >> 4);
            if (tmp == RPLIDAR_RESP_MEASUREMENT_EXP_SYNC_1) {
                // pass
            }
            else {
                _is_previous_capsuledataRdy = false;
                continue;
            }

        }
        break;
        case 1: // 状态1: 等待sync2(高4位=0x5)
        {
            _u8 tmp = (current_data >> 4);
            if (tmp == RPLIDAR_RESP_MEASUREMENT_EXP_SYNC_2) {
                // pass
            }
            else {
                _cached_scan_node_buf_pos = 0;
                _is_previous_capsuledataRdy = false;
                continue;
            }
        }
        break;

        case sizeof(rplidar_response_ultra_capsule_measurement_nodes_t) - 1: // 状态131: 报文完整
        {
            _cached_scan_node_buf[sizeof(rplidar_response_ultra_capsule_measurement_nodes_t) - 1] = current_data;
            _cached_scan_node_buf_pos = 0;

            rplidar_response_ultra_capsule_measurement_nodes_t* node = reinterpret_cast<rplidar_response_ultra_capsule_measurement_nodes_t*>(&_cached_scan_node_buf[0]);

            // 校验和计算（与传统版相同: ChkSum = 除sync字段外按字节异或）
            _u8 checksum = 0;
            _u8 recvChecksum = ((node->s_checksum_1 & 0xF) | (node->s_checksum_2 << 4));
            for (size_t cpos = offsetof(rplidar_response_ultra_capsule_measurement_nodes_t, start_angle_sync_q6);
                cpos < sizeof(rplidar_response_ultra_capsule_measurement_nodes_t); ++cpos)
            {
                checksum ^= _cached_scan_node_buf[cpos];
            }

            if (recvChecksum == checksum)
            {
                // 校验和匹配，报文有效

                // 大端CPU字节序转换
#ifdef _CPU_ENDIAN_BIG
                node->start_angle_sync_q6 = le16_to_cpu(node->start_angle_sync_q6);
                // ultra_cabin使用32位combined_x3字段，需要32位字节序转换
                for (size_t cpos = 0; cpos < _countof(node->ultra_cabins); ++cpos) {
                    node->ultra_cabins[cpos].combined_x3 = le32_to_cpu(node->ultra_cabins[cpos].combined_x3);
                }
#endif
                // 检查起始标志位S【通信协议 LR001 P.27-28】
                if (node->start_angle_sync_q6 & RPLIDAR_RESP_MEASUREMENT_EXP_SYNCBIT)
                {
                    if (_is_previous_capsuledataRdy) {
                        engine->publishDecodingErrorMsg(LIDARSampleDataUnpacker::ERR_EVENT_ON_EXP_ENCODER_RESET
                            , RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED_ULTRA, node, sizeof(*node));

                    }
                    _is_previous_capsuledataRdy = false;

                    engine->publishNewScanReset();

                }
                // 调用解码函数，将ultra capsule报文解包为HQ节点并发布
                _onScanNodeUltraCapsuleData(*node, engine);
            }
            else {
                _is_previous_capsuledataRdy = false;

                engine->publishDecodingErrorMsg(LIDARSampleDataUnpacker::ERR_EVENT_ON_EXP_CHECKSUM_ERR
                    , RPLIDAR_ANS_TYPE_MEASUREMENT_CAPSULED_ULTRA, node, sizeof(*node));

            }
            continue;
        }
        break;

        }
        _cached_scan_node_buf[_cached_scan_node_buf_pos++] = current_data;
    }

}

// -----------------------------------------------------------------------------
// reset: 重置handler状态
// -----------------------------------------------------------------------------
void UnpackerHandler_UltraCapsuleNode::reset()
{
    _cached_scan_node_buf_pos = 0;
    _is_previous_capsuledataRdy = false;
}

// -----------------------------------------------------------------------------
// _varbitscale_decode: SLAMTEC专利的可变比例编码解码器
// 【通信协议 LR001 P.23】Ultra cabin采用SLAMTEC专利压缩编码技术(CN 108306649 A)
// -----------------------------------------------------------------------------
// 可变比例编码原理:
//   12bit的major字段采用4级可变比例编码，将不同距离范围映射到不同的精度档位:
//
//   档位  | 源数据范围        | 目标基准值(DEST_VAL) | 比例(SRC_BIT) | 精度
//   ------+-------------------+---------------------+--------------+------
//   x16   | [3328, ...)       | 3328                | 14bit        | 高精度，近距离
//   x8    | [1792, 3328)      | 1792                | 12bit        | 中精度
//   x4    | [1280, 1792)      | 1280                | 11bit        | 较低精度
//   x2    | [512, 1280)       | 512                 | 9bit         | 低精度，远距离
//
//   解码公式: decoded = TARGET_BASE[i] + (scaled - DEST_VAL[i]) << scaleLevel[i]
//   其中 scaleLevel = {4, 3, 2, 1} 对应 x16/x8/x4/x2 档位
//
// 参数:
//   scaled: 编码后的12bit原始值
//   scaleLevel: 输出参数，返回匹配的档位级别(4=x16, 3=x8, 2=x4, 1=x2)
// 返回值: 解码后的距离值(单位: mm/4, 即q2格式)
// -----------------------------------------------------------------------------
static _u32 _varbitscale_decode(_u32 scaled, _u32& scaleLevel)
{
    // 各档位的目标基准值(DEST_VAL)，从高到低排列
    // 匹配第一个 scaled >= DEST_VAL[i] 的档位
    static const _u32 VBS_SCALED_BASE[] = {
        RPLIDAR_VARBITSCALE_X16_DEST_VAL,  // 3328 — x16档基准
        RPLIDAR_VARBITSCALE_X8_DEST_VAL,   // 1792 — x8档基准
        RPLIDAR_VARBITSCALE_X4_DEST_VAL,   // 1280 — x4档基准
        RPLIDAR_VARBITSCALE_X2_DEST_VAL,   // 512  — x2档基准
        0,                                  // 兜底（scaled=0时返回0）
    };

    // 各档位对应的移位级别（用于解码时左移）
    static const _u32 VBS_SCALED_LVL[] = {
        4,  // x16档: 余数左移4位
        3,  // x8档: 余数左移3位
        2,  // x4档: 余数左移2位
        1,  // x2档: 余数左移1位
        0,  // 兜底
    };

    // 各档位对应的目标基准值（解码后的起始距离值）
    static const _u32 VBS_TARGET_BASE[] = {
        (0x1 << RPLIDAR_VARBITSCALE_X16_SRC_BIT),  // 1<<14 = 16384
        (0x1 << RPLIDAR_VARBITSCALE_X8_SRC_BIT),    // 1<<12 = 4096
        (0x1 << RPLIDAR_VARBITSCALE_X4_SRC_BIT),    // 1<<11 = 2048
        (0x1 << RPLIDAR_VARBITSCALE_X2_SRC_BIT),    // 1<<9  = 512
        0,
    };

    // 从高精度档位(x16)到低精度档位(x2)依次匹配
    // 找到第一个 scaled >= DEST_VAL[i] 的档位，使用该档位解码
    for (size_t i = 0; i < _countof(VBS_SCALED_BASE); ++i)
    {
        int remain = ((int)scaled - (int)VBS_SCALED_BASE[i]);
        if (remain >= 0) {
            // 匹配到档位i，解码: 基准值 + 余数 << 移位级别
            scaleLevel = VBS_SCALED_LVL[i];
            return VBS_TARGET_BASE[i] + (remain << scaleLevel);
        }
    }

    // scaled < 512，无法解码，返回0（表示无效距离）
    return 0;
}

// -----------------------------------------------------------------------------
// _onScanNodeUltraCapsuleData: 解码扩展版ultra capsule报文，计算角度并发布HQ节点
// -----------------------------------------------------------------------------
// 扩展版与传统版的核心区别【通信协议 LR001 P.22-23, P.27】:
//
// 1. 角度增量: angleInc_q16 = (diffAngle_q8 << 3) / 3
//    传统版是 << 3（32点均分），扩展版是 << 3 / 3（96点均分，每cabin 3点）
//
// 2. 距离解码: 使用可变比例编码(varbitscale) + 预测编码
//    ultra_cabin.combined_x3 为32位复合字段:
//    - [11:0]  = major (12bit, 可变比例编码的主距离值)
//    - [21:12] = predict1 (10bit, 有符号预测编码，相对major的差值)
//    - [31:22] = predict2 (10bit, 有符号预测编码，相对下一cabin major的差值)
//
// 3. 角度补偿: 不使用编码的dθ，而是基于光学模型经验公式计算
//    - 近距离(dist < 50mm*4=200mm): 使用固定偏移角 7.5度
//    - 远距离: 使用经验公式: offset = 8° - k1/dist - (k1/dist)³
//      其中 k1 = 98361（经验常数，约等于近场偏移角的定点数表示）
// -----------------------------------------------------------------------------
void UnpackerHandler_UltraCapsuleNode::_onScanNodeUltraCapsuleData(rplidar_response_ultra_capsule_measurement_nodes_t& capsule, LIDARSampleDataUnpackerInner* engine)
{
    _u64 currentTS = engine->getCurrentTimestamp_uS();
    if (_is_previous_capsuledataRdy) {
        int diffAngle_q8;
        int currentStartAngle_q8 = ((capsule.start_angle_sync_q6 & 0x7FFF) << 2);
        int prevStartAngle_q8 = ((_cached_previous_ultracapsuledata.start_angle_sync_q6 & 0x7FFF) << 2);

        // 角度差值计算（与传统版相同）【通信协议 LR001 P.27】
        diffAngle_q8 = (currentStartAngle_q8)-(prevStartAngle_q8);
        if (prevStartAngle_q8 > currentStartAngle_q8) {
            diffAngle_q8 += (360 << 8);
        }

        // 角度增量: q8转q16后除以3（每cabin 3个采样点，96点均分角度差）
        // 传统版: angleInc_q16 = diffAngle_q8 << 3 (32点)
        // 扩展版: angleInc_q16 = (diffAngle_q8 << 3) / 3 (96点 = 32*3)
        int angleInc_q16 = (diffAngle_q8 << 3) / 3;
        int currentAngle_raw_q16 = (prevStartAngle_q8 << 8);
        // 遍历32个ultra_cabin（使用上一条报文的数据，因为需要前后两条报文的角度差值）
        for (int pos = 0; pos < (int)_countof(_cached_previous_ultracapsuledata.ultra_cabins); ++pos)
        {
            int dist_q2[3];   // 3个采样点的距离(q2格式)
            int angle_q6[3];  // 3个采样点的角度(q6格式)
            int syncBit[3];    // 3个采样点的同步标志

            // 获取当前ultra_cabin的32位复合字段
            _u32 combined_x3 = _cached_previous_ultracapsuledata.ultra_cabins[pos].combined_x3;

            // 拆解复合字段:
            // [11:0]  = major (12bit, 可变比例编码的主距离值)
            // 【通信协议 LR001 P.23】major: 12bit的采用可变比例编码的扫描测距数据读数
            int dist_major = (combined_x3 & 0xFFF);

            // [21:12] = predict1 (10bit, 有符号预测编码)
            // [31:22] = predict2 (10bit, 有符号预测编码)
            // 【通信协议 LR001 P.23】predict1/predict2: 10bit的预测编码的扫描测距读数
            // 提取predict1: 先左移10位使predict1移到最高位，再算术右移22位保持符号
            // 注意: 此处的魔法位移不可修改(DO NOT TOUCH)
            int dist_predict1 = (((int)(combined_x3 << 10)) >> 22);
            // 提取predict2: 直接算术右移22位
            int dist_predict2 = (((int)combined_x3) >> 22);

            int dist_major2;

            _u32 scalelvl1=0, scalelvl2 = 0;

            // 获取下一个cabin的major值，用于predict2的解码基准
            // 如果是最后一个cabin，使用当前报文(capsule)的第一个cabin
            if (pos == _countof(_cached_previous_ultracapsuledata.ultra_cabins) - 1)
            {
                dist_major2 = (capsule.ultra_cabins[0].combined_x3 & 0xFFF);
            }
            else {
                dist_major2 = (_cached_previous_ultracapsuledata.ultra_cabins[pos + 1].combined_x3 & 0xFFF);
            }

            // 对major值进行可变比例编码解码
            // 【通信协议 LR001 P.23】Ultra cabin采用SLAMTEC专利压缩编码技术(CN 108306649 A)
            dist_major = _varbitscale_decode(dist_major, scalelvl1);
            dist_major2 = _varbitscale_decode(dist_major2, scalelvl2);


            int dist_base1 = dist_major;
            int dist_base2 = dist_major2;

            // 如果当前cabin的major为0但下一个cabin的major非0，
            // 使用下一个cabin的major和比例级别作为基准（容错处理）
            if ((!dist_major) && dist_major2) {
                dist_base1 = dist_major2;
                scalelvl1 = scalelvl2;
            }


            // 采样点0的距离 = major解码值 << 2 (转为q2格式)
            dist_q2[0] = (dist_major << 2);
            // 采样点1的距离 = (predict1 << scalelvl + dist_base1) << 2
            // predict1是相对major的有符号差值，需加上major基准值
            // 特殊值0xFFFFFE00和0x1FF表示无效采样（10bit有符号的最大/最小值），距离设为0
            if (((_u32)dist_predict1 == 0xFFFFFE00) || ((_u32)dist_predict1 == 0x1FF)) {
                dist_q2[1] = 0;
            }
            else {
                dist_predict1 = (int)(dist_predict1 << scalelvl1);
                dist_q2[1] = (dist_predict1 + dist_base1) << 2;

            }

            // 采样点2的距离 = (predict2 << scalelvl + dist_base2) << 2
            // predict2是相对下一个cabin的major的有符号差值
            if (((_u32)dist_predict2 == 0xFFFFFE00) || ((_u32)dist_predict2 == 0x1FF)) {
                dist_q2[2] = 0;
            }
            else {
                dist_predict2 = (int)(dist_predict2 << scalelvl2);
                dist_q2[2] = (dist_predict2 + dist_base2) << 2;
            }

            // 遍历当前ultra_cabin的3个采样点
            for (int cpos = 0; cpos < 3; ++cpos)
            {

                // 同步位检测: 跨越360度边界时syncBit=1
                syncBit[cpos] = (((currentAngle_raw_q16 + angleInc_q16) % (360 << 16)) < angleInc_q16) ? 1 : 0;


                rplidar_response_measurement_node_hq_t hqNode;

                // 角度补偿: 使用RPLIDAR光学模型的经验公式计算偏移角
                // 【通信协议 LR001 P.27】扩展版本不编码角度补偿量，
                //   需结合光学模型在外部系统按公式计算
                // 默认偏移角: 7.5度（转换为q16定点数）
                int offsetAngleMean_q16 = (int)(7.5 * 3.1415926535 * (1 << 16) / 180.0);

                // 远距离时使用经验公式修正偏移角:
                // offset = 8° - k1/dist - (k1/dist)³
                // 其中 k1 = 98361 (经验常数)
                // 仅当距离 >= 200mm (50*4 q2) 时使用远距离公式
                if (dist_q2[cpos] >= (50 * 4))
                {
                    const int k1 = 98361;
                    const int k2 = int(k1 / dist_q2[cpos]);

                    offsetAngleMean_q16 = (int)(8 * 3.1415926535 * (1 << 16) / 180) - (k2 << 6) - (k2 * k2 * k2) / 98304;
                }

                // 计算最终角度: 当前基准角度 - 光学偏移角，然后q16转q6
                // 注意: 此处的 180/3.14159265 是弧度转度的近似转换
                angle_q6[cpos] = ((currentAngle_raw_q16 - int(offsetAngleMean_q16 * 180 / 3.14159265)) >> 10);
                // 推进到下一个采样点的基准角度
                currentAngle_raw_q16 += angleInc_q16;

                // 角度范围归一化到[0, 360)
                if (angle_q6[cpos] < 0) angle_q6[cpos] += (360 << 6);
                if (angle_q6[cpos] >= (360 << 6)) angle_q6[cpos] -= (360 << 6);


                hqNode.flag = (syncBit[cpos] | ((!syncBit[cpos]) << 1));
                hqNode.quality = dist_q2[cpos] ? (0x2F << RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT) : 0;

                // q6 -> q14 转换
                hqNode.angle_z_q14 = (angle_q6[cpos] << 8) / 90;
                hqNode.dist_mm_q2 = dist_q2[cpos];

                // 发布HQ节点，pos*3+cpos为96个采样点中的全局索引[0,95]
                engine->publishHQNode(_cached_last_data_timestamp_us - _getSampleDelayOffsetInUltraBoostMode(_cachedTimingDesc, pos * 3 + cpos), &hqNode);
            }

        }
    }

    // 缓存当前报文，供下一条报文解码时使用
    _cached_previous_ultracapsuledata = capsule;
    _is_previous_capsuledataRdy = true;
    _cached_last_data_timestamp_us = currentTS;

}


// =============================================================================
// UnpackerHandler_DenseCapsuleNode 实现 —— 密实版 Express 数据处理器
// 报文84字节, 40个cabin(2字节each), 每cabin含1采样点, 共40采样点
// 每个cabin只含distance(16bit)，不编码角度补偿dθ
// 角度公式: θk = ωi + AngleDiff(ωi,ωi+1)/40 * k（无dθ补偿项）
// 【通信协议 LR001 P.24-25, P.27-28】
// =============================================================================

// -----------------------------------------------------------------------------
// _getSampleDelayOffsetInDenseMode: 计算密实版 Express 模式下第 sampleIdx 个
// 采样点相对于报文接收时间戳的延迟补偿量（微秒）
// -----------------------------------------------------------------------------
// 与传统版Express的延迟模型相同，区别:
//   - 默认波特率: 256000
//   - 报文长度: 84字节（rplidar_response_dense_capsule_measurement_nodes_t）
//   - 采样点数: 40个（40个cabin * 1点/cabin）
//   - 分组延迟: (39 - sampleIdx) * sample_duration_uS
//     sampleIdx 范围 [0, 39]，第0个采样点延迟最大
// -----------------------------------------------------------------------------
static _u64 _getSampleDelayOffsetInDenseMode(const SlamtecLidarTimingDesc& timing, int sampleIdx)
{
    // FIXME: to eval
    // 根据雷达型号猜测波特率，密实版默认256000
    const _u64 channelBaudRate = timing.native_baudrate ? timing.native_baudrate : 256000;

    // 传输延迟 = 1e6 * 报文字节数(84) * 10(起始位+8数据+停止位) / 波特率
    _u64 tranmissionDelay = 1000000ULL * sizeof(rplidar_response_dense_capsule_measurement_nodes_t) * 10 / channelBaudRate;

    // 以太网接口的传输延迟远小于UART
    if (timing.native_interface_type == LIDARInterfaceType::LIDAR_INTERFACE_ETHERNET)
    {
        tranmissionDelay = 100; //dummy value
    }

    // 采样积分时间的中点补偿
    const _u64 sampleDelay = (timing.sample_duration_uS >> 1);
    // IIR滤波器群延迟
    const _u64 sampleFilterDelay = timing.sample_duration_uS;
    // 分组延迟: 40个采样点中第sampleIdx个点相对最后一个点的等待时间
    // (39-sampleIdx)为后续采样点数量，sampleIdx=0时延迟最大(39个采样周期)
    const _u64 groupingDelay = (39 - sampleIdx) * timing.sample_duration_uS;


    return sampleFilterDelay + sampleDelay + tranmissionDelay + timing.linkage_delay_uS + groupingDelay;
}

// -----------------------------------------------------------------------------
// 构造函数: 初始化接收缓冲区、缓存状态和定时描述
// -----------------------------------------------------------------------------
UnpackerHandler_DenseCapsuleNode::UnpackerHandler_DenseCapsuleNode()
    : _cached_scan_node_buf_pos(0)           // 接收状态机位置归零
    , _is_previous_capsuledataRdy(false)    // 尚无上一条报文缓存
    , _cached_last_data_timestamp_us(0)    // 上次报文时间戳归零

{
    // 预分配接收缓冲区，大小为密实版dense capsule报文长度(84字节)
    _cached_scan_node_buf.resize(sizeof(rplidar_response_dense_capsule_measurement_nodes_t));
    // 清零定时描述结构体
    memset(&_cachedTimingDesc, 0, sizeof(_cachedTimingDesc));
}

// 析构函数: 无需手动释放资源
UnpackerHandler_DenseCapsuleNode::~UnpackerHandler_DenseCapsuleNode()
{

}

// -----------------------------------------------------------------------------
// onUnpackerContextSet: 接收外部传入的上下文配置
// 仅支持 LIDAR_TIMING 类型，设置采样定时参数
// -----------------------------------------------------------------------------
void UnpackerHandler_DenseCapsuleNode::onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size)
{
    if (type == LIDARSampleDataUnpacker::UNPACKER_CONTEXT_TYPE_LIDAR_TIMING) {
        assert(size == sizeof(_cachedTimingDesc));
        _cachedTimingDesc = *reinterpret_cast<const SlamtecLidarTimingDesc*>(data);
    }
}

// -----------------------------------------------------------------------------
// getSampleAnswerType: 返回此handler处理的数据应答类型
// RPLIDAR_ANS_TYPE_MEASUREMENT_DENSE_CAPSULED = 0x85，对应密实版 EXPRESS_SCAN 应答
// 【通信协议 LR001 P.17, P.24】起始应答: A5 5A 54 00 00 40 85，数据应答长度 84 bytes
// -----------------------------------------------------------------------------
_u8 UnpackerHandler_DenseCapsuleNode::getSampleAnswerType() const
{
    return RPLIDAR_ANS_TYPE_MEASUREMENT_DENSE_CAPSULED;
}


// -----------------------------------------------------------------------------
// onData: 逐字节接收并解析密实版 Express capsule 报文
// 【通信协议 LR001 P.24-25】
// 状态机逻辑与传统版/扩展版 CapsuleNode::onData 完全相同，
// 区别仅在使用 rplidar_response_dense_capsule_measurement_nodes_t 结构体
// 报文结构: 2(sync+chksum) + 2(start_angle+S) + 40*cabin(2B each) = 84字节
// -----------------------------------------------------------------------------
void UnpackerHandler_DenseCapsuleNode::onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t cnt)
{

    for (size_t pos = 0; pos < cnt; ++pos) {
        _u8 current_data = data[pos];
        switch (_cached_scan_node_buf_pos) {
        case 0: // 状态0: 等待sync1(高4位=0xA)
        {
            _u8 tmp = (current_data >> 4);
            if (tmp == RPLIDAR_RESP_MEASUREMENT_EXP_SYNC_1) {
                // pass
            }
            else {
                _is_previous_capsuledataRdy = false;
                continue;
            }

        }
        break;
        case 1: // 状态1: 等待sync2(高4位=0x5)
        {
            _u8 tmp = (current_data >> 4);
            if (tmp == RPLIDAR_RESP_MEASUREMENT_EXP_SYNC_2) {
                // pass
            }
            else {
                _cached_scan_node_buf_pos = 0;
                _is_previous_capsuledataRdy = false;
                continue;
            }
        }
        break;

        case sizeof(rplidar_response_dense_capsule_measurement_nodes_t) - 1: // 状态83: 报文完整
        {
            _cached_scan_node_buf[sizeof(rplidar_response_dense_capsule_measurement_nodes_t) - 1] = current_data;
            _cached_scan_node_buf_pos = 0;

            rplidar_response_dense_capsule_measurement_nodes_t* node = reinterpret_cast<rplidar_response_dense_capsule_measurement_nodes_t*>(&_cached_scan_node_buf[0]);

            // 校验和计算（与传统版相同: ChkSum = 除sync字段外按字节异或）
            _u8 checksum = 0;
            _u8 recvChecksum = ((node->s_checksum_1 & 0xF) | (node->s_checksum_2 << 4));
            for (size_t cpos = offsetof(rplidar_response_dense_capsule_measurement_nodes_t, start_angle_sync_q6);
                cpos < sizeof(rplidar_response_dense_capsule_measurement_nodes_t); ++cpos)
            {
                checksum ^= _cached_scan_node_buf[cpos];
            }

            if (recvChecksum == checksum)
            {
                // 校验和匹配，报文有效

                // 大端CPU字节序转换
#ifdef _CPU_ENDIAN_BIG
                node->start_angle_sync_q6 = le16_to_cpu(node->start_angle_sync_q6);
                for (size_t cpos = 0; cpos < _countof(node->cabins); ++cpos) {
                    node->cabins[cpos].distance_angle_1 = le16_to_cpu(node->cabins[cpos].distance_angle_1);
                    node->cabins[cpos].distance_angle_2 = le16_to_cpu(node->cabins[cpos].distance_angle_2);
                }
#endif
                // 检查起始标志位S【通信协议 LR001 P.27-28】
                if (node->start_angle_sync_q6 & RPLIDAR_RESP_MEASUREMENT_EXP_SYNCBIT)
                {
                    if (_is_previous_capsuledataRdy) {
                        engine->publishDecodingErrorMsg(LIDARSampleDataUnpacker::ERR_EVENT_ON_EXP_ENCODER_RESET
                            , RPLIDAR_ANS_TYPE_MEASUREMENT_DENSE_CAPSULED, node, sizeof(*node));
                    }
                    _is_previous_capsuledataRdy = false;
                    engine->publishNewScanReset();


                }
                // 调用解码函数，将dense capsule报文解包为HQ节点并发布
                _onScanNodeDenseCapsuleData(*node, engine);
            }
            else {
                _is_previous_capsuledataRdy = false;

                engine->publishDecodingErrorMsg(LIDARSampleDataUnpacker::ERR_EVENT_ON_EXP_CHECKSUM_ERR
                    , RPLIDAR_ANS_TYPE_MEASUREMENT_DENSE_CAPSULED, node, sizeof(*node));

            }
            continue;
        }
        break;

        }
        _cached_scan_node_buf[_cached_scan_node_buf_pos++] = current_data;
    }
}

// -----------------------------------------------------------------------------
// reset: 重置handler状态
// -----------------------------------------------------------------------------
void UnpackerHandler_DenseCapsuleNode::reset()
{
    _cached_scan_node_buf_pos = 0;
    _cached_last_data_timestamp_us = 0;
}

// -----------------------------------------------------------------------------
// _onScanNodeDenseCapsuleData: 解码密实版dense capsule报文，计算角度并发布HQ节点
// -----------------------------------------------------------------------------
// 密实版与传统版的核心区别【通信协议 LR001 P.24-25, P.27-28】:
//
// 1. 每个cabin只含1个采样点的distance(16bit)，无角度补偿dθ
//    角度公式: θk = ωi + AngleDiff(ωi,ωi+1)/40 * k（无-dθk项）
//
// 2. 角度增量: angleInc_q16 = (diffAngle_q8 << 8) / 40
//    传统版: << 3 (32点)，密实版: << 8 / 40 (40点均分)
//    注意: 密实版使用 << 8 而非 << 3，因为密实版直接从q8转q16（<<8），
//    再除以40得到每采样点角度增量
//
// 3. 异常角度差值检测:
//    maxDiffAngleThreshold = 360 * 100Hz * 40points / (1e6/sample_duration_uS)
//    当角度差值超过此阈值时，认为数据异常，丢弃当前报文
//
// 4. 同步位检测改进:
//    使用 lastNodeSyncBit 与当前syncBit异或，确保syncBit只被精确检测一次
//    （避免在360度边界附近重复触发）
//
// 5. 时间戳使用 currentTs（当前报文时间戳）而非 _cached_last_data_timestamp_us
//    （与传统版/扩展版不同，密实版使用当前时间戳）
// -----------------------------------------------------------------------------
void UnpackerHandler_DenseCapsuleNode::_onScanNodeDenseCapsuleData(rplidar_response_dense_capsule_measurement_nodes_t& dense_capsule, LIDARSampleDataUnpackerInner* engine)
{
    // 静态变量: 记录上一个节点的syncBit，用于去重（确保每圈只触发一次同步）
    static int lastNodeSyncBit = 0;
    _u64 currentTs = engine->getCurrentTimestamp_uS();

    if (_is_previous_capsuledataRdy) {
        int diffAngle_q8;
        int currentStartAngle_q8 = ((dense_capsule.start_angle_sync_q6 & 0x7FFF) << 2);
        int prevStartAngle_q8 = ((_cached_previous_dense_capsuledata.start_angle_sync_q6 & 0x7FFF) << 2);

        // 角度差值计算（与传统版相同）【通信协议 LR001 P.27-28】
        diffAngle_q8 = (currentStartAngle_q8)-(prevStartAngle_q8);
        if (prevStartAngle_q8 > currentStartAngle_q8) {
            diffAngle_q8 += (360 << 8);
        }
        // 异常角度差值检测:
        // 计算理论最大角度差值 = 360度 * 100Hz转速 * 40点/报文 / (1e6/sample_duration_uS)
        // 如果实际角度差值超过此阈值，说明报文异常（如丢包、转速突变），丢弃
        int maxDiffAngleThreshold_q8 = (360/* 360 degree */ * 100 /*100Hz*/ * _countof(dense_capsule.cabins) /*40 points per capsule*/ / (1000000 / _cachedTimingDesc.sample_duration_uS)) << 8;
        if (diffAngle_q8 > maxDiffAngleThreshold_q8) {//discard
            _cached_previous_dense_capsuledata = dense_capsule;
            return;
        }

        // 角度增量: q8直接转q16(<<8)再除以40（40个采样点均分角度差）
        // 【通信协议 LR001 P.28】θk = ωi + AngleDiff(ωi,ωi+1)/40 * k
        int angleInc_q16 = (diffAngle_q8 << 8) / 40;
        int currentAngle_raw_q16 = (prevStartAngle_q8 << 8);
        // 遍历40个cabin（每cabin含1个采样点）
        for (int pos = 0; pos < (int)_countof(_cached_previous_dense_capsuledata.cabins); ++pos)
        {
            int dist_q2;
            int angle_q6;
            int syncBit;
            // 提取距离: cabin.distance为16bit原始值（单位: mm）
            const int dist = static_cast<const int>(_cached_previous_dense_capsuledata.cabins[pos].distance);
            // 转为q2格式（<<2），实际距离 = dist_q2 / 4 mm
            dist_q2 = dist << 2;
            // 计算角度: q16直接转q6（>>10），无dθ补偿项
            // 对应公式: θk = ωi + AngleDiff/40 * k
            angle_q6 = (currentAngle_raw_q16 >> 10);
            // 同步位检测: 当前角度+增量取模360度后若小于2*增量，
            // 说明在360度边界附近，可能发生回绕
            // 使用 << 1 增加检测窗口宽度
            syncBit = (((currentAngle_raw_q16 + angleInc_q16) % (360 << 16)) < (angleInc_q16 << 1)) ? 1 : 0;
            // 去重: 与上一个syncBit异或后再AND，确保只在syncBit从0变1时触发
            syncBit = (syncBit ^ lastNodeSyncBit) & syncBit;//Ensure that syncBit is exactly detected

            // 推进到下一个采样点的基准角度
            currentAngle_raw_q16 += angleInc_q16;

            // 角度范围归一化到[0, 360)
            if (angle_q6 < 0) angle_q6 += (360 << 6);
            if (angle_q6 >= (360 << 6)) angle_q6 -= (360 << 6);

            rplidar_response_measurement_node_hq_t hqNode;


            hqNode.flag = (syncBit | ((!syncBit) << 1));
            hqNode.quality = dist_q2 ? (0x2F << RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT) : 0;
            // q6 -> q14 转换
            hqNode.angle_z_q14 = (angle_q6 << 8) / 90;
            hqNode.dist_mm_q2 = dist_q2;
            // 发布HQ节点，使用当前时间戳(currentTs)而非缓存时间戳
            engine->publishHQNode(currentTs - _getSampleDelayOffsetInDenseMode(_cachedTimingDesc, pos), &hqNode);

            lastNodeSyncBit = syncBit;

        }
    }

    // 缓存当前报文，供下一条报文解码时使用
    _cached_previous_dense_capsuledata = dense_capsule;
    _is_previous_capsuledataRdy = true;

}

// =============================================================================
// UnpackerHandler_UltraDenseCapsuleNode 实现 —— 超密实版 Express 数据处理器
// SDK扩展类型(0x86)，非协议标准命令表所列
// 报文含32个ultra_dense_cabin(每cabin含2采样点)，共64采样点
// 报文布局: 2(sync+chksum) + 4(time_stamp) + 2(dev_status) + 2(start_angle+S)
//           + 32*ultra_dense_cabin(5字节each) = 报文
// 使用4级可变比例编码压缩quality和distance
// =============================================================================

// -----------------------------------------------------------------------------
// _getSampleDelayOffsetInUltraDenseMode: 计算超密实版模式下第 sampleIdx 个
// 采样点相对于报文接收时间戳的延迟补偿量（微秒）
// -----------------------------------------------------------------------------
// 与其他版本的延迟模型相同，区别:
//   - 默认波特率: 1000000（超密实版需要1Mbps带宽）
//   - 报文长度: sizeof(sl_lidar_response_ultra_dense_capsule_measurement_nodes_t)
//   - 采样点数: 64个（32个cabin * 2点/cabin）
//   - 分组延迟: (31 - sampleIdx) * sample_duration_uS
//     注意: 此处sampleIdx范围为[0,63]，但groupingDelay公式仅用(31-sampleIdx)，
//     可能存在FIXME待修正（其他版本用 总点数-1-sampleIdx）
// -----------------------------------------------------------------------------
static _u64 _getSampleDelayOffsetInUltraDenseMode(const SlamtecLidarTimingDesc& timing, int sampleIdx)
{
    // FIXME: to eval
    // 根据雷达型号猜测波特率，超密实版默认1000000(1Mbps)
    const _u64 channelBaudRate = timing.native_baudrate ? timing.native_baudrate : 1000000;

    // 传输延迟 = 1e6 * 报文字节数 * 10(起始位+8数据+停止位) / 波特率
    _u64 tranmissionDelay = 1000000ULL * sizeof(sl_lidar_response_ultra_dense_capsule_measurement_nodes_t) * 10 / channelBaudRate;

    // 以太网接口的传输延迟远小于UART
    if (timing.native_interface_type == LIDARInterfaceType::LIDAR_INTERFACE_ETHERNET)
    {
        tranmissionDelay = 100; //dummy value
    }

    // 采样积分时间的中点补偿
    const _u64 sampleDelay = (timing.sample_duration_uS >> 1);
    // IIR滤波器群延迟
    const _u64 sampleFilterDelay = timing.sample_duration_uS;
    // 分组延迟
    const _u64 groupingDelay = (31 - sampleIdx) * timing.sample_duration_uS;


    return sampleFilterDelay + sampleDelay + tranmissionDelay + timing.linkage_delay_uS + groupingDelay;
}


// -----------------------------------------------------------------------------
// 构造函数: 初始化接收缓冲区、缓存状态和定时描述
// 超密实版额外初始化 _last_node_sync_bit 和 _last_dist_q2
// -----------------------------------------------------------------------------
UnpackerHandler_UltraDenseCapsuleNode::UnpackerHandler_UltraDenseCapsuleNode()
    : _cached_scan_node_buf_pos(0)           // 接收状态机位置归零
    , _is_previous_capsuledataRdy(false)    // 尚无上一条报文缓存
    , _cached_last_data_timestamp_us(0)    // 上次报文时间戳归零
    , _last_node_sync_bit(0)               // 上一个同步位归零
    , _last_dist_q2(0)                     // 上一个距离值归零

{
    // 预分配接收缓冲区，大小为超密实版ultra dense capsule报文长度
    _cached_scan_node_buf.resize(sizeof(rplidar_response_ultra_dense_capsule_measurement_nodes_t));
    // 清零定时描述结构体
    memset(&_cachedTimingDesc, 0, sizeof(_cachedTimingDesc));
}

// 析构函数: 无需手动释放资源
UnpackerHandler_UltraDenseCapsuleNode::~UnpackerHandler_UltraDenseCapsuleNode()
{

}


// -----------------------------------------------------------------------------
// onUnpackerContextSet: 接收外部传入的上下文配置
// 仅支持 LIDAR_TIMING 类型，设置采样定时参数
// -----------------------------------------------------------------------------
void UnpackerHandler_UltraDenseCapsuleNode::onUnpackerContextSet(LIDARSampleDataUnpacker::UnpackerContextType type, const void* data, size_t size)
{
    if (type == LIDARSampleDataUnpacker::UNPACKER_CONTEXT_TYPE_LIDAR_TIMING) {
        assert(size == sizeof(_cachedTimingDesc));
        _cachedTimingDesc = *reinterpret_cast<const SlamtecLidarTimingDesc*>(data);
    }
}


// -----------------------------------------------------------------------------
// getSampleAnswerType: 返回此handler处理的数据应答类型
// RPLIDAR_ANS_TYPE_MEASUREMENT_ULTRA_DENSE_CAPSULED = 0x86
// SDK扩展类型，非协议标准命令表所列
// -----------------------------------------------------------------------------
_u8 UnpackerHandler_UltraDenseCapsuleNode::getSampleAnswerType() const
{
    return RPLIDAR_ANS_TYPE_MEASUREMENT_ULTRA_DENSE_CAPSULED;
}

// -----------------------------------------------------------------------------
// onData: 逐字节接收并解析超密实版 Express capsule 报文
// SDK扩展类型，非协议标准
// 状态机逻辑与其他版本相同，区别:
//   - 使用 rplidar_response_ultra_dense_capsule_measurement_nodes_t 结构体
//   - 校验和计算从 time_stamp 字段开始（而非start_angle_sync_q6），
//     因为超密实版报文包含额外的time_stamp和dev_status字段
//   - 大端转换需要处理 qualityl_distance_scale[2] (16bit数组)
// -----------------------------------------------------------------------------
void UnpackerHandler_UltraDenseCapsuleNode::onData(LIDARSampleDataUnpackerInner* engine, const _u8* data, size_t cnt)
{
    for (size_t pos = 0; pos < cnt; ++pos) {
        _u8 current_data = data[pos];
        switch (_cached_scan_node_buf_pos) {
        case 0: // 状态0: 等待sync1(高4位=0xA)
        {
            _u8 tmp = (current_data >> 4);
            if (tmp == RPLIDAR_RESP_MEASUREMENT_EXP_SYNC_1) {
                // pass
            }
            else {
                _is_previous_capsuledataRdy = false;
                continue;
            }

        }
        break;
        case 1: // 状态1: 等待sync2(高4位=0x5)
        {
            _u8 tmp = (current_data >> 4);
            if (tmp == RPLIDAR_RESP_MEASUREMENT_EXP_SYNC_2) {
                // pass
            }
            else {
                _cached_scan_node_buf_pos = 0;
                _is_previous_capsuledataRdy = false;
                continue;
            }
        }
        break;

        case sizeof(rplidar_response_ultra_dense_capsule_measurement_nodes_t) - 1: // 报文完整
        {
            _cached_scan_node_buf[sizeof(rplidar_response_ultra_dense_capsule_measurement_nodes_t) - 1] = current_data;
            _cached_scan_node_buf_pos = 0;

            rplidar_response_ultra_dense_capsule_measurement_nodes_t* node = reinterpret_cast<rplidar_response_ultra_dense_capsule_measurement_nodes_t*>(&_cached_scan_node_buf[0]);

            // 校验和计算:
            // 与其他版本不同，超密实版从time_stamp字段开始计算校验和
            // （因为报文包含time_stamp和dev_status字段，这些也需要参与校验）
            _u8 checksum = 0;
            _u8 recvChecksum = ((node->s_checksum_1 & 0xF) | (node->s_checksum_2 << 4));
            for (size_t cpos = offsetof(rplidar_response_ultra_dense_capsule_measurement_nodes_t, time_stamp);
                cpos < sizeof(rplidar_response_ultra_dense_capsule_measurement_nodes_t); ++cpos)
            {
                checksum ^= _cached_scan_node_buf[cpos];
            }

            if (recvChecksum == checksum)
            {
                // 校验和匹配，报文有效

                // 大端CPU字节序转换
#ifdef _CPU_ENDIAN_BIG
                node->start_angle_sync_q6 = le16_to_cpu(node->start_angle_sync_q6);
                // 超密实cabin使用16位qualityl_distance_scale数组(2个元素)
                for (size_t cpos = 0; cpos < _countof(node->cabins); ++cpos) {
                    node->cabins[cpos].qualityl_distance_scale[0] = le16_to_cpu(node->cabins[cpos].qualityl_distance_scale[0]);
                    node->cabins[cpos].qualityl_distance_scale[1] = le16_to_cpu(node->cabins[cpos].qualityl_distance_scale[1]);
                }
#endif
                // 检查起始标志位S
                if (node->start_angle_sync_q6 & RPLIDAR_RESP_MEASUREMENT_EXP_SYNCBIT)
                {
                    if (_is_previous_capsuledataRdy) {
                        engine->publishDecodingErrorMsg(LIDARSampleDataUnpacker::ERR_EVENT_ON_EXP_ENCODER_RESET
                            , RPLIDAR_ANS_TYPE_MEASUREMENT_ULTRA_DENSE_CAPSULED, node, sizeof(*node));

                    }
                    _is_previous_capsuledataRdy = false;
                    engine->publishNewScanReset();

                }
                // 调用解码函数，将ultra dense capsule报文解包为HQ节点并发布
                _onScanNodeUltraDenseCapsuleData(*node, engine);
            }
            else {
                _is_previous_capsuledataRdy = false;

                engine->publishDecodingErrorMsg(LIDARSampleDataUnpacker::ERR_EVENT_ON_EXP_CHECKSUM_ERR
                    , RPLIDAR_ANS_TYPE_MEASUREMENT_ULTRA_DENSE_CAPSULED, node, sizeof(*node));

            }
            continue;
        }
        break;

        }
        _cached_scan_node_buf[_cached_scan_node_buf_pos++] = current_data;
    }

}

// -----------------------------------------------------------------------------
// reset: 重置handler状态
// 额外重置 _last_node_sync_bit 和 _last_dist_q2
// -----------------------------------------------------------------------------
void UnpackerHandler_UltraDenseCapsuleNode::reset()
{
    _cached_scan_node_buf_pos = 0;
    _cached_last_data_timestamp_us = 0;
    _last_node_sync_bit = 0;
    _last_dist_q2 = 0;
}

// -----------------------------------------------------------------------------
// _onScanNodeUltraDenseCapsuleData: 解码超密实版ultra dense capsule报文，计算角度并发布HQ节点
// -----------------------------------------------------------------------------
// 超密实版是SDK扩展格式(0x86)，非协议标准文档所列。
// 报文包含: time_stamp(4B) + dev_status(2B) + start_angle_sync_q6(2B) + 32个cabin(5B each)
// 每个cabin(sl_lidar_response_ultra_dense_cabin_nodes_t)包含:
//   - qualityl_distance_scale[2]: 2个16bit的距离质量尺度化数据
//   - qualityh_array: 1个8bit的质量高位数组
//
// 每个采样点的quality_dist_scale由3部分组合为20bit值:
//   - 偶数采样点(pos&0x1==0): qualityl_distance_scale[0] | (qualityh_array低4位 << 16)
//   - 奇数采样点(pos&0x1==1): qualityl_distance_scale[1] | (qualityh_array高4位 << 16)
//
// 4级可变比例编码(scale字段，2bit):
//   scale=0: 距离范围 [0, 2046]mm,    quality用12bit, distance用10bit
//   scale=1: 距离范围 [2046, 8187]mm,  quality用11bit, distance用11bit
//   scale=2: 距离范围 [8187, 24567]mm, quality用10bit, distance用12bit
//   scale=3: 距离范围 [24567, ...]mm,  quality用9bit,  distance用13bit
//
// 角度公式: θk = ωi + AngleDiff(ωi,ωi+1)/64 * k（无dθ补偿项，与密实版类似）
// 64个采样点(32个cabin * 2点/cabin)
// -----------------------------------------------------------------------------
void UnpackerHandler_UltraDenseCapsuleNode::_onScanNodeUltraDenseCapsuleData(rplidar_response_ultra_dense_capsule_measurement_nodes_t& capsule, LIDARSampleDataUnpackerInner* engine)
{
    _u64 currentTimestamp = engine->getCurrentTimestamp_uS();

    const rplidar_response_ultra_dense_capsule_measurement_nodes_t* ultra_dense_capsule = reinterpret_cast<const rplidar_response_ultra_dense_capsule_measurement_nodes_t*>(&capsule);
    if (_is_previous_capsuledataRdy) {
        int diffAngle_q8;
        int currentStartAngle_q8 = ((ultra_dense_capsule->start_angle_sync_q6 & 0x7FFF) << 2);
        int prevStartAngle_q8 = ((_cached_previous_ultra_dense_capsuledata.start_angle_sync_q6 & 0x7FFF) << 2);



        // 角度差值计算（与其他版本相同）
        diffAngle_q8 = (currentStartAngle_q8)-(prevStartAngle_q8);
        if (prevStartAngle_q8 > currentStartAngle_q8) {
            diffAngle_q8 += (360 << 8);
        }

        // 异常角度差值检测（与密实版相同逻辑）
        // 64个采样点(32个cabin * 2点)，理论最大角度差值
        int maxDiffAngleThreshold_q8 = (360/* 360 degree */ * 100 /*100Hz*/ * _countof(ultra_dense_capsule->cabins) /*64 points per capsule*/ / (1000000 / _cachedTimingDesc.sample_duration_uS)) << 8;
        if (diffAngle_q8 > maxDiffAngleThreshold_q8) {//discard
            _cached_previous_ultra_dense_capsuledata = *ultra_dense_capsule;
            return;
        }
        // 4级可变比例编码的距离阈值定义（单位: mm）
        // scale 0->1 的边界: (2^10-1)*2 = 2046mm
        // scale 1->2 的边界: (2^11-1)*3 + 2046 = 8187mm
        // scale 2->3 的边界: (2^12-1)*4 + 8187 = 24567mm
#define DISTANCE_THRESHOLD_TO_SCALE_1 2046  // (2^10 - 1)*2 mm
#define DISTANCE_THRESHOLD_TO_SCALE_2 8187  // (2^11 - 1)*3 + 2046 mm
#define DISTANCE_THRESHOLD_TO_SCALE_3 24567 // (2^12 - 1)*4 + 8187 mm
        // 角度增量: q8转q16后除以64（64个采样点均分角度差）
        int angleInc_q16 = (diffAngle_q8 << 8) / 64;
        int currentAngle_raw_q16 = (prevStartAngle_q8 << 8);
        // 遍历64个采样点（32个cabin * 2点/cabin）
        for (int pos = 0; pos < (int)_countof(_cached_previous_ultra_dense_capsuledata.cabins) * 2; ++pos)
        {
            int angle_q6;
            int syncBit;
            // cabin索引 = pos / 2（每cabin含2个采样点）
            size_t cabin_idx = pos >> 1;
            _u32  quality_dist_scale;
            // 组合20bit的quality_dist_scale值:
            // 偶数采样点(第0个): qualityl_distance_scale[0] + qualityh_array低4位
            // 奇数采样点(第1个): qualityl_distance_scale[1] + qualityh_array高4位
            if (!(pos & 0x1)) {
                quality_dist_scale = _cached_previous_ultra_dense_capsuledata.cabins[cabin_idx].qualityl_distance_scale[0] | ((_cached_previous_ultra_dense_capsuledata.cabins[cabin_idx].qualityh_array & 0x0F) << 16);
            }
            else {
                quality_dist_scale = _cached_previous_ultra_dense_capsuledata.cabins[cabin_idx].qualityl_distance_scale[1] | ((_cached_previous_ultra_dense_capsuledata.cabins[cabin_idx].qualityh_array >> 4) << 16);
            }

            // scale字段: 最低2bit，决定4级可变比例编码的档位
            _u8 scale = quality_dist_scale & 0x3;
            _u8 quality = 0;
            int dist_q2 = 0;

            // 根据4级可变比例编码解码quality和distance:
            // scale=0: 近距离档, distance 10bit (mask 0xFFC), quality 12bit
            // scale=1: 中近距离档, distance 11bit (mask 0x1FFC), quality 11bit
            // scale=2: 中远距离档, distance 12bit (mask 0x3FFC), quality 10bit
            // scale=3: 远距离档, distance 13bit (mask 0x7FFC), quality 9bit
            // 距离解码: dist_q2 = distance_field * (scale+2) + threshold
            // 质量解码: quality = (quality_field >> shift) << scale
            switch (scale) {
            case 0:
                // scale=0: 距离倍率=2, quality在bit[19:12]
                quality = quality_dist_scale >> 12;
                // 距离: 低12位中的10bit (&0xFFC) 乘以2
                dist_q2 = (quality_dist_scale & 0xFFC) * 2;
                // 近距离平滑: 如果与上一个距离差值很小(<=8, 即2mm*2)，
                // 取平均值，减少近距离噪声
                if (_last_dist_q2) {
                    if (abs(dist_q2 - _last_dist_q2) <= 8/*2mm *2*/) {
                        dist_q2 = (dist_q2 + _last_dist_q2) >> 1;
                    }
                }
                break;
            case 1:
                // scale=1: 距离倍率=3, quality在bit[19:13]
                quality = (quality_dist_scale >> 13) << 1;
                // 距离: 低13位中的11bit (&0x1FFC) 乘以3 + 阈值2046<<2
                dist_q2 = (quality_dist_scale & 0x1FFC) * 3 + (DISTANCE_THRESHOLD_TO_SCALE_1 << 2);
                break;
            case 2:
                // scale=2: 距离倍率=4, quality在bit[19:14]
                quality = (quality_dist_scale >> 14) << 2;
                // 距离: 低14位中的12bit (&0x3FFC) 乘以4 + 阈值8187<<2
                dist_q2 = (quality_dist_scale & 0x3FFC) * 4 + (DISTANCE_THRESHOLD_TO_SCALE_2 << 2);
                break;
            case 3:
                // scale=3: 距离倍率=5, quality在bit[19:15]
                quality = (quality_dist_scale >> 15) << 3;
                // 距离: 低15位中的13bit (&0x7FFC) 乘以5 + 阈值24567<<2
                dist_q2 = (quality_dist_scale & 0x7FFC) * 5 + (DISTANCE_THRESHOLD_TO_SCALE_3 << 2);
                break;
            }
            // 保存当前距离供下一个采样点的近距离平滑使用
            _last_dist_q2 = dist_q2;
            // 计算角度: q16直接转q6（>>10），无dθ补偿项
            angle_q6 = (currentAngle_raw_q16 >> 10);
            // 同步位检测（与密实版相同逻辑）
            syncBit = (((currentAngle_raw_q16 + angleInc_q16) % (360 << 16)) < (angleInc_q16 << 1)) ? 1 : 0;
            // 去重: 确保syncBit只被精确检测一次
            syncBit = (syncBit ^ _last_node_sync_bit) & syncBit;//Ensure that syncBit is exactly detected

            // 推进到下一个采样点的基准角度
            currentAngle_raw_q16 += angleInc_q16;

            // 角度范围归一化到[0, 360)
            if (angle_q6 < 0) angle_q6 += (360 << 6);
            if (angle_q6 >= (360 << 6)) angle_q6 -= (360 << 6);


            rplidar_response_measurement_node_hq_t hqNode;



            hqNode.flag = (syncBit | ((!syncBit) << 1));
            // 超密实版使用解码得到的quality值（而非固定值0x2F）
            hqNode.quality = quality;
            // q6 -> q14 转换
            hqNode.angle_z_q14 = (angle_q6 << 8) / 90;
            hqNode.dist_mm_q2 = dist_q2;
            // 发布HQ节点，使用当前时间戳(currentTimestamp)
            engine->publishHQNode(currentTimestamp - _getSampleDelayOffsetInUltraDenseMode(_cachedTimingDesc, pos), &hqNode);

            _last_node_sync_bit = syncBit;

        }
    }

    // 缓存当前报文，供下一条报文解码时使用
    _cached_previous_ultra_dense_capsuledata = *ultra_dense_capsule;
    _is_previous_capsuledataRdy = true;

}



}


END_DATAUNPACKER_NS()