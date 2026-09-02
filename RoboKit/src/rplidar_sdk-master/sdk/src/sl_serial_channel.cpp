/*
 * Slamtec LIDAR SDK
 *
 *  Copyright (c) 2014 - 2020 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
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
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND IN ANY THEORY OF LIABILITY,
  * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  */

// =============================================================================
// 文件说明：串口通信通道（SerialPortChannel）实现文件
// -----------------------------------------------------------------------------
// 本文件实现了 IChannel 接口的串口通信通道子类 SerialPortChannel。
// 该通道通过 TTL 电平的 UART 串口与 RPLIDAR 测距核心进行通讯。
//
// 【通信协议 LR001 P.3】"外部系统通过 TTL 电平的 UART 串口信号与 RPLIDAR 测距
//   核心进行通讯。" RPLIDAR A 系列使用 UART 串口作为主要通讯接口。
// 【数据手册】RPLIDAR A1 使用 115200bps 波特率进行串口通讯。
//
// SerialPortChannel 封装了底层串口操作（rp::hal::serial_rxtx），提供：
//   - bind/open/close：串口设备的绑定、打开与关闭
//   - waitForDataExt/waitForData：等待串口数据到达
//   - write/read：数据发送与接收（用于发送请求报文和接收应答报文）
//   - setDTR：控制 DTR 信号（RPLIDAR A1 通过 DTR 引脚控制电机启停）
//   - flush：刷新串口缓冲区
// =============================================================================

#include "sl_lidar_driver.h"
#include "hal/abs_rxtx.h"
#include "hal/socket.h"


namespace sl {

    // =========================================================================
    // SerialPortChannel —— 串口通信通道类
    // -------------------------------------------------------------------------
    // 继承自 ISerialPortChannel（IChannel 的子接口），封装 UART 串口通讯。
    // 【通信协议 LR001 P.3】RPLIDAR 通过 TTL 电平的 UART 串口信号与外部系统通讯，
    //   串口信号包括 TX（发送）、RX（接收）、GND（地）。
    // 该类作为 AsyncTransceiver 的下层通道，提供原始字节的读写能力。
    // =========================================================================
    class SerialPortChannel : public ISerialPortChannel
    {
    public:
        // -----------------------------------------------------------------------
        // 构造函数：创建串口通道并初始化底层串口对象
        //   device   - 串口设备路径（如 Linux 的 "/dev/ttyUSB0" 或 Windows 的 "COM3"）
        //   baudrate - 波特率（RPLIDAR A1 默认 115200bps）
        // 通过 rp::hal::serial_rxtx::CreateRxTx() 创建平台相关的串口实现对象
        // -----------------------------------------------------------------------
        SerialPortChannel(const std::string& device, int baudrate) :_rxtxSerial(rp::hal::serial_rxtx::CreateRxTx())
        {
            _device = device;       // 保存串口设备路径
            _baudrate = baudrate;   // 保存波特率
        }

        // -----------------------------------------------------------------------
        // 析构函数：释放底层串口对象
        // -----------------------------------------------------------------------
        ~SerialPortChannel()
        {
            if (_rxtxSerial)
                delete _rxtxSerial;  // 释放串口实现对象
        }

        // -----------------------------------------------------------------------
        // 绑定串口设备：将设备路径和波特率设置到底层串口对象
        //   device   - 串口设备路径
        //   baudrate - 波特率
        // 返回 true 表示绑定成功
        // -----------------------------------------------------------------------
        bool bind(const std::string& device, sl_s32 baudrate)
        {
            _closePending = false;  // 清除关闭挂起标志
            return _rxtxSerial->bind(device.c_str(), baudrate);  // 调用底层串口绑定
        }

        // -----------------------------------------------------------------------
        // 打开串口：先绑定设备参数，再打开串口连接
        // 返回 true 表示打开成功
        // 串口打开后即可进行 read/write 操作
        // -----------------------------------------------------------------------
        bool open()
        {
            if(!bind(_device, _baudrate))  // 先绑定设备参数
                return false;
            return _rxtxSerial->open();    // 打开串口连接
        }

        // -----------------------------------------------------------------------
        // 关闭串口：设置关闭挂起标志，取消正在进行的操作，然后关闭串口
        // _closePending 标志用于通知 waitForDataExt 等阻塞方法尽快返回
        // -----------------------------------------------------------------------
        void close()
        {
            _closePending = true;             // 标记关闭挂起，通知阻塞方法退出
            _rxtxSerial->cancelOperation();   // 取消正在进行的阻塞读写操作
            _rxtxSerial->close();             // 关闭串口
        }

        // -----------------------------------------------------------------------
        // 刷新串口缓冲区：清空输入缓冲区中的残留数据
        // 参数 0 表示仅刷新输入缓冲区（不暂停）
        // 在 openChannelAndBind 中调用，确保开始通讯前串口无残留数据
        // -----------------------------------------------------------------------
        void flush()
        {
            _rxtxSerial->flush(0);
        }

        // -----------------------------------------------------------------------
        // 等待串口数据到达（扩展版）：带超时等待，返回可读字节数提示
        //   size_hint  - 输出参数，返回当前可读字节数（用于预分配接收缓冲区）
        //   timeoutInMs - 超时时间（毫秒）
        // 返回值：
        //   RESULT_OK               - 有数据可读
        //   RESULT_OPERATION_TIMEOUT - 超时无数据
        //   RESULT_OPERATION_FAIL    - 串口错误或已关闭
        //
        // 被 AsyncTransceiver::_proc_rxThread 调用，用于持续等待 RPLIDAR 的应答数据
        // 【通信协议 LR001 P.5】扫描测距模式下 RPLIDAR 会连续发送应答数据，该方法
        //   负责检测数据到达并通知接收线程读取。
        // -----------------------------------------------------------------------
        sl_result waitForDataExt(size_t& size_hint, sl_u32 timeoutInMs)
        {
            _word_size_t result;
            size_t size_holder;
            size_hint = 0;  // 初始化提示大小为0

            // 若串口正在关闭，直接返回超时
            if (_closePending) return  RESULT_OPERATION_TIMEOUT;

            // 串口未打开，返回操作失败
            if (!_rxtxSerial->isOpened()) {
                return RESULT_OPERATION_FAIL;
            }

            // 等待至少1字节的数据到达，超时时间由参数指定
            // size_holder 返回当前缓冲区中可读的字节数
            result = _rxtxSerial->waitfordata(1, timeoutInMs, &size_holder);
            size_hint = size_holder;  // 输出可读字节数提示
            // 串口设备错误
            if (result == (_word_size_t)rp::hal::serial_rxtx::ANS_DEV_ERR)
                return RESULT_OPERATION_FAIL;
            // 等待超时（无数据到达）
            if (result == (_word_size_t)rp::hal::serial_rxtx::ANS_TIMEOUT)
                return RESULT_OPERATION_TIMEOUT;

            return RESULT_OK;  // 有数据可读
        }

        // -----------------------------------------------------------------------
        // 等待指定数量的数据到达（同步等待版）
        //   size       - 需要等待的数据字节数
        //   timeoutInMs - 超时时间（毫秒）
        //   actualReady - 输出参数，实际可读字节数
        // 返回 true 表示数据已就绪
        // -----------------------------------------------------------------------
        bool waitForData(size_t size, sl_u32 timeoutInMs, size_t* actualReady)
        {
            if (_closePending) return false;  // 串口正在关闭，直接返回失败
            return (_rxtxSerial->waitfordata(size, timeoutInMs, actualReady) == rp::hal::serial_rxtx::ANS_OK);
        }

        // -----------------------------------------------------------------------
        // 发送数据：通过串口发送字节流（用于发送请求报文）
        //   data - 待发送数据指针
        //   size - 待发送数据字节数
        // 返回实际发送的字节数
        //
        // 【通信协议 LR001 P.6】请求报文通过此方法发送至 RPLIDAR。报文格式为
        //   起始标志(0xA5)+命令字+负载长度+负载数据+校验和，需在5秒内完整发送。
        // -----------------------------------------------------------------------
        int write(const void* data, size_t size)
        {
           return _rxtxSerial->senddata((const sl_u8 * )data, size);
        }

        // -----------------------------------------------------------------------
        // 接收数据：从串口读取字节流（用于接收应答报文）
        //   buffer - 接收缓冲区指针
        //   size   - 期望读取的最大字节数
        // 返回实际读取的字节数
        //
        // 【通信协议 LR001 P.7-8】应答报文以起始应答报文(0xA5 0x5A)开头，随后是
        //   数据应答报文。此方法读取这些原始字节，交由上层解码器解析。
        // -----------------------------------------------------------------------
        int read(void* buffer, size_t size)
        {
            size_t lenRec = 0;
            lenRec = _rxtxSerial->recvdata((sl_u8 *)buffer, size);
            return (int)lenRec;
        }

        // -----------------------------------------------------------------------
        // 清除读取缓存：串口通道的空实现
        // 串口通道通过 flush 方法清空缓冲区，此方法保留为接口兼容
        // -----------------------------------------------------------------------
        void clearReadCache()
        {

        }

        // -----------------------------------------------------------------------
        // 设置 DTR（Data Terminal Ready）信号电平
        //   dtr - true 设置 DTR（拉低电平），false 清除 DTR（拉高电平）
        //
        // RPLIDAR A1 通过 DTR 引脚控制扫描电机的启停：
        //   - setDTR(true)  即 DTR 拉低 -> 电机停转
        //   - setDTR(false) 即 DTR 拉高 -> 电机运转
        // 【通信协议 LR001 P.41】RPLIDAR A1Mx 系列通过 PWM 电机驱动控制扫描电机转速，
        //   DTR 信号用于控制电机的启停。
        // -----------------------------------------------------------------------
        void setDTR(bool dtr)
        {
            dtr ? _rxtxSerial->setDTR() : _rxtxSerial->clearDTR();
        }

        // -----------------------------------------------------------------------
        // 获取通道类型：返回串口通道类型标识
        // 用于上层判断当前使用的通道类型
        // -----------------------------------------------------------------------
        int getChannelType() {
            return CHANNEL_TYPE_SERIALPORT;
        }

    private:
        rp::hal::serial_rxtx  * _rxtxSerial;  // 底层串口收发对象指针（平台相关实现）
        bool _closePending;                     // 关闭挂起标志：true表示正在关闭，阻塞方法应尽快返回
        std::string _device;                    // 串口设备路径（如 "COM3" 或 "/dev/ttyUSB0"）
        int _baudrate;                          // 通讯波特率（RPLIDAR A1 默认 115200bps）

    };

    // -----------------------------------------------------------------------
    // 工厂函数：创建串口通信通道对象
    //   device   - 串口设备路径
    //   baudrate - 波特率
    // 返回 IChannel 指针（实际类型为 SerialPortChannel）
    // 上层通过此函数创建串口通道，再传给 AsyncTransceiver::openChannelAndBind 使用
    // -----------------------------------------------------------------------
    Result<IChannel*> createSerialPortChannel(const std::string& device, int baudrate)
    {
        return new  SerialPortChannel(device, baudrate);
    }

}
