/*
 *  Slamtec LIDAR SDK
 *
 *  Copyright (c) 2020 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
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

// ===================================================================
// 本文件声明了 SL_LidarDriver 类，即 ILidarDriver 抽象接口的具体实现。
//
// 该类是 SDK 驱动层的核心实现，将 ILidarDriver 中定义的纯虚函数
// 落实为实际的协议交互逻辑。所有与 RPLIDAR 的通信协议细节
// （请求报文构造、应答报文解析、扫描数据组装、多模式适配等）
// 均封装在此类内部。
//
// 核心设计要点：
//   1) 后台缓存线程：startScan() 后启动 _cachethread 后台线程，
//      持续从 IChannel 读取应答数据并解析为 HQ 测量点存入缓存。
//      grabScanDataHq() 从缓存中取出完整一圈数据，实现收发解耦。
//   2) 多扫描模式适配：根据 ans_type(0x81~0x86) 选择不同解码路径，
//      将各类 capsuled 报文统一转换为 sl_lidar_response_measurement_node_hq_t 格式。
//   3) 协议层封装：_sendCommand() 负责构造请求报文(Sync+Cmd+Size+Payload+Checksum)，
//      _waitResponseHeader() 负责解析起始应答报文(0xA5 0x5A + Size/Mode + Type)。
//
// 【通信协议 LR001 P.6】请求报文格式：0xA5 + 命令 + 负载长度 + 负载 + 校验和。
// 【通信协议 LR001 P.8】起始应答报文：0xA5 0x5A + 长度/模式 + 类型 + 数据。
// 【SDK手册 LR002 P.12】SL_LidarDriver 为驱动接口 ILidarDriver 的实现类。
// ===================================================================

#pragma once
// 引入驱动抽象接口定义（ILidarDriver、IChannel、LidarScanMode、
// MotorCtrlSupport、DEPRECATED 宏等），本类继承自 ILidarDriver。
#include "sl_lidar_driver.h"

// 所有 SDK 实现均位于 sl 命名空间下。
namespace sl {
    // ===============================================================
    // SL_LidarDriver：LIDAR 驱动实现类
    //
    // 继承自 ILidarDriver 抽象接口，实现了与 RPLIDAR 的全部通信协议逻辑。
    // 此类是用户通过 createLidarDriver() 工厂函数最终获得的驱动对象类型。
    //
    // 内部架构分层：
    //   - 公有方法(public)：实现 ILidarDriver 接口，供用户直接调用。
    //   - 保护方法(protected)：实现细节，如配置查询、电机启动等辅助操作。
    //   - 私有方法(private)：底层协议收发，如 _sendCommand / _waitResponseHeader /
    //     各类 capsule 解码 / 后台缓存线程函数。
    // ===============================================================
	class SL_LidarDriver :public ILidarDriver
	{
		public:
            // -------------------------------------------------------
            // 采样耗时相关常量
            // LEGACY_SAMPLE_DURATION：传统标准扫描模式(SCAN)下单次激光测距
            // 的经验耗时（476 微秒），作为采样率计算的回退默认值。
            // 当无法通过 GET_SAMPLERATE 命令获取实际值时使用此经验值。
            // 对应通信协议中 Tstandard 字段的回退估计值。
            // 【通信协议 LR001 P.32】Tstandard: 标准扫描模式下单次激光测距耗时。
            // -------------------------------------------------------
			enum {
				LEGACY_SAMPLE_DURATION = 476,
			};

            // -------------------------------------------------------
            // Capsule 类型标识（用于区分传统版本与密实版本的 capsule 报文）
            // 这两个常量用于 _cacheCapsuledScanData() 等函数内部判断
            // 当前接收的 capsule 报文类型，从而选择正确的解码路径。
            //   NORMAL_CAPSULE(0)：传统版本 capsuled（84字节/包，16 cabin×2点=32点）
            //   DENSE_CAPSULE(1)：密实版本 dense capsuled（84字节/包，40 cabin×1点=40点）
            // 【通信协议 LR001 P.19】传统版本应答长度 84 bytes，16组 cabin。
            // 【通信协议 LR001 P.25】密实版本应答长度 84 bytes，40组 cabin。
            // -------------------------------------------------------
			enum
			{
					NORMAL_CAPSULE = 0,
					DENSE_CAPSULE = 1,
			};

            // -------------------------------------------------------
            // 型号判定阈值常量（用于区分不同测距技术的雷达系列）
            // A2A3_LIDAR_MINUM_MAJOR_ID：A2/A3 系列雷达的最低主型号 ID 阈值。
            //   model 字段低 6 位(MajorModel) >= 2 时，认为是 A2 或 A3 系列，
            //   支持 EXPRESS_SCAN 高速采样模式。
            // TOF_LIDAR_MINUM_MAJOR_ID：TOF(飞行时间)测距雷达的最低主型号 ID 阈值。
            //   MajorModel >= 6 时，认为是 S 系列(TOF)或其他 TOF 雷达，
            //   采用不同的数据解析策略。
            // 这两个阈值用于 getLIDARTechnologyType() 等函数按型号推导雷达技术类型。
            // -------------------------------------------------------
			enum {
				A2A3_LIDAR_MINUM_MAJOR_ID  = 2,
				TOF_LIDAR_MINUM_MAJOR_ID = 6,
			};
		public:
            // 构造函数：初始化所有成员变量为安全默认值。
            //   _channel = NULL：未绑定通信通道。
            //   _isConnected = false：未连接。
            //   _isScanning = false：未处于扫描状态。
            //   _isSupportingMotorCtrl = MotorCtrlSupportNone：默认不支持电机控制。
            //   _cached_sampleduration_std/express = LEGACY_SAMPLE_DURATION：
            //     采样耗时回退为传统经验值 476 微秒。
            //   _cached_scan_node_hq_count = 0：缓存扫描数据计数清零。
            //   _cached_scan_node_hq_count_for_interval_retrieve = 0：
            //     interval 检索专用缓存计数清零。
			SL_LidarDriver()
				:_channel(NULL)
				, _isConnected(false)
				, _isScanning(false)
				, _isSupportingMotorCtrl(MotorCtrlSupportNone)
				, _cached_sampleduration_std(LEGACY_SAMPLE_DURATION)
				,_cached_sampleduration_express(LEGACY_SAMPLE_DURATION)
				, _cached_scan_node_hq_count(0)
				, _cached_scan_node_hq_count_for_interval_retrieve(0)
			{}

            // ---- 公有方法：实现 ILidarDriver 接口 ----

            // 连接 LIDAR。绑定通信通道并验证连通性。
            // 保存 channel 引用，可选发送探测命令验证雷达响应。
            sl_result connect(IChannel* channel);

            // 断开连接。停止扫描线程，释放驱动内部资源（不关闭通道）。
            void disconnect();

            // 检查是否已连接。
            bool isConnected();

            // 软重启雷达测距核心。发送 RESET(0x40) 命令。
            // 【通信协议 LR001 P.13】RESET 命令，值 0x40，无负载，无应答。
            sl_result reset(sl_u32 timeoutInMs = DEFAULT_TIMEOUT);

            // 获取设备支持的全部扫描模式列表。
            // 依次发送 GET_LIDAR_CONF(0x84) 查询模式数量及每个模式属性。
            // 【通信协议 LR001 P.36】配置字段类型 0x70~0x7F。
            sl_result getAllSupportedScanModes(std::vector<LidarScanMode>& outModes, sl_u32 timeoutInMs = DEFAULT_TIMEOUT);

            // 获取推荐扫描模式。查询 SL_LIDAR_CONF_SCAN_MODE_TYPICAL(0x7C)。
            // 【通信协议 LR001 P.36】0x7C 获取推荐扫描工作模式ID。
            sl_result getTypicalScanMode(sl_u16& outMode, sl_u32 timeoutInMs = DEFAULT_TIMEOUT);

            // 开始扫描。根据参数选择 SCAN(0x20) 或 EXPRESS_SCAN(0x82)。
            // useTypicalScan=true 时用推荐模式，false 时用标准 SCAN 模式。
            // 【通信协议 LR001 P.14】SCAN 命令，值 0x20，多次应答，5 bytes/点。
            sl_result startScan(bool force, bool useTypicalScan, sl_u32 options = 0, LidarScanMode* outUsedScanMode = nullptr);

            // 标准扫描模式启动（内部方法）。发送 SCAN(0x20) 或 FORCE_SCAN(0x21)。
            // 启动后台缓存线程接收标准测距数据应答(0x81)，每点 5 字节。
            // 【通信协议 LR001 P.15】标准测距点含 syncbit/quality/angle_q6/distance_q2。
            sl_result startScanNormal(bool force, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 指定模式高速采样启动。发送 EXPRESS_SCAN(0x82)，负载含 working_mode。
            // 根据 scanMode 对应的 ans_type 选择后台缓存线程的解码路径。
            // 【通信协议 LR001 P.17】EXPRESS_SCAN 命令，值 0x82，多次应答。
            // 【通信协议 LR001 P.18】负载 5 字节：working_mode + working_flags + param。
            sl_result startScanExpress(bool force, sl_u16 scanMode, sl_u32 options = 0, LidarScanMode* outUsedScanMode = nullptr, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 停止扫描。发送 STOP(0x25) 并终止后台缓存线程。
            // 【通信协议 LR001 P.13】STOP 命令，值 0x25，无负载，无应答。
            sl_result stop(sl_u32 timeout = DEFAULT_TIMEOUT);

            // [已废弃] 获取标准格式扫描数据。已由 grabScanDataHq 替代。
            // 使用 DEPRECATED 宏标记，编译时产生迁移警告。
            DEPRECATED(sl_result grabScanData(sl_lidar_response_measurement_node_t * nodebuffer, size_t& count, sl_u32 timeout = DEFAULT_TIMEOUT));

            // 获取完整一圈扫描数据（HQ 格式）。
            // 从 _cached_scan_node_hq_buf 中取出后台线程已组装的完整扫描圈。
            // 返回数据首节点 syncbit=1，属于一次完整 360 度扫描。
            // 【通信协议 LR001 P.15】S=1 表示新的一圈360度扫描的开始。
            sl_result grabScanDataHq(sl_lidar_response_measurement_node_hq_t* nodebuffer, size_t& count, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 获取设备信息。发送 GET_INFO(0x50) 命令，解析 20 字节应答。
            // 【通信协议 LR001 P.29】GET_INFO 命令，值 0x50，应答 20 bytes。
            sl_result getDeviceInfo(sl_lidar_response_device_info_t& info, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 检查电机控制支持。发送 GET_ACC_BOARD_FLAG(0xFF) 查询转接板能力。
            // 返回 MotorCtrlSupport 枚举(None/Pwm/Rpm)。
            sl_result checkMotorCtrlSupport(MotorCtrlSupport & support, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 计算扫描频率。利用采样点数与单次采样耗时反推雷达转速(Hz)。
            sl_result getFrequency(const LidarScanMode& scanMode, const sl_lidar_response_measurement_node_hq_t* nodes, size_t count, float& frequency);

            // 设置网络雷达静态 IP。通过 SET_LIDAR_CONF(0x85) 配置 IP/掩码/网关。
            sl_result setLidarIpConf(const sl_lidar_ip_conf_t& conf, sl_u32 timeout);

            // 获取网络雷达静态 IP。通过 GET_LIDAR_CONF(0x84) 查询。
            sl_result getLidarIpConf(sl_lidar_ip_conf_t& conf, sl_u32 timeout);

            // 获取雷达健康状态。发送 GET_HEALTH(0x52)，解析 3 字节应答。
            // 【通信协议 LR001 P.31】GET_HEALTH 命令，值 0x52，应答 3 bytes。
            //   status: 0 良好/1 警告/2 错误(保护性停机)。
            sl_result getHealth(sl_lidar_response_device_health_t& health, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 获取雷达 MAC 地址。通过 GET_LIDAR_CONF 查询 0x79 字段。
            sl_result getDeviceMacAddr(sl_u8* macAddrArray, sl_u32 timeoutInMs);

            // [已废弃] 标准格式扫描数据按角度升序排列。已由 HQ 版本替代。
            DEPRECATED(sl_result ascendScanData(sl_lidar_response_measurement_node_t * nodebuffer, size_t count));

            // HQ 格式扫描数据按角度升序排列（原地排序）。
            // grabScanDataHq 返回数据可能角度非升序，此函数重新排序。
            sl_result ascendScanData(sl_lidar_response_measurement_node_hq_t * nodebuffer, size_t count);

            // 获取 interval（增量）扫描数据。返回当前已缓存的所有点，
            // 不要求完整一圈。适合低延迟增量处理场景。
            sl_result getScanDataWithIntervalHq(sl_lidar_response_measurement_node_hq_t * nodebuffer, size_t & count);

            // 设置电机转速。根据电机控制方式选择 PWM(0xF0) 或 RPM(0xA8) 命令。
            // speed=DEFAULT_MOTOR_PWM 时使用默认值并停止扫描。
            // 【通信协议 LR001 P.39】MOTOR_SPEED_CTRL 命令，值 0xA8，负载 2字节 Rpm。
            sl_result setMotorSpeed(sl_u16 speed = DEFAULT_MOTOR_PWM);//

            // 串口波特率协商。发送 NEW_BAUDRATE_CONFIRM(0x90) 切换波特率。
            sl_result negotiateSerialBaudRate(sl_u32 requiredBaudRate, sl_u32* baudRateDetected = NULL);
		
	protected:
            // ---- 保护方法：实现辅助逻辑 ----

            // 启动电机。根据电机控制方式(DTR/PWM/RPM)启动电机旋转，
            // 为 startScan 前的准备工作。
            sl_result startMotor();

            // 检查设备是否支持配置命令(GET_LIDAR_CONF/SET_LIDAR_CONF)。
            // 通过固件版本判断，固件 >= 1.24 才支持配置命令。
            // \param outSupport [输出] 是否支持配置命令。
            // \param timeoutInMs 超时时间。
            sl_result checkSupportConfigCommands(bool& outSupport, sl_u32 timeoutInMs = DEFAULT_TIMEOUT);

            // 获取扫描模式总数。发送 GET_LIDAR_CONF 查询
            // SL_LIDAR_CONF_SCAN_MODE_COUNT(0x70)。
            // 【通信协议 LR001 P.36】0x70 获取扫描工作模式ID最大值。
            sl_result getScanModeCount(sl_u16& modeCount, sl_u32 timeoutInMs = DEFAULT_TIMEOUT);

            // 设置雷达配置参数。发送 SET_LIDAR_CONF(0x85) 命令。
            // \param type 配置字段类型。\param payload 负载数据。\param payloadSize 负载大小。
            sl_result setLidarConf(sl_u32 type, const void* payload, size_t payloadSize, sl_u32 timeout);

            // 获取雷达配置参数。发送 GET_LIDAR_CONF(0x84) 命令。
            // \param type 配置字段类型。\param outputBuf [输出] 接收数据的缓冲区。
            // \param reserve 保留字段(可选)。\param timeout 超时时间。
            // 【通信协议 LR001 P.34】GET_LIDAR_CONF 请求：type(4B) + 可选 payload。
            // 【通信协议 LR001 P.35】GET_LIDAR_CONF 应答：type(4B) + payload[n]。
            sl_result getLidarConf(sl_u32 type, std::vector<sl_u8> &outputBuf, const std::vector<sl_u8> &reserve = std::vector<sl_u8>(), sl_u32 timeout = DEFAULT_TIMEOUT);

            // 获取指定扫描模式的单次采样耗时。查询 0x71 字段。
            // 【通信协议 LR001 P.36】0x71 获取采样频率(微秒)。
            sl_result getLidarSampleDuration(float& sampleDurationRes, sl_u16 scanModeID, sl_u32 timeoutInMs = DEFAULT_TIMEOUT);

            // 获取指定扫描模式的最大测距距离。查询 0x74 字段。
            // 【通信协议 LR001 P.36】0x74 获取最大测距半径(米)。
            sl_result getMaxDistance(float &maxDistance, sl_u16 scanModeID, sl_u32 timeoutInMs = DEFAULT_TIMEOUT);

            // 获取指定扫描模式的数据应答类型。查询 0x75 字段。
            // 返回 0x81~0x85 之一，决定后台线程使用哪个解码函数。
            // 【通信协议 LR001 P.36】0x75 获取 EXPRESS_SCAN 协议版本(数据类型)。
            sl_result getScanModeAnsType(sl_u8 &ansType, sl_u16 scanModeID, sl_u32 timeoutInMs = DEFAULT_TIMEOUT);

            // 获取指定扫描模式的名称。查询 0x7F 字段。
            // 【通信协议 LR001 P.36】0x7F 获取扫描模式可阅读名称。
            sl_result getScanModeName(char* modeName, sl_u16 scanModeID, sl_u32 timeoutInMs = DEFAULT_TIMEOUT);
            //DEPRECATED(sl_result getSampleDuration_uS(sl_lidar_response_sample_rate_t & rateInfo, sl_u32 timeout = DEFAULT_TIMEOUT));
            //DEPRECATED (sl_result checkExpressScanSupported(bool & support, sl_u32 timeout = DEFAULT_TIMEOUT));
            //DEPRECATED(sl_result getFrequency(bool inExpressMode, size_t count, float & frequency, bool & is4kmode));
		private:
            // ---- 私有方法：底层协议收发与数据解码 ----

            // 发送请求命令报文。
            // 构造请求报文：Sync(0xA5) + Cmd + PayloadSize + Payload + Checksum(异或校验)。
            // 校验和 = 0 ^ 0xA5 ^ Cmd ^ PayloadSize ^ Payload[0] ^ ... ^ Payload[n]。
            // \param cmd 命令码（如 SCAN=0x20、EXPRESS_SCAN=0x82 等）。
            // \param payload 负载数据指针（无负载时传 NULL）。
            // \param payloadsize 负载字节数。
            // 【通信协议 LR001 P.6】请求报文格式与校验和计算公式。
            sl_result  _sendCommand(sl_u16 cmd, const void * payload = NULL, size_t payloadsize = 0 );

            // 等待并解析起始应答报文头。
            // 从通道读取 7 字节起始应答：0xA5 0x5A + size_q30_subtype(4B) + type(1B)。
            // 验证同步字节(0xA5 0x5A)，解析数据应答长度(低30位)和应答模式(高2位)。
            // \param header [输出] 填充解析后的应答头信息。
            // \param timeout 超时时间。
            // 【通信协议 LR001 P.8】起始应答报文结构。
            sl_result _waitResponseHeader(sl_lidar_ans_header_t * header, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 等待并解析单次应答的负载数据（模板方法）。
            // 先调用 _waitResponseHeader 验证应答类型，再读取指定长度的负载。
            // \param payload [输出] 接收的负载数据(T 为负载结构体类型)。
            // \param ansType 期望的应答数据类型（如 0x04=设备信息、0x06=健康状态）。
            // \param timeout 超时时间。
            template <typename T>
            sl_result _waitResponse(T &payload ,sl_u8 ansType, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 禁用数据抓取。停止后台缓存线程，重置扫描状态标志。
            // 在 stop()/disconnect() 等需要终止数据流时调用。
            void _disableDataGrabbing();

            // 等待并解析单个标准测距数据节点（5 字节）。
            // 读取并解析 SCAN 命令的应答点(sl_lidar_response_measurement_node_t)。
            // 提取 syncbit、quality、angle_q6(需/64)、distance_q2(需/4)。
            // \param node [输出] 解析后的测距点数据。
            // \param timeout 超时时间。
            // 【通信协议 LR001 P.14~P.15】标准测距数据应答格式（5字节/点）。
            sl_result _waitNode(sl_lidar_response_measurement_node_t * node, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 批量等待标准测距数据节点（循环调用 _waitNode）。
            // \param nodebuffer 测距点缓冲区。\param count [输入/输出] 容量/实际数量。
            // \param timeout 超时时间。
            sl_result _waitScanData(sl_lidar_response_measurement_node_t * nodebuffer, size_t & count, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 后台缓存线程函数（标准扫描模式）。
            // 循环调用 _waitScanData 接收测距点，转换为 HQ 格式存入
            // _cached_scan_node_hq_buf 缓冲区。通过 _dataEvt 事件通知
            // grabScanDataHq() 数据就绪。
            sl_result _cacheScanData();

            // 将扩展版本(ultra capsuled)报文解码为标准 HQ 测距点数组。
            // ultra capsule 含 32 个 ultra_cabin(每个3点，共96点)，采用
            // SLAMTEC 专利可变比例编码(VarBitScale)压缩。
            // 需结合上一包的起止角度进行角度插值，故使用 _cached_previous_ultracapsuledata。
            // \param capsule 扩展版本 capsule 报文（132 字节）。
            // \param nodebuffer [输出] HQ 测距点数组。\param nodeCount [输出] 解码点数。
            // 【通信协议 LR001 P.22~P.23】扩展版本 ultra cabin 结构与可变比例编码。
            void _ultraCapsuleToNormal(const sl_lidar_response_ultra_capsule_measurement_nodes_t & capsule, sl_lidar_response_measurement_node_hq_t *nodebuffer, size_t &nodeCount);

            // 等待并解析一个扩展版本(ultra capsuled)数据应答报文（132 字节）。
            // 验证同步字节(sync1=0xA, sync2=0x5)和校验和(按字节异或)。
            // \param node [输出] 接收的 ultra capsule 报文。\param timeout 超时时间。
            // 【通信协议 LR001 P.20】sync1=0xA, sync2=0x5, ChkSum 按字节异或。
            sl_result _waitCapsuledNode(sl_lidar_response_capsule_measurement_nodes_t & node, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 将传统版本(capsuled)报文解码为标准 HQ 测距点数组。
            // 传统 capsule 含 16 个 cabin(每个2点，共32点)，每个 cabin 的距离
            // 和角度补偿量(dθ)为复合位域字段，需用掩码分离。
            // 角度计算：基于 start_angle_q6 加上各点角度补偿量。
            // \param capsule 传统版本 capsule 报文（84 字节）。
            // \param nodebuffer [输出] HQ 测距点数组。\param nodeCount [输出] 解码点数。
            // 【通信协议 LR001 P.19~P.21】传统版本 cabin 结构与位域定义。
            void _capsuleToNormal(const sl_lidar_response_capsule_measurement_nodes_t & capsule, sl_lidar_response_measurement_node_hq_t *nodebuffer, size_t &nodeCount);

            // 将密实版本(dense capsuled)报文解码为标准 HQ 测距点数组。
            // 密实 capsule 含 40 个 dense_cabin(每个1点，共40点)，每 cabin
            // 仅含距离值(无角度补偿)，角度由相邻包起始角度线性插值计算。
            // \param capsule 密实版本 capsule 报文（84 字节）。
            // \param nodebuffer [输出] HQ 测距点数组。\param nodeCount [输出] 解码点数。
            // 【通信协议 LR001 P.24~P.26】密实版本 dense cabin 结构。
            void _dense_capsuleToNormal(const sl_lidar_response_capsule_measurement_nodes_t & capsule, sl_lidar_response_measurement_node_hq_t *nodebuffer, size_t &nodeCount);

            // 后台缓存线程函数（传统/密实 capsule 扫描模式）。
            // 循环接收 capsule 报文并解码为 HQ 点存入缓存。
            // 根据扫描模式选择 _capsuleToNormal 或 _dense_capsuleToNormal 解码。
            sl_result _cacheCapsuledScanData();

            // 将超密实版本(ultra dense capsuled)报文解码为标准 HQ 测距点数组。
            // 超密实 capsule 含 32 个 ultra_dense_cabin，每 cabin 含 2 组距离与质量数据。
            // SDK 扩展格式，非协议标准命令表所列。
            // \param capslue 超密实 capsule 报文。\param nodebuffer [输出] HQ 点数组。\param nodeCount [输出] 解码点数。
            void _ultra_dense_capsuleToNormal(const sl_lidar_response_ultra_dense_capsule_measurement_nodes_t& capslue, sl_lidar_response_measurement_node_hq_t* nodebuffer, size_t& nodeCount);

            // 等待并解析一个超密实版本(ultra dense capsuled)数据应答报文。
            // \param node [输出] 接收的超密实 capsule 报文。\param timeout 超时时间。
            sl_result _waitUltraDenseCapsuledNode(sl_lidar_response_ultra_dense_capsule_measurement_nodes_t& node, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 后台缓存线程函数（超密实 capsule 扫描模式）。
            // 循环接收超密实 capsule 报文并解码为 HQ 点存入缓存。
            sl_result _cacheUltraDenseCapsuledScanData();
			

            // 等待并解析一个 HQ 扫描模式数据应答报文。
            // HQ 报文含 sync_byte(0xA5) + time_stamp(8B) + 96×node_hq + crc32(4B)。
            // 使用 CRC32(IEEE 802.3 多项式 0x4C11DB7)校验数据完整性。
            // \param node [输出] 接收的 HQ capsule 报文。\param timeout 超时时间。
            // HQ 模式应答类型为 SL_LIDAR_ANS_TYPE_MEASUREMENT_HQ(0x83)。
            sl_result _waitHqNode(sl_lidar_response_hq_capsule_measurement_nodes_t & node, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 将 HQ capsule 报文解码为标准 HQ 测距点数组。
            // HQ 报文直接包含 96 个 HQ 测距点(angle_z_q14/dist_mm_q2/quality/flag)，
            // 仅需简单拷贝并处理同步标志位即可，无需复杂角度插值。
            // \param node_hq HQ capsule 报文。\param nodebuffer [输出] HQ 点数组。\param nodeCount [输出] 解码点数。
            void _HqToNormal(const sl_lidar_response_hq_capsule_measurement_nodes_t & node_hq, sl_lidar_response_measurement_node_hq_t *nodebuffer, size_t &nodeCount);

            // 后台缓存线程函数（HQ 扫描模式）。
            // 循环接收 HQ capsule 报文并解码为 HQ 点存入缓存。
            sl_result _cacheHqScanData();

            // 等待并解析一个扩展版本(ultra capsuled)数据应答报文（132 字节）。
            // 验证同步字节和校验和后返回 ultra capsule 报文数据。
            // \param node [输出] 接收的 ultra capsule 报文。\param timeout 超时时间。
            // 【通信协议 LR001 P.22】扩展版本应答 132 bytes，32 组 ultra cabin。
            sl_result _waitUltraCapsuledNode(sl_lidar_response_ultra_capsule_measurement_nodes_t & node, sl_u32 timeout = DEFAULT_TIMEOUT);

            // 后台缓存线程函数（扩展版本 ultra capsule 扫描模式）。
            // 循环接收 ultra capsule 报文并经 _ultraCapsuleToNormal 解码为 HQ 点存入缓存。
            sl_result _cacheUltraCapsuledScanData();

            // 清空接收数据缓存。丢弃所有已缓存的扫描数据，
            // 在重新开始扫描前调用以确保数据干净。
            sl_result _clearRxDataCache();

		private:
            // ---- 私有成员变量：驱动运行时状态与缓存 ----

            // 绑定的通信通道指针。由 connect() 注入，驱动通过此通道
            // 发送请求报文和接收应答报文。通道生命周期由外部管理。
            IChannel *_channel;

            // 是否已连接。connect() 成功后置 true，disconnect()/错误后置 false。
            bool _isConnected;

            // 是否正在扫描。startScan() 后置 true，stop() 后置 false。
            // 后台缓存线程据此判断是否继续接收数据。
            bool _isScanning;

            // 电机控制支持类型。由 checkMotorCtrlSupport() 查询得出，
            // 决定 setMotorSpeed() 采用 PWM 还是 RPM 命令。
            MotorCtrlSupport _isSupportingMotorCtrl;

            // 互斥锁。保护缓存缓冲区的线程安全访问。
            // 后台缓存线程(写入)与 grabScanDataHq(读取)并发访问时加锁。
            rp::hal::Locker         _lock;

            // 数据就绪事件。后台缓存线程每组装完一圈完整扫描后
            // 触发此事件，通知 grabScanDataHq() 可以取数据了。
            rp::hal::Event          _dataEvt;

            // 后台缓存线程对象。startScan() 时创建并启动，
            // 持续从通道接收应答数据并解码到缓存。stop() 时终止。
            rp::hal::Thread         _cachethread;

            // 标准扫描模式(SCAN)的单次采样耗时缓存（微秒）。
            // 用于 getFrequency() 计算扫描频率。初始为 LEGACY_SAMPLE_DURATION(476)。
            sl_u16                    _cached_sampleduration_std;

            // 高速采样模式(EXPRESS_SCAN)的单次采样耗时缓存（微秒）。
            // 用于 getFrequency() 计算。初始为 LEGACY_SAMPLE_DURATION(476)。
            sl_u16                    _cached_sampleduration_express;

            // HQ 格式扫描数据缓存缓冲区（主缓冲区）。
            // 后台缓存线程将各类 capsule 报文解码后的 HQ 测量点存入此处。
            // grabScanDataHq() 从此缓冲区取出完整一圈数据。
            // 容量 8192 点，足以容纳一圈扫描的所有采样点。
            sl_lidar_response_measurement_node_hq_t   _cached_scan_node_hq_buf[8192];

            // 主缓冲区中当前已缓存的 HQ 测量点数量。
            // 后台线程每解码一个点就递增，grabScanDataHq 取出后清零或回绕。
            size_t                                   _cached_scan_node_hq_count;

            // Capsule 报文解析标志位缓存。用于在 capsule 解码过程中
            // 标记是否已收到第一个包含同步标志(S=1)的报文，
            // 确保从一圈扫描的起点开始缓存数据。
            sl_u8                                    _cached_capsule_flag;

            // interval 检索专用 HQ 数据缓冲区。
            // getScanDataWithIntervalHq() 从此缓冲区返回增量数据，
            // 与主缓冲区分离避免与 grabScanDataHq() 竞争。
            sl_lidar_response_measurement_node_hq_t   _cached_scan_node_hq_buf_for_interval_retrieve[8192];

            // interval 缓冲区中当前缓存的 HQ 测量点数量。
            size_t                                   _cached_scan_node_hq_count_for_interval_retrieve;

            // 上一包传统/密实 capsule 报文缓存。
            // capsule 解码需要上一包的起止角度进行角度插值，
            // 故缓存上一包数据。_is_previous_capsuledataRdy 标记是否已有有效上一包。
            sl_lidar_response_capsule_measurement_nodes_t       _cached_previous_capsuledata;

            // 上一包密实版本(dense) capsule 报文缓存。
            // 密实版本角度插值同样需要上一包数据。
            sl_lidar_response_dense_capsule_measurement_nodes_t _cached_previous_dense_capsuledata;

            // 上一包扩展版本(ultra) capsule 报文缓存。
            // 扩展版本的可变比例编码解码需要上一包的起止角度作为基准。
            sl_lidar_response_ultra_capsule_measurement_nodes_t _cached_previous_ultracapsuledata;

            // 上一包 HQ capsule 报文缓存。
            // HQ 模式通常不需要上一包数据（点数据自包含），
            // 但保留以备时间戳连续性处理等场景。
            sl_lidar_response_hq_capsule_measurement_nodes_t _cached_previous_Hqdata;

            // 传统/密实 capsule 上一包数据是否就绪标志。
            // 首次接收 capsule 时为 false，收到第一包后置 true。
            // 在此标志为 false 时不能进行角度插值（无上一包基准）。
            bool                                         _is_previous_capsuledataRdy;

            // HQ capsule 上一包数据是否就绪标志。
            // 用于 HQ 模式的时间戳连续性等处理。
            bool                                         _is_previous_HqdataRdy;
	};

}
