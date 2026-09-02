/*
* Slamtec LIDAR SDK
*
* sl_lidar_cmd.h
*
* Copyright (c) 2020 Shanghai Slamtec Co., Ltd.
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
// 本文件定义了 RPLIDAR 通讯协议文档中描述的各类请求/应答相关的
// 数据结构和常量定义。包含：
//   - 所有请求命令码(STOP/SCAN/EXPRESS_SCAN/GET_INFO/GET_HEALTH 等)
//   - 请求负载数据结构(working_mode/pwm/rpm 等)
//   - 应答数据类型(0x81~0x86 等)
//   - 各类应答数据结构(measurement_node/cabin/capsule/device_info 等)
//   - GET_LIDAR_CONF 配置字段类型(0x70~0x7F 等)
//   - SLAMTEC 专利的可变比例编码(VarBitScale)的解码常量
// 【SDK手册 LR002 P.12】sl_lidar_cmd.h 定义了 RPLIDAR 通讯协议文档中
//   描述的各类请求/应答相关的数据结构和常量定义。
// 【通信协议 LR001 P.12】请求命令总览图表 4-1 列出了被 RPLIDAR 支持的
//   请求命令。
// ===================================================================

#pragma once

// MSVC 下关闭 C4200 警告（零长度数组 data[0] 为 GCC 扩展）。
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4200)
#endif

// 引入协议基础常量与请求/应答报文头结构（同步字节、命令标志位、
// sl_lidar_cmd_packet_t、sl_lidar_ans_header_t 等）。
#include "sl_lidar_protocol.h"

 // Commands  请求命令码定义
 //-----------------------------------------

// 自动波特率检测时使用的魔数字节。
// SDK 在检测雷达实际波特率时会发送该字节，雷达据此进行波特率自适应。
// 非协议文档定义的标准命令，属 SDK 内部使用的辅助字节。
#define SL_LIDAR_AUTOBAUD_MAGICBYTE         0x41

 // 无负载且无应答的命令（单次请求-无应答模式）
// 【通信协议 LR001 P.6】单次请求-无应答模式：对于停止扫描、重启测距核心
//   这类请求命令，RPLIDAR 采用单次请求但不做应答的通讯模式。

// 停止扫描(STOP)命令。请求报文: A5 25。
// 发送后 RPLIDAR 将退出正在进行的扫描采样状态，关闭测距系统和激光器，
// 进入空闲模式。RPLIDAR 不会为该请求发送回应报文，建议发送后延迟 1ms 以上。
// 【通信协议 LR001 P.13】停止扫描(STOP)命令请求，值 0x25，无负载，
//   无应答，离开扫描采样模式进入空闲状态，支持固件版本 1.0。
#define SL_LIDAR_CMD_STOP                   0x25

// 开始扫描采样(SCAN)命令。请求报文: A5 20。
// 请求进入扫描采样状态。数据应答类型为多次，长度 5 bytes/点。
// 注意：对支持4kHz及以上采样频率的设备在该命令下将降低自身采样频率，
// 且超过16米的测量数据会被丢弃，请使用 EXPRESS_SCAN 获得最佳性能。
// 本命令仅支持 Legacy 工作模式。
// 【通信协议 LR001 P.14】开始扫描采样(SCAN)命令请求，值 0x20，无负载，
//   多次应答，数据应答长度 5 bytes，支持固件版本 1.0。
#define SL_LIDAR_CMD_SCAN                   0x20

// 强制扫描采样(FORCE_SCAN)命令。请求报文: A5 21。
// 使 RPLIDAR 忽略当前扫描电机的工作状态而强行进行扫描测距并发送数据应答，
// 可用于设备测试。起始应答与数据应答均与 SCAN 命令一致（5 bytes/点）。
// 【通信协议 LR001 P.28】强制扫描采样(FORCE_SCAN)命令请求，值 0x21，
//   多次应答，数据应答长度 5 bytes，支持固件版本 1.0。
#define SL_LIDAR_CMD_FORCE_SCAN             0x21

// 测距核心软重启(RESET)命令。请求报文: A5 40。
// 测距核心将进行软重启，恢复到与通电后一样的状态。当 RPLIDAR 因故障
// 进入保护性停机后，可发送 RESET 尝试恢复。无应答，建议延迟 2ms 以上。
// 【通信协议 LR001 P.13】测距核心软重启(RESET)命令请求，值 0x40，无负载，
//   无应答，测距核心软重启，支持固件版本 1.0。
#define SL_LIDAR_CMD_RESET                  0x40

// 带负载但无应答的命令
// 确认新波特率命令。固件 1.30 新增。用于在线切换通讯波特率后进行确认，
// 雷达据此将通讯速率固化到新波特率。
// 非协议标准命令表(图表4-1)所列，为 SDK 扩展命令。
#define SL_LIDAR_CMD_NEW_BAUDRATE_CONFIRM   0x90 //added in fw 1.30

// 无负载但有应答的命令（单次请求-单次应答模式）
// 【通信协议 LR001 P.7】单次请求-单次应答模式：RPLIDAR 在收到这类请求后，
//   将在必要的操作后通过单个应答包发送外部系统需要的数据。

// 设备信息获取(GET_INFO)命令。请求报文: A5 50。
// 起始应答: A5 5A 14 00 00 00 04，单次应答，长度 20 bytes。
// 应答包含型号、固件/硬件版本、16字节序列号。
// 【通信协议 LR001 P.29】设备信息获取(GET_INFO)命令请求，值 0x50，无负载，
//   单次应答，数据应答长度 20 bytes，支持固件版本 1.0。
#define SL_LIDAR_CMD_GET_DEVICE_INFO        0x50

// 设备健康状态获取(GET_HEALTH)命令。请求报文: A5 52。
// 起始应答: A5 5A 03 00 00 00 06，单次应答，长度 3 bytes。
// 应答包含 status(0良好/1警告/2错误) 与 error_code。
// 【通信协议 LR001 P.31】设备健康状态获取(GET_HEALTH)命令请求，值 0x52，
//   无负载，单次应答，数据应答长度 3 bytes，支持固件版本 1.0。
#define SL_LIDAR_CMD_GET_DEVICE_HEALTH      0x52

// 激光测距用时获取(GET_SAMPLERATE)命令。请求报文: A5 59。
// 起始应答: A5 5A 04 00 00 00 15，单次应答，长度 4 bytes。
// 应答包含 Tstandard 与 Texpress（单位微秒），用于计算雷达旋转速度。
// 固件 1.17 新增。
// 【通信协议 LR001 P.32】激光测距用时获取(GET_SAMPLERATE)命令请求，
//   值 0x59，无负载，单次应答，数据应答长度 4 bytes，支持固件版本 1.17。
#define SL_LIDAR_CMD_GET_SAMPLERATE         0x59 //added in fw 1.17

// 设备转速控制(MOTOR_SPEED_CTRL)命令。请求报文: A5 A8 02 Rpm C。
// 设置 RPLIDAR 测距核心的旋转速度(Rpm)，支持在线调速。转速设为0时进入空闲。
// 负载为 2 字节的 Rpm 值。目前该命令仅支持 S 系列雷达。
// 【通信协议 LR001 P.39】设备转速控制(MOTOR_SPEED_CTRL)命令请求，值 0xA8，
//   有负载(2字节Rpm)，无应答（注：协议文档将其归为转速控制类请求）。
#define SL_LIDAR_CMD_HQ_MOTOR_SPEED_CTRL    0xA8


// 带负载且有应答的命令
// 开始高速采样(EXPRESS_SCAN)命令。请求报文: A5 82 05 <payload> C。
// 使 RPLIDAR 使用尽可能快的采样频率工作。负载为 5 字节
// (sl_lidar_payload_express_scan_t)。分传统版本(84B)/扩展版本(132B)/
// 密实版本(84B)。固件 1.17 新增。
// 注意：本宏值 0x82 = 0x80(SL_LIDAR_CMDFLAG_HAS_PAYLOAD) | 0x02。
// 【通信协议 LR001 P.17】开始高速采样(EXPRESS_SCAN)命令请求，值 0x82，
//   有负载，多次应答，支持固件版本 1.17。
#define SL_LIDAR_CMD_EXPRESS_SCAN           0x82 //added in fw 1.17

// HQ 扫描命令。固件 1.24 新增。HQ(High Quality)扫描模式的数据应答使用
// CRC32 校验（见 sl_crc.cpp），每个报文含 96 个 HQ 测距点。
// 非协议标准命令表所列，为 SDK 扩展命令。
#define SL_LIDAR_CMD_HQ_SCAN                0x83 //added in fw 1.24

// 设备配置信息获取(GET_LIDAR_CONF)命令。请求报文: A5 84 S <data> C。
// 按地址(配置字段类型 type)获取雷达配置信息，应答为可变长。
// 固件 1.24 新增。本宏值 0x84 = 0x80 | 0x04。
// 【通信协议 LR001 P.34】设备配置信息获取(GET_LIDAR_CONF)命令请求，
//   值 0x84，有负载，单次应答，数据应答长度可变长，支持固件版本 1.24。
#define SL_LIDAR_CMD_GET_LIDAR_CONF         0x84 //added in fw 1.24

// 设备配置信息设置(SET_LIDAR_CONF)命令。固件 1.24 新增。
// 用于设置雷达的可写配置参数。非协议标准命令表所列，为 SDK 扩展命令。
#define SL_LIDAR_CMD_SET_LIDAR_CONF         0x85 //added in fw 1.24

// 为 A2 配套转接板设置 RPLIDAR 电机 PWM 占空比。
// 当使用 accessory board(转接板)时，通过该命令控制电机转速。
// 非协议标准命令，为 SDK 扩展命令。
//add for A2 to set RPLIDAR motor pwm when using accessory board
#define SL_LIDAR_CMD_SET_MOTOR_PWM          0xF0

// 获取转接板(accessory board)标志位命令。
// 用于判断当前所用转接板是否支持电机控制等功能。
// 非协议标准命令，为 SDK 扩展命令。
#define SL_LIDAR_CMD_GET_ACC_BOARD_FLAG     0xFF

// Windows 下使用 1 字节紧凑对齐，保证协议结构体在线缆上的字节布局精确。
#if defined(_WIN32)
#pragma pack(1)
#endif


// Payloads  请求负载数据结构定义
// ------------------------------------------
// 【通信协议 LR001 P.18】EXPRESS_SCAN 命令需要包含5个字节的负载数据，
//   其结构不可省略。负载结构：working_mode(1B) + Reserved(4B)。
//   注：协议文档定义负载为 5 字节(1B working_mode + 4B reserved)，
//   本 SDK 实现将其扩展为 working_mode(1B)+working_flags(2B)+param(2B)，
//   共 5 字节，与协议总长度一致。

// 传统版本工作模式：working_mode=0 时采用传统版本协议工作。
// 【通信协议 LR001 P.18】当数值为0时，将采用传统版本的定义进行工作。
#define SL_LIDAR_EXPRESS_SCAN_MODE_NORMAL      0
// 定角扫描模式（不再支持，保留以避免编译错误）。
#define SL_LIDAR_EXPRESS_SCAN_MODE_FIXANGLE    0  // won't been supported but keep to prevent build fail
// 高速采样工作标志位（用于扩展高速采样协议）
// BOOST 标志：性能优先模式，优先提升采样率。
// 对应图表3-3 中 Boost 模式。
//for express working flag(extending express scan protocol)
#define SL_LIDAR_EXPRESS_SCAN_FLAG_BOOST                 0x0001
// 抗日光标志：提升环境光抗击能力。
#define SL_LIDAR_EXPRESS_SCAN_FLAG_SUNLIGHT_REJECTION    0x0002

// Ultra express 工作标志位（扩展版本/ultra 版本专用）
// 标准模式标志。
//for ultra express working flag
#define SL_LIDAR_ULTRAEXPRESS_SCAN_FLAG_STD                 0x0001
// 高灵敏度模式标志：以提升测量距离、测量灵敏度为前提，牺牲一定的环境光抗击能力。
// 对应图表3-3 中 Sensitivity 模式。
#define SL_LIDAR_ULTRAEXPRESS_SCAN_FLAG_HIGH_SENSITIVITY    0x0002

// EXPRESS_SCAN 请求负载数据结构（5 字节）。
// 对应通信协议 PDF 第18页图表4-9 高速采样模式请求报文的数据负载结构定义。
// 【通信协议 LR001 P.18】字节偏移 +0 working_mode；+1~+4 Reserved(必须为0)。
typedef struct _sl_lidar_payload_express_scan_t
{
    // 扫描工作模式。0=传统版本；其余值=通过 GET_LIDAR_CONF 获取的模式编号，
    // 采用对应的扫描工作模式进行工作。
    // 【通信协议 LR001 P.18】working_mode: RPLIDAR所需要进入的扫描工作模式。
    sl_u8   working_mode;
    // 工作标志位（SDK 扩展，协议中为 Reserved）。用于控制 BOOST/抗日光等特性。
    sl_u16  working_flags;
    // 预留参数（SDK 扩展，协议中为 Reserved，必须为0）。
    sl_u16  param;
} __attribute__((packed)) sl_lidar_payload_express_scan_t;

// HQ 扫描请求负载结构。flag 为工作标志，reserved 为保留字段。
// 非协议标准结构，为 HQ 扫描命令(SL_LIDAR_CMD_HQ_SCAN)专用。
typedef struct _sl_lidar_payload_hq_scan_t
{
    // HQ 扫描工作标志。
    sl_u8  flag;
    // 保留字段。
    sl_u8   reserved[32];
} __attribute__((packed)) sl_lidar_payload_hq_scan_t;

// GET_LIDAR_CONF 请求负载结构。
// 对应通信协议 PDF 第34页图表4-32：数据请求报文格式 type(4B) + 可选 payload。
// 【通信协议 LR001 P.34】字节偏移 +0 type(32bits)；+4 payload[0]...可选。
typedef struct _sl_lidar_payload_get_scan_conf_t
{
    // 需要获取的配置数据的字段类型（如 0x70=模式数量、0x7C=典型模式等）。
    // 【通信协议 LR001 P.34】type: 需要获取的配置数据的字段类型。
    sl_u32  type;
} __attribute__((packed)) sl_lidar_payload_get_scan_conf_t;

// SET_LIDAR_CONF 请求负载结构。type 指定要设置的配置字段类型。
// 非协议标准结构，为 SDK 扩展命令专用。
typedef struct _sl_lidar_payload_set_scan_conf_t {
    // 要设置的配置字段类型。
    sl_u32  type;
} __attribute__((packed)) sl_lidar_payload_set_scan_conf_t;


// 默认电机转速占空比。0xFFFF 表示使用默认值（不主动设置电机转速）。
#define DEFAULT_MOTOR_SPEED         (0xFFFFu)

// SET_MOTOR_PWM 请求负载结构（转接板电机 PWM 控制）。
// 用于 A2 配套转接板设置电机 PWM 占空比以控制转速。
typedef struct _sl_lidar_payload_motor_pwm_t
{
    // PWM 占空比数值。
    sl_u16 pwm_value;
} __attribute__((packed)) sl_lidar_payload_motor_pwm_t;

// GET_ACC_BOARD_FLAG 请求负载结构。reserved 为保留字段。
typedef struct _sl_lidar_payload_acc_board_flag_t
{
    // 保留字段。
    sl_u32 reserved;
} __attribute__((packed)) sl_lidar_payload_acc_board_flag_t;

// MOTOR_SPEED_CTRL 请求负载结构（2 字节 Rpm）。
// 对应通信协议 PDF 第39页图表4-37：设备转速控制命令请求对应的数据请求报文。
// 【通信协议 LR001 P.39】字节偏移 +0 Rpm(16bits)。设置转速为0时进入空闲状态。
typedef struct _sl_lidar_payload_hq_spd_ctrl_t {
    // 目标旋转速度（RPM，转/分钟）。
    // 【通信协议 LR001 P.39】Rpm: 外部系统设置 RPLIDAR 测距核心的旋转速度。
    sl_u16  rpm;
} __attribute__((packed))sl_lidar_payload_hq_spd_ctrl_t;


// 新波特率确认请求负载结构。
// flag 固定为 0x5F5F，required_bps 为目标波特率，param 为附加参数。
// 非协议标准结构，为 SDK 扩展命令(SL_LIDAR_CMD_NEW_BAUDRATE_CONFIRM)专用。
typedef struct _sl_lidar_payload_new_bps_confirmation_t {
    // 保留标志，必须为 0x5F5F。
    sl_u16   flag; // reserved, must be 0x5F5F
    // 要求切换到的目标波特率。
    sl_u32  required_bps;
    // 附加参数。
    sl_u16  param;
} __attribute__((packed)) sl_lidar_payload_new_bps_confirmation_t;

// Response  应答数据类型与数据结构定义
// ------------------------------------------
// 以下宏定义了起始应答报文中的“数据类型(type)”字段取值，
// 对应通信协议 PDF 第8页起始应答报文结构中的“数据类型 1byte”。
// 【通信协议 LR001 P.8】数据类型表示了数据应答报文发送内容的类型，
//   它与 RPLIDAR 接收到的请求报文类型所对应。

// GET_INFO 命令的应答数据类型。应答长度 20 bytes。
// 【通信协议 LR001 P.29】起始应答: A5 5A 14 00 00 00 04，数据应答长度 20 bytes。
#define SL_LIDAR_ANS_TYPE_DEVINFO          0x4

// GET_HEALTH 命令的应答数据类型。应答长度 3 bytes。
// 【通信协议 LR001 P.31】起始应答: A5 5A 03 00 00 00 06，数据应答长度 3 bytes。
#define SL_LIDAR_ANS_TYPE_DEVHEALTH        0x6

// SCAN/FORCE_SCAN 命令的标准测距数据应答类型。多次应答，每点 5 bytes。
// 对应通信协议 PDF 第14页数据应答报文格式(measurement_node_t)。
// 【通信协议 LR001 P.14】起始应答: A5 5A 05 00 00 40 81，数据应答长度 5 bytes。
#define SL_LIDAR_ANS_TYPE_MEASUREMENT                       0x81
// Added in FW ver 1.17
// EXPRESS_SCAN 传统版本(capsuled)应答类型。多次应答，每包 84 bytes，含 16 cabin(32点)。
// 对应通信协议 PDF 第19页图表4-11 高速扫描测距输出的数据应答报文格式(传统版本)。
// 【通信协议 LR001 P.17】传统版本起始应答: A5 5A 54 00 00 40 82，数据应答长度 84 bytes。
#define SL_LIDAR_ANS_TYPE_MEASUREMENT_CAPSULED              0x82
// HQ 扫描模式应答类型。该模式应答报文使用 CRC32 校验(见 sl_crc.cpp)。
// 非协议标准命令表所列，为 SDK 扩展类型。
#define SL_LIDAR_ANS_TYPE_MEASUREMENT_HQ                    0x83
//added in FW ver 1.23alpha
// EXPRESS_SCAN 扩展版本(ultra capsuled)应答类型。多次应答，每包 132 bytes，
// 含 32 个 ultra_cabin(96点)。对应通信协议 PDF 第22页图表4-15(扩展版本)。
// 【通信协议 LR001 P.17】扩展版本1起始应答: A5 5A 84 00 00 40 84，数据应答长度 132 bytes。
// 【通信协议 LR001 P.37】0x83 - 该模式将采用扩展版本格式回传扫描测距数据。
#define SL_LIDAR_ANS_TYPE_MEASUREMENT_CAPSULED_ULTRA        0x84
// EXPRESS_SCAN 密实版本(dense capsuled)应答类型。多次应答，每包 84 bytes，
// 含 40 个 dense cabin(40点)。对应通信协议 PDF 第24页图表4-19(密实版本)。
// 【通信协议 LR001 P.17】密实版本起始应答: A5 5A 54 00 00 40 85，数据应答长度 84 bytes。
#define SL_LIDAR_ANS_TYPE_MEASUREMENT_DENSE_CAPSULED        0x85
// 超密实版本(ultra dense capsuled)应答类型。SDK 扩展类型，非协议标准命令表所列。
#define SL_LIDAR_ANS_TYPE_MEASUREMENT_ULTRA_DENSE_CAPSULED  0x86


// Added in FW ver 1.17
// GET_SAMPLERATE 命令的应答数据类型。应答长度 4 bytes。
// 【通信协议 LR001 P.32】起始应答: A5 5A 04 00 00 00 15，数据应答长度 4 bytes。
#define SL_LIDAR_ANS_TYPE_SAMPLE_RATE      0x15

//added in FW ver 1.24
// GET_LIDAR_CONF 命令的应答数据类型。应答长度可变。
// 【通信协议 LR001 P.34】起始应答: A5 5A S 00 00 00 20，数据应答长度可变长。
#define SL_LIDAR_ANS_TYPE_GET_LIDAR_CONF     0x20
// SET_LIDAR_CONF 命令的应答数据类型。SDK 扩展类型。
#define SL_LIDAR_ANS_TYPE_SET_LIDAR_CONF     0x21


// GET_ACC_BOARD_FLAG 命令的应答数据类型。SDK 扩展类型。
#define SL_LIDAR_ANS_TYPE_ACC_BOARD_FLAG   0xFF

// 转接板功能支持标志掩码：第0位为1表示支持电机控制。
#define SL_LIDAR_RESP_ACC_BOARD_FLAG_MOTOR_CTRL_SUPPORT_MASK      (0x1)
// GET_ACC_BOARD_FLAG 应答结构。support_flag 的各比特表示转接板支持的功能。
typedef struct _sl_lidar_response_acc_board_flag_t
{
    // 转接板功能支持标志。bit0=1 表示支持电机控制。
    sl_u32 support_flag;
} __attribute__((packed)) sl_lidar_response_acc_board_flag_t;


// GET_HEALTH 应答中的健康状态取值定义。
// 对应通信协议 PDF 第31页 status 字段取值定义。
// 【通信协议 LR001 P.31】status 取值：0 状态良好；1 警告；2 错误。
//   当为错误时，RPLIDAR 已进入保护性停机状态。
#define SL_LIDAR_STATUS_OK                 0x0
#define SL_LIDAR_STATUS_WARNING            0x1
#define SL_LIDAR_STATUS_ERROR              0x2

// -------------------------------------------------------------------
// SCAN 命令标准测距数据应答(measurement_node_t)的位域定义
// 对应通信协议 PDF 第14-15页图表4-4 数据应答报文格式与字段定义。
// 每个测距点 5 字节：
//   +0: [S(syncbit) | S_bar(syncbit_inverse) | C(checkbit) | quality(6bits高位)]
//   +1~+2: [checkbit(1bit) | angle_q6(15bits)] 小端拼接
//   +3~+4: distance_q2(16bits) 小端
// 【通信协议 LR001 P.15】S=1 表示新的一圈360度扫描的开始；
//   S_bar 为 S 的取反；C 校验位永远为1；quality 与激光接收信号质量相关；
//   angle_q6 实际角度=angle_q6/64.0 Deg；distance_q2 实际距离=distance_q2/4.0 mm。
// -------------------------------------------------------------------

// 起始标志位掩码：bit0。当为1时表示新的一圈360度扫描的开始。
// 【通信协议 LR001 P.15】S: 扫描起始标志位。S=1表示新的一圈360度扫描的开始。
#define SL_LIDAR_RESP_MEASUREMENT_SYNCBIT        (0x1<<0)
// 信号质量在 sync_quality 字节中的位移：quality 占 bit2~bit7（6 bits）。
#define SL_LIDAR_RESP_MEASUREMENT_QUALITY_SHIFT  2

// HQ 测量数据同步标志位掩码：bit0。
#define SL_LIDAR_RESP_HQ_FLAG_SYNCBIT               (0x1<<0)

// angle_q6_checkbit 字段中的校验位掩码：bit0。
// 对应协议中 C 校验位，永远为1，可用于数据应答报文起始字节的判断和数据校验。
#define SL_LIDAR_RESP_MEASUREMENT_CHECKBIT       (0x1<<0)
// angle_q6 在 angle_q6_checkbit 字段中的位移：bit1~bit15（15 bits）。
#define SL_LIDAR_RESP_MEASUREMENT_ANGLE_SHIFT    1

// GET_SAMPLERATE 应答结构（4 字节）。
// 对应通信协议 PDF 第32页图表4-30 激光测距用时获取请求对应的数据应答报文。
// 【通信协议 LR001 P.32】Tstandard[15:0] + Texpress[15:0]，单位微秒(uS)。
typedef struct _sl_lidar_response_sample_rate_t
{
    // 标准扫描模式(SCAN)下，单次激光测距的耗时（微秒）。可用于旋转速度检测。
    // 【通信协议 LR001 P.33】Tstandard: 在标准扫描模式(SCAN)下，RPLIDAR测距核心
    //   进行单次激光测距的耗时，单位：微秒(uS)。
    sl_u16  std_sample_duration_us;
    // 高速采样模式(EXPRESS_SCAN)下，单次激光测距的耗时（微秒）。
    // 【通信协议 LR001 P.33】Texpress: 在高速采样模式(EXPRESS_SCAN)下，
    //   RPLIDAR测距核心进行单次激光测距的耗时，单位：微秒(uS)。
    sl_u16  express_sample_duration_us;
} __attribute__((packed)) sl_lidar_response_sample_rate_t;

// SCAN/FORCE_SCAN 命令的标准测距点应答结构（5 字节）。
// 对应通信协议 PDF 第14-15页图表4-4 数据应答报文格式。
// 【通信协议 LR001 P.14】字节偏移: +0 [Quality|S_bar|S]；+1 angle_q6[6:0]；
//   +2 angle_q6[14:7]；+3 distance_q2[7:0]；+4 distance_q2[15:8]。
typedef struct _sl_lidar_response_measurement_node_t
{
    // 起始字节：bit0=syncbit(S)；bit1=syncbit_inverse(S_bar)；bit2~7=quality。
    // S=1 表示新的一圈扫描开始；S_bar = !S；quality 与激光接收信号质量相关。
    // 【通信协议 LR001 P.15】S: 扫描起始标志位；S_bar: 扫描起始标志位的取反；
    //   quality: 采样点信号质量，与激光接收信号质量相关。
    sl_u8    sync_quality;      // syncbit:1;syncbit_inverse:1;quality:6;
    // 角度+校验位：bit0=checkbit(C，永远为1)；bit1~15=angle_q6(定点小数)。
    // 实际角度 = angle_q6 / 64.0 度，范围 [0,360)。
    // 【通信协议 LR001 P.15】C: 校验位，永远为1；angle_q6: 实际角度=angle_q6/64.0 Deg。
    sl_u16   angle_q6_checkbit; // check_bit:1;angle_q6:15;
    // 距离（定点小数）。实际距离 = distance_q2 / 4.0 毫米。
    // 当采集到无效点时，该字段为零。
    // 【通信协议 LR001 P.15】distance_q2: 实际距离=distance_q2/4.0 mm，
    //   当采集到无效点时该字段为零。
    sl_u16   distance_q2;
} __attribute__((packed)) sl_lidar_response_measurement_node_t;

// -------------------------------------------------------------------
// EXPRESS_SCAN 传统版本 cabin 的 [distance_sync flags] 位域定义。
// cabin 的 distance_angle_1/distance_angle_2 为 16 位复合字段：
//   低 2 位 = 角度补偿量 dθ 的高 2 位；
//   高 14 位 = 距离值 distance 的低 14 位。
// 对应通信协议 PDF 第19-21页图表4-11~4-13 cabin 数据结构各字段定义。
// 【通信协议 LR001 P.21】distance1/distance2: 单位毫米，为0表示无效；
//   dθ1/dθ2: 夹角补偿量，采用 q3 有符号定点小数，最高位为符号位。
// -------------------------------------------------------------------
//[distance_sync flags]
// cabin 的低2位存放角度补偿量 dθ。
#define SL_LIDAR_RESP_MEASUREMENT_EXP_ANGLE_MASK           (0x3)
// cabin 的高14位存放距离值 distance。
#define SL_LIDAR_RESP_MEASUREMENT_EXP_DISTANCE_MASK        (0xFC)

// EXPRESS_SCAN 传统版本 cabin 结构（5 字节）。
// 对应通信协议 PDF 第19页图表4-11 中的 Cabin 子结构。
// 每个 cabin 包含 2 组测距采样数据的距离和角度补偿信息。
// 【通信协议 LR001 P.19】5字节的测距数据结构体，包含2组测距数据的距离和角度信息。
//   一个数据应答报文中包含了16组cabin数据。
typedef struct _sl_lidar_response_cabin_nodes_t
{
    // 第1组测距采样数据（复合字段，见上方 [distance_sync flags] 掩码）：
    //   高14位=distance1；低2位=dθ1 的高2位。distance 单位毫米，为0表示无效。
    // 【通信协议 LR001 P.21】distance1: 第1组测距采样的距离数据，单位毫米。
    sl_u16   distance_angle_1; // see [distance_sync flags]
    // 第2组测距采样数据（同上）：高14位=distance2；低2位=dθ2 的高2位。
    // 第一组数据的采集时间先于第二组。
    // 【通信协议 LR001 P.21】distance2: 第2组测距采样的距离数据，单位毫米。
    sl_u16   distance_angle_2; // see [distance_sync flags]
    // 角度补偿量 dθ1、dθ2 的低 4 位（各 4 bits，共 8 bits）。
    // dθ 采用 q3 有符号定点小数表示，最高位为符号位。
    // 需与 start_angle_q6 运算得到每个采样点的实际夹角。
    // 【通信协议 LR001 P.21】dθ1/dθ2: 夹角补偿量，采用q3有符号定点小数，最高位为符号位。
    sl_u8    offset_angles_q3;
} __attribute__((packed)) sl_lidar_response_cabin_nodes_t;


// EXPRESS_SCAN 报文的同步字节（sync1/sync2 字段取值）。
// sync1 永远为 0xA，sync2 永远为 0x5，用于外部系统判断一个新的应答报文的开始。
// 【通信协议 LR001 P.20】sync1: 数据包起始同步标志1，永远为0xA；
//   sync2: 数据包起始同步标志2，永远为0x5。
#define SL_LIDAR_RESP_MEASUREMENT_EXP_SYNC_1               0xA
#define SL_LIDAR_RESP_MEASUREMENT_EXP_SYNC_2               0x5

// HQ 报文的同步字节（sync_byte 字段取值），固定为 0xA5。
#define SL_LIDAR_RESP_MEASUREMENT_HQ_SYNC                  0xA5

// EXPRESS_SCAN 报文 start_angle_sync_q6 字段中的起始标志位 S 掩码（bit15）。
// 当 S=1 时表示当前应答报文是本轮测距采样中的第一个。
// 【通信协议 LR001 P.20】S: 起始应答报文标志。当设置为1时，表示当前应答报文
//   是本轮测距采样中的第一个。
#define SL_LIDAR_RESP_MEASUREMENT_EXP_SYNCBIT              (0x1<<15)

// EXPRESS_SCAN 传统版本(capsuled)数据应答报文结构（84 字节）。
// 对应通信协议 PDF 第19页图表4-11 高速扫描测距输出的数据应答报文格式。
// 布局：sync1(4b)|ChkSum[3:0](4b) + sync2(4b)|ChkSum[7:4](4b)
//       + start_angle_sync_q6(16b) + 16×cabin(5B) = 2+2+80 = 84 字节。
// 【通信协议 LR001 P.19】一条高速扫描测距的应答报文将带有32个测距采样数据点
//   （16个cabin×每cabin 2点）。
typedef struct _sl_lidar_response_capsule_measurement_nodes_t
{
    // 同步标志1与校验和低4位。高4位=sync1(0xA)，低4位=ChkSum[3:0]。
    // 【通信协议 LR001 P.20】sync1: 数据包起始同步标志1，永远为0xA。
    sl_u8                             s_checksum_1; // see [s_checksum_1]
    // 同步标志2与校验和高4位。高4位=sync2(0x5)，低4位=ChkSum[7:4]。
    // ChkSum 为对整个应答报文数据按字节依次异或得到的校验和（不含2个sync字段）。
    // 【通信协议 LR001 P.20】sync2: 数据包起始同步标志2，永远为0x5；
    //   ChkSum: 对报文数据进行按字节依次异或运算得到。
    sl_u8                             s_checksum_2; // see [s_checksum_1]
    // 起始角度与同步标志。bit15=S(起始标志)；bit0~14=start_angle_q6。
    // 实际角度 = start_angle_q6 / 64.0 度。
    // 【通信协议 LR001 P.20】start_angle_q6: 当前应答报文中各测距数据角度的基准值，
    //   实际角度=start_angle_q6/64.0 Deg。
    sl_u16                            start_angle_sync_q6;
    // 16 组 cabin 数据，每组 5 字节，共 80 字节。每 cabin 含 2 个测距点。
    // 【通信协议 LR001 P.19】一个数据应答报文中包含了16组cabin数据。
    sl_lidar_response_cabin_nodes_t  cabins[16];
} __attribute__((packed)) sl_lidar_response_capsule_measurement_nodes_t;

// EXPRESS_SCAN 密实版本(dense)的 cabin 结构（2 字节）。
// 对应通信协议 PDF 第24-26页图表4-19~4-21 密实版本。
// 每个 dense cabin 仅包含 1 组测距采样数据的距离（无角度补偿，角度由公式计算）。
// 【通信协议 LR001 P.25】cabin: 2字节的测距数据结构体，包含1组测距数据的距离和角度信息。
typedef struct _sl_lidar_response_dense_cabin_nodes_t
{
    // 测距采样的距离数据（单位毫米）。为0表示无效。
    // 【通信协议 LR001 P.25】distance: 测距采样的距离数据，单位毫米，
    //   当数值为0时表示对应的测距采样点无效。
    sl_u16   distance;
} __attribute__((packed)) sl_lidar_response_dense_cabin_nodes_t;

// EXPRESS_SCAN 密实版本(dense capsuled)数据应答报文结构（84 字节）。
// 对应通信协议 PDF 第24页图表4-19 高速扫描测距输出的数据应答报文格式(密实版本)。
// 布局：2(sync+chksum) + 2(start_angle_sync_q6) + 40×dense_cabin(2B) = 84 字节。
// 【通信协议 LR001 P.25】一条高速扫描测距的应答报文将带有40个测距采样数据点
//   （40个cabin×每cabin 1点）。
typedef struct _sl_lidar_response_dense_capsule_measurement_nodes_t
{
    // 同步标志1与校验和低4位。高4位=sync1(0xA)，低4位=ChkSum[3:0]。
    sl_u8                             s_checksum_1; // see [s_checksum_1]
    // 同步标志2与校验和高4位。高4位=sync2(0x5)，低4位=ChkSum[7:4]。
    sl_u8                             s_checksum_2; // see [s_checksum_1]
    // 起始角度与同步标志。bit15=S；bit0~14=start_angle_q6。
    sl_u16                            start_angle_sync_q6;
    // 40 组 dense cabin 数据，每组 2 字节，共 80 字节。每 cabin 含 1 个测距点。
    // 【通信协议 LR001 P.25】一个数据应答报文中包含了40组cabin数据。
    sl_lidar_response_dense_cabin_nodes_t  cabins[40];
} __attribute__((packed)) sl_lidar_response_dense_capsule_measurement_nodes_t;


// 超密实版本(ultra dense)的 cabin 结构（SDK 扩展，非协议标准）。
// 包含 2 个距离尺度字段与 1 个质量高位字段。
typedef struct _sl_lidar_response_ultra_dense_cabin_nodes_t {
    // 距离与质量低位的尺度化数据（2组）。
    sl_u16  qualityl_distance_scale[2];
    // 质量高位数组。
    sl_u8   qualityh_array;
} __attribute__((packed)) sl_lidar_response_ultra_dense_cabin_nodes_t;

// 超密实版本(ultra dense capsuled)数据应答报文结构（SDK 扩展）。
// 布局：2(sync+chksum) + 4(time_stamp) + 2(dev_status) + 2(start_angle_sync_q6)
//       + 32×ultra_dense_cabin = 报文。
typedef struct _sl_lidar_response_ultra_dense_capsule_measurement_nodes_t {
    // 同步标志1与校验和低4位。
    sl_u8                             s_checksum_1; // see [s_checksum_1]
    // 同步标志2与校验和高4位。
    sl_u8                             s_checksum_2; // see [s_checksum_1]
    // 时间戳。
    sl_u32                            time_stamp;
    // 设备状态。
    sl_u16                            dev_status;
    // 起始角度与同步标志。
    sl_u16                            start_angle_sync_q6;
    // 32 组超密实 cabin 数据。
    sl_lidar_response_ultra_dense_cabin_nodes_t  cabins[32];
} __attribute__((packed)) sl_lidar_response_ultra_dense_capsule_measurement_nodes_t;


// ext1 : x2 boost mode  扩展版本1（x2 boost 模式）
// 对应通信协议 PDF 第22-23页图表4-15~4-17 扩展版本 ultra cabin 结构。
// ultra cabin 采用 SLAMTEC 专利的可变比例编码(CN 108306649 A)压缩数据。

// major 字段位数：12 bits（采用可变比例编码的主测量数据值）。
// 【通信协议 LR001 P.23】major: 12bit 的采用可变比例编码的扫描测距数据读数。
#define SL_LIDAR_RESP_MEASUREMENT_EXP_ULTRA_MAJOR_BITS     12
// predict1/predict2 字段位数：10 bits（采用预测编码的测量数据值）。
// 【通信协议 LR001 P.23】predict1/predict2: 10bit 的预测编码的扫描测距读数。
#define SL_LIDAR_RESP_MEASUREMENT_EXP_ULTRA_PREDICT_BITS   10

// EXPRESS_SCAN 扩展版本(ultra)的 cabin 结构（4 字节）。
// 对应通信协议 PDF 第22页图表4-15 中的 Ultra Cabin 子结构。
// 32 位复合字段：低12位=major；中10位=predict1；高10位=predict2。
// 【通信协议 LR001 P.22】4字节的测距数据结构体，包含3组测距数据的距离和角度信息。
//   一个数据应答报文中包含了32组ultra cabin数据（共96个测距采样数据点）。
typedef struct _sl_lidar_response_ultra_cabin_nodes_t
{
    // 31                                              0
    // | predict2 10bit | predict1 10bit | major 12bit |
    // 复合字段：低12位=major(可变比例编码主测量值)；
    // 中10位=predict1(预测编码值)；高10位=predict2(预测编码值)。
    // 【通信协议 LR001 P.23】major: 采用可变比例编码的主测量数据值；
    //   predict1/predict2: 采用预测编码的测量数据值。
    // 【通信协议 LR001 P.23】Ultra cabin 采用 SLAMTEC 专利的压缩编码技术
    //   (CN 108306649 A)对测量数据进行了编码。
    sl_u32 combined_x3;
} __attribute__((packed)) sl_lidar_response_ultra_cabin_nodes_t;

// EXPRESS_SCAN 扩展版本(ultra capsuled)数据应答报文结构（132 字节）。
// 对应通信协议 PDF 第22页图表4-15 高速扫描测距输出的数据应答报文格式(扩展版本)。
// 布局：2(sync+chksum) + 2(start_angle_sync_q6) + 32×ultra_cabin(4B) = 132 字节。
// 【通信协议 LR001 P.22】一条高速扫描测距的应答报文将带有96个测距采样数据点
//   （32个ultra cabin×每cabin 3点）。
typedef struct _sl_lidar_response_ultra_capsule_measurement_nodes_t
{
    // 同步标志1与校验和低4位。高4位=sync1(0xA)，低4位=ChkSum[3:0]。
    sl_u8                             s_checksum_1; // see [s_checksum_1]
    // 同步标志2与校验和高4位。高4位=sync2(0x5)，低4位=ChkSum[7:4]。
    sl_u8                             s_checksum_2; // see [s_checksum_1]
    // 起始角度与同步标志。bit15=S；bit0~14=start_angle_q6。
    // 【通信协议 LR001 P.22】start_angle_q6: 当前应答报文中各测距数据角度的基准值。
    sl_u16                            start_angle_sync_q6;
    // 32 组 ultra cabin 数据，每组 4 字节，共 128 字节。每 cabin 含 3 个测距点。
    // 【通信协议 LR001 P.22】一个数据应答报文中包含了32组ultra cabin数据。
    sl_lidar_response_ultra_cabin_nodes_t  ultra_cabins[32];
} __attribute__((packed)) sl_lidar_response_ultra_capsule_measurement_nodes_t;

// HQ 扫描模式单个测距点结构（SDK 扩展，HQ 模式专用）。
// HQ 模式的应答报文使用 CRC32 校验（见 sl_crc.cpp 与 sl_crc.h）。
typedef struct sl_lidar_response_measurement_node_hq_t
{
    // 角度定点小数（q14 格式，带符号）。
    sl_u16   angle_z_q14;
    // 距离（毫米，q2 定点小数）。
    sl_u32   dist_mm_q2;
    // 信号质量。
    sl_u8    quality;
    // 标志位（bit0=syncbit 起始标志）。
    sl_u8    flag;
} __attribute__((packed)) sl_lidar_response_measurement_node_hq_t;

// HQ 扫描模式数据应答报文结构（SDK 扩展，HQ 模式专用）。
// 布局：1(sync_byte=0xA5) + 8(time_stamp) + 96×node_hq + 4(crc32)。
// 该报文使用 CRC32 校验（多项式 0x4C11DB7，IEEE 802.3 标准），见 sl_crc.cpp。
typedef struct _sl_lidar_response_hq_capsule_measurement_nodes_t
{
    // 同步字节，固定为 SL_LIDAR_RESP_MEASUREMENT_HQ_SYNC(0xA5)。
    sl_u8 sync_byte;
    // 时间戳。
    sl_u64 time_stamp;
    // 96 个 HQ 测距点数据。
    sl_lidar_response_measurement_node_hq_t node_hq[96];
    // CRC32 校验值（IEEE 802.3 标准多项式 0x4C11DB7）。
    // HQ 模式(ANS_TYPE_MEASUREMENT_HQ=0x83)的应答报文使用 CRC32 校验。
    sl_u32  crc32;
}__attribute__((packed)) sl_lidar_response_hq_capsule_measurement_nodes_t;


// -------------------------------------------------------------------
// GET_LIDAR_CONF 命令的扫描工作模式相关定义
// 以下为扫描工作模式编号，对应通信协议 PDF 第11页图表3-3 几个典型的扫描
// 工作模式特性。具体可用模式可通过 GET_LIDAR_CONF 获取。
// 【通信协议 LR001 P.12】可以使用命令 GET_LIDAR_CONF 来获取当前设备所支持
//   的所有模式。
// -------------------------------------------------------------------
// 标准模式(Standard)：传统模式。
#   define SL_LIDAR_CONF_SCAN_COMMAND_STD            0
// 高速传统模式(Express)。
#   define SL_LIDAR_CONF_SCAN_COMMAND_EXPRESS        1
// HQ 模式。
#   define SL_LIDAR_CONF_SCAN_COMMAND_HQ             2
// 性能优先模式(Boost)：优先提升采样率。
#   define SL_LIDAR_CONF_SCAN_COMMAND_BOOST          3
// 稳定性优先模式(Stability)。
#   define SL_LIDAR_CONF_SCAN_COMMAND_STABILITY      4
// 灵敏度优先模式(Sensitivity)。
#   define SL_LIDAR_CONF_SCAN_COMMAND_SENSITIVITY    5

// -------------------------------------------------------------------
// GET_LIDAR_CONF 命令的配置字段类型(type)定义
// 对应通信协议 PDF 第36页图表4-36 所支持的设备配置信息类型字段数值和含义。
// 请求报文负载为 type(4B) + 可选 payload，应答为 type(4B) + payload(可变长)。
// 【通信协议 LR001 P.34】数据请求报文格式: +0 type(32bits) +4 payload[0]...可选。
// 【通信协议 LR001 P.35】数据应答报文格式: +0 type(32bits) +4 payload[0]...。
// -------------------------------------------------------------------
// 以下为 SDK 扩展的配置字段（非协议图表4-36所列标准字段）
// 角度范围。
#define SL_LIDAR_CONF_ANGLE_RANGE                    0x00000000
// 期望旋转频率。
#define SL_LIDAR_CONF_DESIRED_ROT_FREQ               0x00000001
// 扫描命令位图。
#define SL_LIDAR_CONF_SCAN_COMMAND_BITMAP            0x00000002
// 最小旋转频率。
#define SL_LIDAR_CONF_MIN_ROT_FREQ                   0x00000004
// 最大旋转频率。
#define SL_LIDAR_CONF_MAX_ROT_FREQ                   0x00000005
// 最大测距距离。
#define SL_LIDAR_CONF_MAX_DISTANCE                   0x00000060

// 以下为协议图表4-36 定义的标准配置字段类型
// 【通信协议 LR001 P.36】RPLIDAR_CONF_SCAN_MODE_COUNT (0x70):
//   获取设备所支持的扫描工作模式ID最大值。请求数据: 无；应答数据: u16。
#define SL_LIDAR_CONF_SCAN_MODE_COUNT                0x00000070
// 【通信协议 LR001 P.36】RPLIDAR_CONF_SCAN_MODE_US_PER_SAMPLE (0x71):
//   获取给定扫描工作模式下的采样频率。请求数据: u16(模式ID)；应答数据: u32(微秒)。
#define SL_LIDAR_CONF_SCAN_MODE_US_PER_SAMPLE        0x00000071
// 【通信协议 LR001 P.36】RPLIDAR_CONF_SCAN_MODE_MAX_DISTANCE (0x74):
//   获取给定扫描工作模式下的最大测距半径。请求数据: u16；应答数据: u32(米,q8定点)。
#define SL_LIDAR_CONF_SCAN_MODE_MAX_DISTANCE         0x00000074
// 【通信协议 LR001 P.36】RPLIDAR_CONF_SCAN_MODE_ANS_TYPE (0x75):
//   获取给定扫描工作模式下EXPRESS_SCAN命令请求采用的协议版本。
//   请求数据: u16；应答数据: u8(等同于起始应答报文的数据类型字段)。
//   典型值: 0x81=SCAN标准格式；0x82=传统版本；0x83=扩展版本。
//   【通信协议 LR001 P.37】0x81~0x83 的数据类型定义。
#define SL_LIDAR_CONF_SCAN_MODE_ANS_TYPE             0x00000075
// 雷达 MAC 地址（SDK 扩展字段）。
#define SL_LIDAR_CONF_LIDAR_MAC_ADDR                 0x00000079
// 【通信协议 LR001 P.36】RPLIDAR_CONF_SCAN_MODE_TYPICAL (0x7C):
//   获取当前设备推荐的扫描工作模式ID。请求数据: 无；应答数据: u16。
//   【通信协议 LR001 P.38】建议采用该字段返回的工作模式 ID 驱动雷达。
#define SL_LIDAR_CONF_SCAN_MODE_TYPICAL              0x0000007C
// 【通信协议 LR001 P.36】RPLIDAR_CONF_SCAN_MODE_NAME (0x7F):
//   获取给定扫描工作模式所对应的可供阅读的名称。请求数据: u16；应答数据: String。
#define SL_LIDAR_CONF_SCAN_MODE_NAME                 0x0000007F


// 型号修订 ID（SDK 扩展字段）。
#define SL_LIDAR_CONF_MODEL_REVISION_ID              0x00000080
// 型号别名（SDK 扩展字段）。
#define SL_LIDAR_CONF_MODEL_NAME_ALIAS               0x00000081

// 检测到的串口波特率（SDK 扩展字段）。
#define SL_LIDAR_CONF_DETECTED_SERIAL_BPS            0x000000A1

// 雷达静态 IP 地址（SDK 扩展字段）。
#define SL_LIDAR_CONF_LIDAR_STATIC_IP_ADDR           0x0001CCC0
// 稳定性优先模式在位图中的位（对应图表3-3 Stability 模式）。
#define SL_LIDAR_EXPRESS_SCAN_STABILITY_BITMAP                 4
// 灵敏度优先模式在位图中的位（对应图表3-3 Sensitivity 模式）。
#define SL_LIDAR_EXPRESS_SCAN_SENSITIVITY_BITMAP               5

// GET_LIDAR_CONF 应答结构（可变长）。
// 对应通信协议 PDF 第35页图表4-33 数据应答报文格式: type(4B) + payload[n]。
// 【通信协议 LR001 P.35】type: 返回的设备配置数据的对应配置字段值；
//   Payload[n]: 根据特定的配置字段以及请求数据所返回的配置数据。
typedef struct _sl_lidar_response_get_lidar_conf
{
    // 配置字段类型，与对应请求字段的 type 相同。
    // 【通信协议 LR001 P.35】type: 与对应的请求字段的对应数据相同。
    sl_u32 type;
    // 配置数据负载（柔性数组，长度由起始应答报文长度字段决定）。
    // 【通信协议 LR001 P.35】Payload[n]: 具体数据格式和含义由对应的配置字段决定。
    sl_u8  payload[0];
}__attribute__((packed)) sl_lidar_response_get_lidar_conf_t;

// SET_LIDAR_CONF 应答结构。type 为配置字段类型，result 为设置结果。
// 非协议标准结构，为 SDK 扩展命令专用。
typedef struct _sl_lidar_response_set_lidar_conf
{
    // 配置字段类型。
    sl_u32 type;
    // 设置操作结果。
    sl_u32 result;
}__attribute__((packed)) sl_lidar_response_set_lidar_conf_t;


// GET_INFO 应答结构（20 字节）。
// 对应通信协议 PDF 第29-30页图表4-25~4-26 设备信息获取对应的数据应答报文。
// 【通信协议 LR001 P.29】字节偏移: +0 [MajorModel|SubModel]；+1 firmware_minor；
//   +2 firmware_major；+3 hardware；+4~+19 serialnumber[16]。
typedef struct _sl_lidar_response_device_info_t
{
    // RPLIDAR 主型号(MajorModel，低6位)与子型号(SubModel，高2位)的复合字段。
    // 【通信协议 LR001 P.30】MajorModel: RPLIDAR主型号，例如 A2M4 对应 2；
    //   SubModel: RPLIDAR子型号，例如 A2M4 对应 4。
    sl_u8   model;
    // 固件版本号。高字节=firmware_major(主版本号)；低字节=firmware_minor(次版本号)。
    // 【通信协议 LR001 P.30】firmware_minor: 固件版本号，次版本号；
    //   firmware_major: 固件版本号，主版本号。
    sl_u16  firmware_version;
    // 硬件版本号。
    // 【通信协议 LR001 P.30】hardware: 硬件版本号。
    sl_u8   hardware_version;
    // 16 字节唯一序列号。文本表示上低字节数据在前，高字节部分在后。
    // 【通信协议 LR001 P.30】serialnumber[16]: 16字节的唯一序列号，
    //   文本表示上低字节数据在前，高字节部分在后。
    sl_u8   serialnum[16];
} __attribute__((packed)) sl_lidar_response_device_info_t;

// GET_HEALTH 应答结构（3 字节）。
// 对应通信协议 PDF 第31页图表4-28 设备健康状态请求对应的数据应答报文。
// 【通信协议 LR001 P.31】字节偏移: +0 status；+1 error_code[7:0]；+2 error_code[15:8]。
typedef struct _sl_lidar_response_device_health_t
{
    // 健康状态。0=状态良好；1=警告；2=错误(已进入保护性停机)。
    // 【通信协议 LR001 P.31】status: 0 状态良好；1 警告；2 错误。
    //   当为警告时 RPLIDAR 仍旧可能正常工作；当为错误时已进入保护性停机状态。
    sl_u8   status;
    // 具体的警告/错误代码。当出现警告或者错误状态时，具体错误代号记录在此。
    // 【通信协议 LR001 P.31】error_code: 具体的警告/错误代码。
    sl_u16  error_code;
} __attribute__((packed)) sl_lidar_response_device_health_t;

// 雷达 IP 配置结构（SDK 扩展，用于网络通讯模式）。
typedef struct _sl_lidar_ip_conf_t {
    // IP 地址。
    sl_u8 ip_addr[4];
    // 子网掩码。
    sl_u8 net_mask[4];
    // 网关。
    sl_u8 gw[4];
}__attribute__((packed)) sl_lidar_ip_conf_t;

// 雷达 MAC 地址信息应答结构。
// 对应配置字段 SL_LIDAR_CONF_LIDAR_MAC_ADDR(0x79)。
typedef struct _sl_lidar_response_device_macaddr_info_t {
    // 6 字节 MAC 地址。
    sl_u8   macaddr[6];
} __attribute__((packed)) sl_lidar_response_device_macaddr_info_t;

// 期望旋转速度应答结构（SDK 扩展）。
// 对应配置字段 SL_LIDAR_CONF_DESIRED_ROT_FREQ(0x01)。
typedef struct  _sl_lidar_response_desired_rot_speed_t{
    // 期望转速 RPM。
    sl_u16 rpm;
    // PWM 参考值。
    sl_u16 pwm_ref;
}__attribute__((packed)) sl_lidar_response_desired_rot_speed_t;

// -------------------------------------------------------------------
// 可变比例编码(VarBitScale)机制的定义
// 对应通信协议 PDF 第23页所述 SLAMTEC 专利的压缩编码技术(CN 108306649 A)。
// ultra cabin 的 major 字段为 12bit 采用可变比例编码的扫描测距数据读数。
// 该编码通过分段使用不同的比例因子来扩展 12bit 能表示的距离范围。
// 【通信协议 LR001 P.23】Ultra cabin 采用 SLAMTEC 专利的压缩编码技术
//   (CN 108306649 A)对测量数据进行了编码。请参考 SDK 代码了解此编码协议
//   的设计规则。
// -------------------------------------------------------------------
// 各比例档位的源数据有效位起点（bit 位置）。
// x2 档：源数据从第 9 位开始使用 ×2 比例。
#define SL_LIDAR_VARBITSCALE_X2_SRC_BIT  9
// x4 档：源数据从第 11 位开始使用 ×4 比例。
#define SL_LIDAR_VARBITSCALE_X4_SRC_BIT  11
// x8 档：源数据从第 12 位开始使用 ×8 比例。
#define SL_LIDAR_VARBITSCALE_X8_SRC_BIT  12
// x16 档：源数据从第 14 位开始使用 ×16 比例。
#define SL_LIDAR_VARBITSCALE_X16_SRC_BIT 14

// 各比例档位的目标基准值（即该档起始处对应的解码距离值）。
// x2 档基准值。
#define SL_LIDAR_VARBITSCALE_X2_DEST_VAL 512
// x4 档基准值。
#define SL_LIDAR_VARBITSCALE_X4_DEST_VAL 1280
// x8 档基准值。
#define SL_LIDAR_VARBITSCALE_X8_DEST_VAL 1792
// x16 档基准值。
#define SL_LIDAR_VARBITSCALE_X16_DEST_VAL 3328

// 宏：根据给定的源数据有效位数 _BITS_，计算可变比例编码所能表示的最大原始值。
// 该宏按档位从高到低累加各段可表示的范围，得到给定 bit 宽度下的最大可表示值。
#define SL_LIDAR_VARBITSCALE_GET_SRC_MAX_VAL_BY_BITS(_BITS_) \
    (  (((0x1<<(_BITS_)) - SL_LIDAR_VARBITSCALE_X16_DEST_VAL)<<4) + \
       ((SL_LIDAR_VARBITSCALE_X16_DEST_VAL - SL_LIDAR_VARBITSCALE_X8_DEST_VAL)<<3) + \
       ((SL_LIDAR_VARBITSCALE_X8_DEST_VAL - SL_LIDAR_VARBITSCALE_X4_DEST_VAL)<<2) + \
       ((SL_LIDAR_VARBITSCALE_X4_DEST_VAL - SL_LIDAR_VARBITSCALE_X2_DEST_VAL)<<1) + \
       SL_LIDAR_VARBITSCALE_X2_DEST_VAL - 1)


// 结束 1 字节对齐设置。
#if defined(_WIN32)
#pragma pack()
#endif

// 恢复此前 push 的编译器警告状态。
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
