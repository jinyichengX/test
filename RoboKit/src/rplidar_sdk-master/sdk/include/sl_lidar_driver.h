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
  *   and any other materials provided with the distribution.
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
// 本文件定义了 Slamtec LIDAR SDK 的驱动层抽象接口。
//
// 设计理念：采用"通道(Channel)"与"驱动(Driver)"分离的架构。
//   - IChannel：通信通道抽象层，屏蔽串口/TCP/UDP 等物理传输差异，
//     负责底层数据的收发与缓存管理。对应通信协议中请求报文的发送
//     和应答报文的接收。
//   - ILidarDriver：雷达驱动抽象层，定义连接/复位/扫描/取数据/电机控制
//     等高层业务操作，通过 IChannel 与雷达进行协议交互。
//
// 这种分离使得驱动逻辑与传输介质解耦：同一套 ILidarDriver 接口可
// 适配串口雷达(如 A 系列)和网络雷达(如 LPX/S2E 系列)，只需注入
// 不同类型的 IChannel 即可。
//
// 工厂函数模式：createLidarDriver() / createSerialPortChannel() 等
// 工厂函数返回 Result<T> 包装的对象指针，调用者通过 bool 转换和
// 解引用操作获取对象，无需直接管理构造/析构细节。
//
// 【SDK手册 LR002 P.12】sl_lidar_driver.h: 驱动与通道抽象接口定义。
// 【通信协议 LR001 P.4】基本通讯模式：与 RPLIDAR 的通讯采用二进制
//   数据报文进行，每个报文均具有统一的报头数据格式。
// ===================================================================

#pragma once

// SDK 驱动层接口基于 C++ 特性（类、模板、std 容器），要求 C++ 编译器。
#ifndef __cplusplus
#error "The Slamtec LIDAR SDK requires a C++ compiler to be built"
#endif

// 引入 STL 动态数组容器，用于扫描模式列表、扫描数据缓冲等场景。
#include <vector>
// 引入 STL 关联容器，用于内部映射缓存（如模式 ID 到模式信息的映射）。
#include <map>
// 引入 STL 字符串类，用于设备型号名称、串口设备路径、IP 地址等字符串参数。
#include <string>

// -------------------------------------------------------------------
// DEPRECATED 宏：跨编译器的废弃函数标注宏。
// 用于标记不再推荐使用的 API（如旧版 grabScanData），在编译时产生
// 警告，引导用户迁移到新版接口（如 grabScanDataHq）。
// GCC 使用 __attribute__((deprecated))；MSVC 使用 __declspec(deprecated)；
// 其他编译器输出编译消息提醒开发者自行实现。
// -------------------------------------------------------------------
#ifndef DEPRECATED
    #ifdef __GNUC__
        // GCC/Clang：在函数声明前添加 __attribute__((deprecated)) 标记。
        #define DEPRECATED(func) func __attribute__ ((deprecated))
    #elif defined(_MSC_VER)
        // MSVC：在函数声明前添加 __declspec(deprecated) 标记。
        #define DEPRECATED(func) __declspec(deprecated) func
    #else
        // 其他编译器：输出编译期提醒，并不实际标记废弃（降级处理）。
        #pragma message("WARNING: You need to implement DEPRECATED for this compiler")
        #define DEPRECATED(func) func
    #endif
#endif


// 引入命令/应答协议结构体与常量定义（请求命令码、应答数据类型、
// measurement_node/capsule/device_info 等协议结构体）。
// 【通信协议 LR001 P.12】请求命令总览图表 4-1 列出了 RPLIDAR 支持的请求命令。
#include "sl_lidar_cmd.h"

// 重复引入 string（可能是历史遗留，不影响编译）。
#include <string>

// 所有 SDK 接口均位于 sl 命名空间下，避免与用户代码符号冲突。
namespace sl {

// -------------------------------------------------------------------
// DEPRECATED_WARN 宏：废弃函数运行时提醒。
// 当调用方仍使用废弃函数时，首次调用打印一次迁移警告（通过静态
// __shown__ 标志保证只提示一次），后续静默执行。
// 这样既提醒了用户迁移到新接口，又不会因反复打印日志影响性能。
// -------------------------------------------------------------------
#ifdef DEPRECATED
#define DEPRECATED_WARN(fn, replacement) do { \
        static bool __shown__ = false; \
        if (!__shown__) { \
            printDeprecationWarn(fn, replacement); \
            __shown__ = true; \
        } \
    } while (0)
#endif

    // ===============================================================
    // LidarScanMode：雷达扫描模式描述结构体
    //
    // RPLIDAR 支持多种扫描工作模式（Standard/Express/Boost/HQ/
    // Sensitivity/Stability 等），每种模式有不同的采样率、最大量程
    // 和应答数据格式。用户通过 getAllSupportedScanModes() 获取设备
    // 支持的全部模式列表，通过 getTypicalScanMode() 获取推荐模式。
    //
    // 【通信协议 LR001 P.11】图表3-3 几个典型的扫描工作模式特性。
    // 【通信协议 LR001 P.12】可用 GET_LIDAR_CONF 命令获取设备支持的所有模式。
    // ===============================================================
    struct LidarScanMode
    {
        // 扫描模式编号。从 0 开始递增，用于在 startScanExpress() 中指定
        // 要使用的扫描模式。模式编号由各设备固件自行定义，不同型号雷达
        // 可能编号不同，因此需通过 getAllSupportedScanModes() 动态获取。
        sl_u16  id;

        // 单次激光测距耗时（微秒）。采样率的倒数，数值越小采样率越高。
        // 对应 GET_LIDAR_CONF 的 SL_LIDAR_CONF_SCAN_MODE_US_PER_SAMPLE(0x71) 字段。
        // 【通信协议 LR001 P.36】获取给定扫描工作模式下的采样频率(uS)。
        float   us_per_sample;

        // 当前扫描模式下的最大测距距离（米）。超过此距离的回波数据
        // 会被雷达丢弃，用户可据此判断数据有效范围。
        // 对应 GET_LIDAR_CONF 的 SL_LIDAR_CONF_SCAN_MODE_MAX_DISTANCE(0x74) 字段。
        // 【通信协议 LR001 P.36】获取给定扫描工作模式下的最大测距半径。
        float   max_distance;

        // 当前扫描模式对应的数据应答类型。该值等同于起始应答报文头中的
        // "数据类型(type)"字段，决定了 SDK 用何种解析逻辑解码扫描数据。
        // 典型取值：
        //   0x81 = SL_LIDAR_ANS_TYPE_MEASUREMENT（标准 SCAN 模式，5字节/点）
        //   0x82 = SL_LIDAR_ANS_TYPE_MEASUREMENT_CAPSULED（传统版本，84字节/包）
        //   0x83 = SL_LIDAR_ANS_TYPE_MEASUREMENT_HQ（HQ模式）
        //   0x84 = SL_LIDAR_ANS_TYPE_MEASUREMENT_CAPSULED_ULTRA（扩展版本，132字节/包）
        //   0x85 = SL_LIDAR_ANS_TYPE_MEASUREMENT_DENSE_CAPSULED（密实版本，84字节/包）
        // 【通信协议 LR001 P.8】数据类型表示数据应答报文发送内容的类型。
        // 【通信协议 LR001 P.36】0x75 获取给定模式下 EXPRESS_SCAN 命令采用的协议版本。
        sl_u8   ans_type;

        // 扫描模式名称（最长 64 字符，不足补 '\0'）。用于人类可读的模式标识，
        // 如 "Standard"、"Express"、"Boost"、"Sensitivity"、"Stability" 等。
        // 对应 GET_LIDAR_CONF 的 SL_LIDAR_CONF_SCAN_MODE_NAME(0x7F) 字段。
        // 【通信协议 LR001 P.36】获取给定扫描工作模式对应的可阅读名称。
        char    scan_mode[64];
    };

    // ===============================================================
    // Result<T>：结果包装模板类
    //
    // 设计动机：SDK 的工厂函数（如 createLidarDriver、createSerialPortChannel）
    // 既要返回创建的对象指针(T)，又要返回操作结果码(sl_result)以表示
    // 是否成功及失败原因。Result<T> 将两者包装在一起，提供类型安全的
    // 链式调用。
    //
    // 使用模式：
    //   Result<ILidarDriver*> lidar = createLidarDriver();
    //   if (!lidar) { /* 失败处理，lidar.err 为错误码 */ }
    //   (*lidar)->connect(channel);  // 成功时解引用获取对象指针
    //
    // 这种设计避免了输出参数方式（如 ILidarDriver** out）的笨拙语法，
    // 同时保留了错误码的完整性。
    // ===============================================================
    template <typename T>
    struct Result
    {
        // 操作结果码。成功时为 SL_RESULT_OK(0)，失败时高位(bit31)为1。
        // 见 sl_types.h 中 SL_RESULT_* 错误码定义。
        sl_result err;

        // 成功时持有的返回值（对象指针等）。失败时为默认构造的空值。
        T value;

        // 成功构造：用返回值构造，err 置为 SL_RESULT_OK。
        // 用于工厂函数成功创建对象时。
        Result(const T& value)
            : err(SL_RESULT_OK)
            , value(value)
        {
        }

        // 失败构造：用错误码构造，value 为默认值。
        // 用于工厂函数创建失败时（如通道打开失败、内存不足等）。
        Result(sl_result err)
            : err(err)
            , value()
        {
        }

        // 隐式转换为 sl_result：允许在条件判断中直接使用 Result 对象。
        // 例如 sl_result r = createLidarDriver(); 获取错误码。
        operator sl_result() const
        {
            return err;
        }

        // 隐式转换为 bool：通过 SL_IS_OK(err) 判断操作是否成功。
        // 允许 if (result) { ... } 或 if (!result) { ... } 式的简洁写法。
        // 失败标志位为0即成功，见 sl_types.h 中 SL_IS_OK 宏。
        operator bool() const
        {
            return SL_IS_OK(err);
        }

        // 解引用操作符：成功时获取返回值的引用。
        // 配合 operator-> 实现链式访问，如 (*lidar)->connect(...)。
        T& operator* ()
        {
            return value;
        }

        // 成员访问操作符：成功时获取返回值的指针，直接调用其方法。
        // 例如 auto lidar = createLidarDriver(); (*lidar)->getDeviceInfo(info);
        T* operator-> ()
        {
            return &value;
        }
    };

    // ===============================================================
    // LIDARTechnologyType：雷达测距技术类型枚举
    //
    // 不同测距技术原理决定了雷达的性能特性（量程、精度、抗光干扰等），
    // SDK 据此选择合适的数据处理策略。技术类型可通过设备型号推导。
    // ===============================================================
    enum LIDARTechnologyType {
        // 未知技术类型（无法从设备信息推断时使用）。
        LIDAR_TECHNOLOGY_UNKNOWN = 0,
        // 三角测距法：通过激光反射点在感光阵列上的位置，利用几何三角关系
        // 计算距离。成本低、近距离精度高，但量程有限（通常<16m）。
        // 典型产品：A 系列(A1/A2/A3)、C 系列。
        LIDAR_TECHNOLOGY_TRIANGULATION = 1,
        // 直接飞行时间(DTOF)测距法：直接测量激光脉冲往返时间计算距离。
        // 量程远、精度高、抗光干扰强，但成本较高。
        // 典型产品：部分 S 系列雷达。
        LIDAR_TECHNOLOGY_DTOF = 2,
        // 间接飞行时间(ETOF)测距法：通过连续光波相位差间接测量飞行时间。
        // 介于 DTOF 与三角测距之间的性能与成本折中方案。
        LIDAR_TECHNOLOGY_ETOF = 3,
        // 调频连续波(FMCW)测距法：通过线性调频光波的拍频测量距离。
        // 抗干扰能力极强，适合室外长距离应用。
        // 典型产品：部分高端型号。
        LIDAR_TECHNOLOGY_FMCW = 4,
    };

    // ===============================================================
    // LIDARMajorType：雷达产品系列枚举
    //
    // Slamtec LIDAR 产品按系列划分，不同系列在测距技术、接口类型、
    // 应用场景上有所差异。系列信息从设备型号(model)字段推导。
    // ===============================================================
    enum LIDARMajorType {
        // 未知系列（无法从设备信息推断时使用）。
        LIDAR_MAJOR_TYPE_UNKNOWN = 0,
        // A 系列：三角测距雷达，消费级产品线。型号如 A1M8、A2M8、A3M1 等。
        // model 字段低 6 位(MajorModel)为 1~3 左右对应 A 系列。
        LIDAR_MAJOR_TYPE_A_SERIES = 1,
        // S 系列：长距离测距雷达，面向机器人导航。如 S1、S2。
        LIDAR_MAJOR_TYPE_S_SERIES = 2,
        // T 系列：特定应用场景雷达。
        LIDAR_MAJOR_TYPE_T_SERIES = 3,
        // M 系列：模块化雷达。
        LIDAR_MAJOR_TYPE_M_SERIES = 4,
        // C 系列：成本优化的三角测距雷达。注意编号跳过了 5。
        // 典型产品如 C1。
        LIDAR_MAJOR_TYPE_C_SERIES = 6,
    };

    // ===============================================================
    // LIDARInterfaceType：雷达通信接口类型枚举
    //
    // 标识雷达与主机之间的物理/逻辑通信方式，影响通道对象的创建方式
    // 和数据帧的封装格式。
    // ===============================================================
    enum LIDARInterfaceType {
        // 串口(UART)接口：最常见的雷达通信方式，通过 createSerialPortChannel 创建。
        // 【通信协议 LR001 P.4】与 RPLIDAR 的通讯采用二进制数据报文进行。
        LIDAR_INTERFACE_UART = 0,
        // 以太网接口：网络雷达使用 UDP 协议通信，通过 createUdpChannel 创建。
        // 典型产品：LPX 系列、S2E 系列。
        LIDAR_INTERFACE_ETHERNET = 1,
        // USB 接口：部分雷达通过 USB 虚拟串口通信，本质仍为串口通道。
        LIDAR_INTERFACE_USB = 2,
        // CAN 总线接口：车载应用场景使用，SDK 当前版本未完整支持此通道。
        LIDAR_INTERFACE_CANBUS = 5,

        // 未知接口类型（无法确定通信方式时使用）。
        // 注意值 0xFFFF，作为哨兵值避免与正常接口类型冲突。
        LIDAR_INTERFACE_UNKNOWN = 0xFFFF,
    };

    // ===============================================================
    // SlamtecLidarTimingDesc：雷达时序描述结构体
    //
    // 描述雷达在特定配置下的时序特征参数，用于时间戳映射与数据同步。
    // 这些参数影响 grabScanDataHqWithTimeStamp() 中时间戳从设备时钟域
    // 到主机时钟域的映射精度。
    // ===============================================================
    struct SlamtecLidarTimingDesc {

        // 单次采样耗时（微秒）。与 LidarScanMode.us_per_sample 含义相同，
        // 但此处作为时序描述的独立字段，用于时间戳计算的基准步进量。
        sl_u32  sample_duration_uS;

        // 原生通信波特率。串口雷达的标称波特率（如 115200 或 256000），
        // 用于计算数据传输延迟补偿。
        sl_u32  native_baudrate;
        
        // 链路延迟（微秒）。数据从雷达发出到主机接收的网络/串口传输延迟，
        // 用于将设备时间戳对齐到主机时间域。网络雷达此值可能较大。
        sl_u32  linkage_delay_uS;

        // 原生接口类型。标识该时序描述对应的通信接口，确保延迟补偿
        // 参数与实际通信方式匹配。
        LIDARInterfaceType native_interface_type;

        // 是否支持原生（硬件）时间戳。若为 true，雷达在每个采样点附带
        // 硬件时间戳，可实现高精度时间映射；若为 false，SDK 只能通过
        // 采样间隔估算时间戳，存在偏差。
        bool    native_timestamp_support;
    };

    // ===============================================================
    // IChannel：通信通道抽象接口类
    //
    // 定义了与 LIDAR 物理通信的所有底层操作。SDK 的驱动层(ILidarDriver)
    // 不直接操作串口/网络 socket，而是通过 IChannel 间接进行数据收发。
    // 这种抽象层使得同一套驱动逻辑可适配多种通信介质。
    //
    // 生命周期：通道对象由用户通过工厂函数(createSerialPortChannel 等)
    // 创建并持有，在 connect() 时注入驱动，驱动的生命周期内需保证通道
    // 存活。
    //
    // 数据流：IChannel 负责发送请求报文(SL_LIDAR_CMD_SYNC_BYTE=0xA5 开头)
    // 和接收应答报文(0xA5 0x5A 起始应答 + 数据应答)。
    // 【通信协议 LR001 P.6】请求报文格式：0xA5 + 命令 + 负载长度 + 负载 + 校验和。
    // 【通信协议 LR001 P.8】起始应答报文：0xA5 0x5A + 长度/模式 + 类型 + 数据。
    // ===============================================================
    class IChannel
    {
    public:
        // 虚析构函数：确保通过基类指针删除派生类对象时正确调用派生类析构。
        virtual ~IChannel() {}

    public:
        // 打开通信通道。在 connect() 之前会被驱动调用以建立底层连接
        // （如打开串口设备文件、建立 TCP 连接等）。返回 true 表示成功。
        virtual bool open() = 0;

        // 关闭通信通道。释放底层资源（如关闭串口句柄、断开网络连接）。
        // 在 disconnect() 时被调用。
        virtual void close() = 0;

        // 将已写入发送缓冲区的数据全部推送到远端（flush）。
        // 对于串口，确保发送缓冲区数据被实际发出；对于网络，确保数据
        // 已通过 socket 推送。在发送请求报文后调用以保证雷达及时收到。
        virtual void flush() = 0;

        // 阻塞等待指定字节数的数据到达。
        // \param size 需要等待的字节数（对应应答报文的预期长度）。
        // \param timeoutInMs 超时时间（微秒，-1 表示无限等待）。
        //   注意：参数名为 timeoutInMs 但注释说明单位为微秒(microseconds)，
        //   实际单位由各平台实现决定，调用时应参考实现文档。
        // \param actualReady [输出] 实际已就绪的字节数，可用于部分读取场景。
        // \return true 表示数据已就绪，false 表示超时或错误。
        // 在接收应答报文时，驱动先通过此函数等待足够字节到达，
        // 再调用 read() 读取。
        virtual bool waitForData(size_t size, sl_u32 timeoutInMs = -1, size_t* actualReady = nullptr) = 0;

        // 扩展的数据等待接口，返回可用数据量的提示信息。
        // \param size_hint [输出] 当前可无需阻塞即可读取的字节数。
        // \param timeoutInMs 超时时间（默认 1000 微秒）。
        // \return SL_RESULT_OK 表示有数据可读；
        //         SL_RESULT_OPERATION_TIMEOUT 表示超时；
        //         SL_RESULT_OPERATION_FAIL 表示通道错误。
        // 与 waitForData() 相比，此接口返回更丰富的结果码，且提供
        // "有多少数据可读"的提示，适合需要灵活处理部分数据的场景。
        virtual sl_result waitForDataExt(size_t& size_hint, sl_u32 timeoutInMs = 1000) = 0;

        // 向远端（雷达）发送数据。
        // \param data 待发送数据缓冲区指针（通常为请求报文 sl_lidar_cmd_packet_t）。
        // \param size 待发送字节数。
        // \return 实际写入字节数，负值表示写入失败。
        // 驱动在 _sendCommand() 中构造请求报文后通过此接口发出。
        // 【通信协议 LR001 P.6】请求报文以 0xA5 起始，末尾附带异或校验和。
        virtual int write(const void* data, size_t size) = 0;

        // 从通道读取远端（雷达）发来的数据。
        // \param buffer 接收数据缓冲区（通常用于存放应答报文）。
        // \param size 缓冲区最大容量。
        // \return 实际读取字节数，负值表示读取失败。
        // 驱动在 _waitResponseHeader() / _waitNode() 中通过此接口
        // 读取起始应答报文头和数据应答负载。
        // 【通信协议 LR001 P.8】应答报文以 0xA5 0x5A 起始。
        virtual int read(void* buffer, size_t size) = 0;

        // 清空接收缓存。丢弃已接收但尚未被上层读取的数据。
        // 在发送新命令前调用，避免残留的旧数据干扰新应答的解析。
        // 例如 stop() 后重新 startScan() 前需要清空残留扫描数据。
        virtual void clearReadCache() = 0;

        // 获取通道类型标识。返回 ChannelType 枚举值，
        // 用于驱动内部判断当前通信介质并采取相应的处理策略
        // （如网络雷达需要处理 UDP 数据报边界，串口雷达需要处理字节流）。
        virtual int getChannelType() = 0;

    private:

    };

    // ===============================================================
    // ISerialPortChannel：串口通信通道抽象接口
    //
    // 继承自 IChannel，额外提供串口特有的 DTR(数据终端就绪)信号控制。
    // DTR 信号在部分 RPLIDAR 型号中用于控制电机启停：
    //   - DTR 高电平 → 电机启动旋转
    //   - DTR 低电平 → 电机停止
    // 这种设计使电机控制无需额外命令报文，仅需硬件信号线即可实现。
    // 典型应用于 A1 等早期型号，其电机由主板 DTR 信号直接驱动。
    // ===============================================================
    class ISerialPortChannel : public IChannel
    {
    public:
        virtual ~ISerialPortChannel() {}

    public:
        // 设置 DTR(Data Terminal Ready) 信号电平。
        // \param dtr true=高电平(电机启动)，false=低电平(电机停止)。
        // 对于电机由 DTR 信号控制的雷达型号，此函数是启停电机的关键。
        virtual void setDTR(bool dtr) = 0;
    };

    // ===============================================================
    // 工厂函数：创建串口通信通道
    //
    // \param device 串口设备路径。
    //   Windows 下如 "com3" 或 "\\.\com10"（端口号>9 时需用 \\.\ 前缀）。
    //   Unix-Like 系统下如 "/dev/ttyS1"、"/dev/ttyUSB2" 等。
    // \param baudrate 波特率。请参考雷达数据手册，常见值为 115200 或 256000。
    //   波特率必须与雷达配置匹配，否则通信将产生乱码或超时。
    // \return Result<IChannel*> 成功时持有通道指针，失败时包含错误码。
    //
    // 串口雷达(如 A/C 系列)通信使用此通道。请求/应答报文通过串口
    // 字节流传输，通道内部处理波特率匹配和帧封装。
    // 【通信协议 LR001 P.4】通讯采用二进制数据报文，小字端模式。
    // ===============================================================
    Result<IChannel*> createSerialPortChannel(const std::string& device, int baudrate);

    // ===============================================================
    // 工厂函数：创建 TCP 通信通道
    //
    // \param ip 雷达设备的 IP 地址。
    // \param port TCP 端口号。
    // \return Result<IChannel*> 成功时持有通道指针，失败时包含错误码。
    //
    // 用于支持以太网接口的雷达设备，通过 TCP 长连接进行数据收发。
    // 相比 UDP，TCP 提供可靠传输但延迟略高。
    // ===============================================================
    Result<IChannel*> createTcpChannel(const std::string& ip, int port);

    // ===============================================================
    // 工厂函数：创建 UDP 通信通道
    //
    // \param ip 雷达设备的 IP 地址。
    // \param port UDP 端口号。
    // \return Result<IChannel*> 成功时持有通道指针，失败时包含错误码。
    //
    // 用于支持以太网接口的雷达设备（如 LPX 系列、S2E 系列），
    // 通过 UDP 数据报进行数据收发。雷达的扫描数据以 UDP 报文形式
    // 持续推送，适合实时性要求高的场景。
    // ===============================================================
    Result<IChannel*> createUdpChannel(const std::string& ip, int port);

    // ===============================================================
    // MotorCtrlSupport：电机控制方式支持枚举
    //
    // 不同型号雷达的电机控制机制不同，SDK 需要据此选择正确的调速方法：
    //   - PWM 调速：通过 SET_MOTOR_PWM 命令设置占空比（转接板方案）
    //   - RPM 调速：通过 MOTOR_SPEED_CTRL 命令直接设置目标转速
    //   - 无电机控制：电机由硬件信号(DTR)控制或不可调速
    // ===============================================================
    enum MotorCtrlSupport
    {
        // 不支持电机控制。电机可能由 DTR 信号直接驱动(ISerialPortChannel.setDTR)
        // 或为外部独立电机，SDK 无法通过命令调速。
        MotorCtrlSupportNone = 0,
        // 支持 PWM 占空比调速。通过 SL_LIDAR_CMD_SET_MOTOR_PWM(0xF0) 命令
        // 设置 PWM 占空比来间接控制电机转速。需配合 accessory board(转接板)。
        MotorCtrlSupportPwm = 1,
        // 支持 RPM 直接调速。通过 SL_LIDAR_CMD_HQ_MOTOR_SPEED_CTRL(0xA8) 命令
        // 直接设置目标转速(RPM)。S 系列等较新雷达支持此方式，控速更精确。
        // 【通信协议 LR001 P.39】设备转速控制命令请求，值 0xA8，负载为 2字节 Rpm。
        MotorCtrlSupportRpm = 2,
    };

    // ===============================================================
    // ChannelType：通道类型枚举
    //
    // 标识 IChannel 的具体通信介质类型，由 getChannelType() 返回。
    // 驱动根据通道类型适配不同的数据处理逻辑（如帧边界处理、超时策略）。
    // ===============================================================
    enum ChannelType{
        // 串口通道（UART/USB虚拟串口）。
        CHANNEL_TYPE_SERIALPORT = 0x0,
        // TCP 通道（以太网）。
        CHANNEL_TYPE_TCP = 0x1,
        // UDP 通道（以太网，雷达数据推送）。
        CHANNEL_TYPE_UDP = 0x2,
    };

    // ===============================================================
    // LidarMotorInfo：雷达电机信息结构体
    //
    // 描述雷达电机的控制方式及转速范围，由 getMotorInfo() 获取。
    // 用于 setMotorSpeed() 前的参数校验和 UI 层显示转速范围。
    // ===============================================================
    struct LidarMotorInfo
    {
        // 电机控制方式支持情况。决定 setMotorSpeed() 内部采用的调速命令。
        // 见 MotorCtrlSupport 枚举定义。
        MotorCtrlSupport motorCtrlSupport;

        // 期望（推荐）转速。雷达出厂标称的最佳工作转速，
        // 采样频率和扫描频率在此转速下达到最优平衡。单位通常为 RPM。
        sl_u16 desired_speed;

        // 电机最大允许转速。超过此值可能损坏雷达或导致数据异常。
        sl_u16 max_speed;

        // 电机最小允许转速。低于此值可能导致激光触发异常或数据不完整。
        sl_u16 min_speed;
    };

    // ===============================================================
    // ILidarDriver：LIDAR 驱动抽象接口类
    //
    // 这是 SDK 的核心接口，定义了与 LIDAR 交互的全部高层操作。
    // 用户通过 createLidarDriver() 工厂函数获取实现此接口的对象指针，
    // 调用 connect() 绑定通信通道后即可使用各项功能。
    //
    // 典型工作流程：
    //   1. createLidarDriver() → 获取驱动实例
    //   2. createSerialPortChannel() → 创建通信通道
    //   3. connect(channel) → 连接雷达
    //   4. getHealth() / getDeviceInfo() → 检查设备状态与信息
    //   5. getAllSupportedScanModes() → 获取可用扫描模式
    //   6. startScan() → 开始扫描
    //   7. grabScanDataHq() → 获取完整一圈扫描数据
    //   8. ascendScanData() → 按角度排序
    //   9. stop() → 停止扫描
    //   10. disconnect() → 断开连接
    //
    // 底层通信映射：
    //   - connect → 通过通道发送 GET_HEALTH/GET_INFO 等请求验证连接
    //   - startScan → 发送 SCAN(0x20) 或 EXPRESS_SCAN(0x82) 命令
    //   - grabScanDataHq → 接收并解析 0x81~0x85 类型的连续应答数据
    //   - stop → 发送 STOP(0x25) 命令
    // 【通信协议 LR001 P.6~P.8】请求报文与应答报文格式。
    // 【SDK手册 LR002 P.13~P.16】驱动接口的使用说明。
    // ===============================================================
    class ILidarDriver
    {
    public:
        virtual ~ILidarDriver() {}

    public:
        // 连接 LIDAR。将通信通道绑定到驱动实例，并验证设备连通性。
        // \param channel 通信通道指针。调用者负责管理通道生命周期，
        //   须保证在驱动整个使用周期内通道对象有效。
        // 工作流程：保存通道引用 → 可选发送探测命令验证连通性。
        // 返回 SL_RESULT_OK 表示成功，否则为错误码。
        virtual sl_result connect(IChannel* channel) = 0;

        // 断开与 LIDAR 的连接。释放驱动内部资源（如停止扫描线程），
        // 但不关闭/销毁通道对象（通道由调用者管理）。
        virtual void disconnect() = 0;
        
        // 检查当前是否已连接到 LIDAR。
        // 返回 true 表示已通过 connect() 成功建立连接。
        virtual bool isConnected() = 0;

    public:
        // 默认操作超时值（毫秒）。大部分命令操作的默认超时时间。
        // 2000ms 适用于大多数串口/网络通信场景，兼顾响应速度和容错性。
        enum
        {
            DEFAULT_TIMEOUT = 2000
        };

    public:
        // 请求 LIDAR 测距核心软重启。
        // 对应发送 SL_LIDAR_CMD_RESET(0x40) 命令。
        // 当雷达因故障进入保护性停机(自保护模式)时，主机可通过此操作
        // 使雷达恢复到通电后的初始状态。无应答报文，建议发送后延迟 2ms 以上。
        // \param timeoutInMs 操作超时时间（毫秒），默认 DEFAULT_TIMEOUT。
        // 【通信协议 LR001 P.13】RESET 命令请求，值 0x40，无负载，无应答。
        virtual sl_result reset(sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;

        // 获取设备支持的所有扫描工作模式列表。
        // 工作流程：发送 GET_LIDAR_CONF(0x84) 命令依次查询模式数量
        // (SL_LIDAR_CONF_SCAN_MODE_COUNT=0x70)、每个模式的采样率、
        // 最大距离、应答类型和模式名称。
        // \param outModes [输出] 扫描模式列表，每个元素为 LidarScanMode 结构。
        // \param timeoutInMs 操作超时时间。
        // 【通信协议 LR001 P.12】可用 GET_LIDAR_CONF 命令获取所有模式。
        // 【通信协议 LR001 P.36】图表4-36 配置字段类型数值和含义。
        virtual sl_result getAllSupportedScanModes(std::vector<LidarScanMode>& outModes, sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;

        // 获取设备推荐的典型扫描工作模式。
        // 对应发送 GET_LIDAR_CONF 查询 SL_LIDAR_CONF_SCAN_MODE_TYPICAL(0x7C) 字段。
        // 返回的模式 ID 可直接传入 startScanExpress() 使用，能获得该设备的最佳性能。
        // \param outMode [输出] 推荐的扫描模式编号。
        // \param timeoutInMs 操作超时时间。
        // 【通信协议 LR001 P.36】0x7C 获取当前设备推荐的扫描工作模式ID。
        // 【通信协议 LR001 P.38】建议采用该字段返回的工作模式 ID 驱动雷达。
        virtual sl_result getTypicalScanMode(sl_u16& outMode, sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;

        // 开始扫描采样。
        // 根据参数选择发送 SCAN(0x20) 或 EXPRESS_SCAN(0x82) 命令，
        // 驱动内部启动后台数据接收线程持续接收应答报文。
        // \param force 是否强制扫描（对应 FORCE_SCAN=0x21）。
        //   true 时忽略电机是否旋转而强行测距，用于测试；false 时正常扫描。
        // \param useTypicalScan true=使用设备推荐模式(发送 EXPRESS_SCAN)，
        //   false=使用兼容模式(发送标准 SCAN 命令，2k 采样率，5字节/点)。
        // \param options 扫描选项（保留参数，请传 0）。
        // \param outUsedScanMode [输出] 雷达实际选用的扫描模式信息，可传 nullptr 忽略。
        // 返回 SL_RESULT_OK 表示成功开始扫描。
        // 【通信协议 LR001 P.14】SCAN 命令请求，值 0x20，多次应答，5 bytes/点。
        // 【通信协议 LR001 P.17】EXPRESS_SCAN 命令请求，值 0x82，多次应答。
        virtual sl_result startScan(bool force, bool useTypicalScan, sl_u32 options = 0, LidarScanMode* outUsedScanMode = nullptr) = 0;

        // 以指定扫描模式开始高速采样。
        // 始终发送 EXPRESS_SCAN(0x82) 命令，通过负载中的 working_mode 字段
        // 指定具体扫描模式编号。
        // \param force 是否强制扫描。
        // \param scanMode 扫描模式编号（通过 getAllSupportedScanModes 获取）。
        // \param options 扫描选项（保留参数，请传 0）。
        // \param outUsedScanMode [输出] 雷达实际选用的扫描模式信息。
        // \param timeout 操作超时时间。
        // 【通信协议 LR001 P.18】EXPRESS_SCAN 命令负载含 5 字节 working_mode 等。
        virtual sl_result startScanExpress(bool force, sl_u16 scanMode, sl_u32 options = 0, LidarScanMode* outUsedScanMode = nullptr, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        // 获取 LIDAR 健康状态。
        // 对应发送 GET_HEALTH(0x52) 命令，接收 3 字节应答(status + error_code)。
        // 主机可通过此操作检查雷达是否处于保护性停机状态。
        // \param health [输出] 健康状态信息。
        //   status: 0=良好, 1=警告(仍可工作), 2=错误(已停机)。
        // \param timeout 操作超时时间。
        // 【通信协议 LR001 P.31】GET_HEALTH 命令，值 0x52，应答 3 bytes。
        //   status: 0 状态良好；1 警告；2 错误(保护性停机)。
        virtual sl_result getHealth(sl_lidar_response_device_health_t& health, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        // 获取 LIDAR 设备信息。
        // 对应发送 GET_INFO(0x50) 命令，接收 20 字节应答。
        // \param info [输出] 设备信息，包含型号(model)、固件版本、硬件版本、
        //   16 字节序列号。
        // \param timeout 操作超时时间。
        // 【通信协议 LR001 P.29】GET_INFO 命令，值 0x50，应答 20 bytes。
        //   含 MajorModel/SubModel/firmware_version/hardware_version/serialnumber。
        virtual sl_result getDeviceInfo(sl_lidar_response_device_info_t& info, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        // 检查设备是否支持电机控制。
        // 内部发送 GET_ACC_BOARD_FLAG(0xFF) 命令查询转接板功能标志，
        // 或通过其他方式判断电机控制能力。此 API 会禁用数据抓取(grab)。
        // \param motorCtrlSupport [输出] 电机控制支持类型(None/Pwm/Rpm)。
        // \param timeout 操作超时时间。
        // 返回值用于决定后续 setMotorSpeed() 采用 PWM 还是 RPM 方式。
        virtual sl_result checkMotorCtrlSupport(MotorCtrlSupport& motorCtrlSupport, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        // 根据扫描数据计算 LIDAR 当前扫描频率(Hz)。
        // 利用一圈扫描数据中的采样点数量和该模式的单次采样耗时反推
        // 雷达转速。计算公式：频率 = 采样点数 / (采样点数 * us_per_sample) * 1e6
        // 当数据不足时计算结果不准确。
        // \param scanMode 当前扫描模式（提供 us_per_sample 单次采样耗时）。
        // \param nodes 一圈扫描数据数组。
        // \param count 采样点数量。
        // \param frequency [输出] 计算得到的扫描频率(Hz)。
        virtual sl_result getFrequency(const LidarScanMode& scanMode, const sl_lidar_response_measurement_node_hq_t* nodes, size_t count, float& frequency) = 0;

        // 设置 LPX 和 S2E 系列网络雷达的静态 IP 地址。
        // 通过 SET_LIDAR_CONF(0x85) 命令配置网络参数，使雷达在 DHCP 失败时
        // 使用此静态 IP。仅适用于以太网接口雷达。
        // \param conf 网络配置参数(IP/子网掩码/网关)，见 sl_lidar_ip_conf_t。
        // \param timeout 操作超时时间。
        virtual sl_result setLidarIpConf(const sl_lidar_ip_conf_t& conf, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;
       
        // 获取 LPX 和 S2E 系列网络雷达的静态 IP 地址。
        // 通过 GET_LIDAR_CONF(0x84) 命令查询 SL_LIDAR_CONF_LIDAR_STATIC_IP_ADDR 配置字段。
        // \param conf [输出] 网络配置参数。
        // \param timeout 操作超时时间。
        virtual sl_result getLidarIpConf( sl_lidar_ip_conf_t& conf, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;
  
        // 获取 LPX 系列网络雷达的 MAC 地址。
        // 通过 GET_LIDAR_CONF 命令查询 SL_LIDAR_CONF_LIDAR_MAC_ADDR(0x79) 配置字段。
        // \param macAddrArray [输出] MAC 地址缓冲区，必须至少 6 字节，否则缓冲区溢出。
        // \param timeoutInMs 操作超时时间。
        virtual sl_result getDeviceMacAddr(sl_u8* macAddrArray, sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;

        // 停止当前扫描操作，进入空闲状态。
        // 对应发送 STOP(0x25) 命令。驱动内部终止后台数据接收线程。
        // 发送后雷达关闭激光器并退出扫描采样模式，无应答，建议延迟 1ms。
        // \param timeout 操作超时时间。
        // 【通信协议 LR001 P.13】STOP 命令，值 0x25，无负载，无应答，
        //   离开扫描采样模式进入空闲状态。
        virtual sl_result stop(sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        // 等待并获取一个完整的 0-360 度扫描数据（高质量 HQ 格式）。
        // 从驱动内部缓存中取出后台线程已接收并组装好的一圈完整扫描数据。
        // 返回数据特性：
        //   1) 首个节点的 start_flag(syncbit)=1，表示一圈扫描起点
        //   2) 所有节点属于同一次完整的 360 度扫描
        //   3) 角度可能非升序，可用 ascendScanData() 重新排序
        // \param nodebuffer 调用方提供的缓冲区，用于存放扫描数据。
        // \param count [输入/输出] 调用前设为缓冲区最大容量，返回时为实际数据量。
        // \param timeout 最长等待时间，0 表示非阻塞立即返回。
        // 返回 SL_RESULT_OPERATION_TIMEOUT 表示在超时时间内未获得完整一圈扫描。
        // 【通信协议 LR001 P.15】S=1 表示新的一圈360度扫描的开始(syncbit)。
        virtual sl_result grabScanDataHq(sl_lidar_response_measurement_node_hq_t* nodebuffer, size_t& count, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        // 等待并获取一个完整的 0-360 度扫描数据（附带时间戳）。
        // 与 grabScanDataHq() 功能相同，额外返回扫描起始点的时间戳。
        // 时间戳为扫描数据第一个点的采集时刻，单位微秒，基于主机时间域。
        // 若雷达支持硬件时间戳，则使用设备发出的实际时间戳并映射到主机时间域；
        // 否则通过采样间隔估算，可能存在偏差。
        // \param nodebuffer 扫描数据缓冲区。
        // \param count [输入/输出] 缓冲区容量/实际数据量。
        // \param timestamp_uS [输出] 扫描起始点的时间戳（微秒）。
        // \param timeout 最长等待时间。
        // 返回 SL_RESULT_OPERATION_TIMEOUT 表示超时未获得完整扫描。
        virtual sl_result grabScanDataHqWithTimeStamp(sl_lidar_response_measurement_node_hq_t* nodebuffer, size_t& count, sl_u64 & timestamp_uS, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        // 将扫描数据按角度值升序排列。
        // grabScanDataHq() 返回的数据可能角度非升序（因电机转速波动），
        // 此函数对数据进行原地排序，方便后续按角度遍历处理。
        // \param nodebuffer 扫描数据缓冲区（原地排序）。
        // \param count 数据点数量。
        // 返回 SL_RESULT_OPERATION_FAIL 表示所有数据均无效。
        virtual sl_result ascendScanData(sl_lidar_response_measurement_node_hq_t* nodebuffer, size_t count) = 0;

        // 获取当前已接收的扫描数据点（不要求是完整一圈扫描）。
        // 与 grabScanDataHq() 不同，此函数返回截至调用时刻所有已缓存的
        // 扫描点，即使一圈扫描尚未完成。适合需要低延迟、逐帧增量处理的场景。
        // \param nodebuffer 扫描数据缓冲区。
        // \param count [输入/输出] 缓冲区容量/实际数据量。
        // 返回 SL_RESULT_OPERATION_TIMEOUT 表示自上次调用后无新数据。
        virtual sl_result getScanDataWithIntervalHq(sl_lidar_response_measurement_node_hq_t* nodebuffer, size_t& count) = 0;

        // 设置 LIDAR 电机转速。
        // 根据设备支持的电机控制方式(MotorCtrlSupport)选择不同命令：
        //   - PWM 方式：发送 SET_MOTOR_PWM(0xF0) 设置占空比
        //   - RPM 方式：发送 MOTOR_SPEED_CTRL(0xA8) 设置目标转速
        // \param speed 目标转速。传 DEFAULT_MOTOR_SPEED(0xFFFF) 表示使用默认转速。
        // 注意：当 speed 为 DEFAULT_MOTOR_SPEED 时，函数会停止扫描。
        // 【通信协议 LR001 P.39】MOTOR_SPEED_CTRL 命令，值 0xA8，负载 2字节 Rpm。
        virtual sl_result setMotorSpeed(sl_u16 speed = DEFAULT_MOTOR_SPEED) = 0;
        
        // 获取 LIDAR 电机信息。
        // 包含电机控制方式、推荐转速、最大/最小转速范围。
        // 用于 setMotorSpeed() 前的参数校验。
        // \param motorInfo [输出] 电机信息。
        // \param timeoutInMs 操作超时时间。
        virtual sl_result getMotorInfo(LidarMotorInfo &motorInfo, sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;
    

        // 请求 LIDAR 切换串口波特率。
        // 发送 NEW_BAUDRATE_CONFIRM(0x90) 命令使雷达切换到新波特率。
        // 目标雷达固件须支持此功能。此函数不验证新波特率是否可用，
        // 调用后应通过 getDeviceInfo() 等命令验证通信是否正常。
        // \param requiredBaudRate 要求使用的新波特率，须与绑定的通道波特率匹配。
        // \param baudRateDetected [输出] 雷达系统实际检测到的波特率，可为 NULL。
        virtual sl_result negotiateSerialBaudRate(sl_u32 requiredBaudRate, sl_u32* baudRateDetected = NULL) = 0;

        // 获取 LIDAR 测距技术类型（三角测距/DTOF/ETOF/FMCW）。
        // 通过设备型号(model)字段推导测距技术原理。
        // \param devInfo 设备信息指针。传 nullptr 则使用驱动缓存的已连接设备信息。
        // 返回 LIDARTechnologyType 枚举值。
        virtual LIDARTechnologyType getLIDARTechnologyType(const sl_lidar_response_device_info_t* devInfo = nullptr) = 0;
        
        // 获取 LIDAR 产品系列（A/S/T/M/C 等系列）。
        // 通过设备型号(model)字段推导所属产品系列。
        // \param devInfo 设备信息指针。传 nullptr 则使用驱动缓存的已连接设备信息。
        // 返回 LIDARMajorType 枚举值。
        virtual LIDARMajorType getLIDARMajorType(const sl_lidar_response_device_info_t* devInfo = nullptr) = 0;

        // 获取 LIDAR 型号名称字符串。
        // 结果形如 "A1M8"、"S1M1"、"A3M1-R1" 等，由型号和子型号组合推导。
        // \param out_description [输出] 型号名称字符串。
        // \param fetchAliasName 是否查询型号别名（true 时会发起一次通信查询别名）。
        // \param devInfo 设备信息指针。传 nullptr 则使用驱动缓存信息。
        // \param timeout 潜在通信的超时时间。
        // 返回 SL_RESULT_OK 表示成功。
        virtual sl_result getModelNameDescriptionString(std::string& out_description, bool fetchAliasName = true, const sl_lidar_response_device_info_t* devInfo = nullptr, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

};

    // ===============================================================
    // 工厂函数：创建 LIDAR 驱动实例
    //
    // 返回 Result<ILidarDriver*>，成功时持有驱动对象指针，
    // 调用者通过解引用获取对象，使用完毕后需 delete 释放。
    //
    // 典型用法：
    //   Result<IChannel*> channel = createSerialPortChannel("/dev/ttyUSB0", 115200);
    //   assert((bool)channel);       // 检查通道创建成功
    //   assert(*channel);             // 检查指针非空
    //
    //   auto lidar = createLidarDriver();
    //   assert((bool)lidar);          // 检查驱动创建成功
    //   assert(*lidar);               // 检查指针非空
    //
    //   auto res = (*lidar)->connect(*channel);  // 连接雷达
    //   assert(SL_IS_OK(res));
    //
    //   sl_lidar_response_device_info_t deviceInfo;
    //   res = (*lidar)->getDeviceInfo(deviceInfo);  // 获取设备信息
    //   assert(SL_IS_OK(res));
    //
    //   printf("Model: %d, Firmware Version: %d.%d, Hardware Version: %d\n",
    //          deviceInfo.model,
    //          deviceInfo.firmware_version >> 8,        // 高字节=主版本号
    //          deviceInfo.firmware_version & 0xffu,      // 低字节=次版本号
    //          deviceInfo.hardware_version);
    //
    //   delete *lidar;    // 释放驱动对象
    //   delete *channel;  // 释放通道对象
    // ===============================================================
    Result<ILidarDriver*> createLidarDriver();
}
