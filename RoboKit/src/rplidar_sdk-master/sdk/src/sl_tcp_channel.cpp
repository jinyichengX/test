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
// 文件说明：TCP 网络通信通道（TcpChannel）实现文件
// -----------------------------------------------------------------------------
// 本文件实现了 IChannel 接口的 TCP 网络通信通道子类 TcpChannel。
// 部分 RPLIDAR 型号（如 S 系列）支持以太网接口，通过 TCP 协议进行通讯。
// 该类使用 rp::net::StreamSocket（流式套接字）实现 TCP 连接的建立、数据收发。
//
// 与串口通道(SerialPortChannel)类似，TcpChannel 同样作为 AsyncTransceiver 的下层
// 通道，提供原始字节的读写能力。上层的协议编解码逻辑完全一致，实现了通讯接口
// 与协议解析的解耦。
//
// 【通信协议 LR001 P.4-5】无论底层是串口还是 TCP，RPLIDAR 通讯协议的三种
//   请求/应答模式（单次请求-单次应答/单次请求-多次应答/单次请求-无应答）保持不变。
// =============================================================================

#include "sl_lidar_driver.h"
#include "hal/abs_rxtx.h"
#include "hal/socket.h"


namespace sl {

    // =========================================================================
    // TcpChannel —— TCP 网络通信通道类
    // -------------------------------------------------------------------------
    // 继承自 IChannel，封装 TCP 网络通讯。
    // 使用 rp::net::StreamSocket 实现面向连接的可靠 TCP 数据传输。
    // 适用于支持以太网接口的 RPLIDAR 型号（如 S 系列）。
    // =========================================================================
    class TcpChannel : public IChannel
    {
    public:
        // -----------------------------------------------------------------------
        // 构造函数：创建 TCP 通道并初始化流式套接字
        //   ip   - RPLIDAR 设备的 IP 地址（如 "192.168.1.100"）
        //   port - RPLIDAR 设备的 TCP 端口号
        // 通过 rp::net::StreamSocket::CreateSocket() 创建平台相关的 TCP 套接字
        // -----------------------------------------------------------------------
        TcpChannel(const std::string& ip, int port) : _binded_socket(rp::net::StreamSocket::CreateSocket()) {
            _ip = ip;       // 保存目标 IP 地址
            _port = port;   // 保存目标端口号
        }

        // -----------------------------------------------------------------------
        // 绑定目标地址：构造 SocketAddress 对象，记录 RPLIDAR 设备的网络地址
        //   ip   - 目标 IP 地址
        //   port - 目标端口号
        // 返回 true（地址构造始终成功，实际连接在 open() 中建立）
        // -----------------------------------------------------------------------
        bool bind(const std::string & ip, sl_s32 port)
        {
            _socket = rp::net::SocketAddress(ip.c_str(), port);  // 构造目标地址对象
            return true;
        }

        // -----------------------------------------------------------------------
        // 打开 TCP 连接：先绑定地址，再发起 TCP 连接（connect）
        // 返回 true 表示连接建立成功
        // TCP 连接建立后即可进行 read/write 操作
        // -----------------------------------------------------------------------
        bool open()
        {
            if(!bind(_ip, _port))  // 先绑定目标地址
                return false;
            // 发起 TCP 连接，IS_OK 宏判断连接是否成功
            return IS_OK(_binded_socket->connect(_socket));

        }

        // -----------------------------------------------------------------------
        // 关闭 TCP 连接：释放套接字资源
        // dispose() 会关闭连接并释放底层套接字资源，之后将指针置空
        // -----------------------------------------------------------------------
        void close()
        {
            _binded_socket->dispose();  // 关闭并释放套接字
            _binded_socket = NULL;      // 置空指针防止重复释放
        }

        // -----------------------------------------------------------------------
        // 刷新缓冲区：TCP 通道的空实现
        // TCP 是流式协议，没有类似串口的硬件缓冲区需要手动清空
        // -----------------------------------------------------------------------
        void flush()
        {

        }

        // -----------------------------------------------------------------------
        // 等待数据到达（扩展版）：带超时等待 TCP 数据
        //   size_hint   - 输出参数，返回可读字节数提示（TCP 无法精确获取，使用固定值1024）
        //   timeoutInMs - 超时时间（毫秒）
        // 返回值：
        //   RESULT_OK               - 有数据可读
        //   RESULT_OPERATION_TIMEOUT - 超时无数据
        //   其他错误码               - 套接字异常
        //
        // 注意：TCP 通道无法像串口那样获取精确的可读字节数，因此 size_hint 使用
        // 固定值 1024 作为提示。实际读取量由 read() 返回。
        // 被 AsyncTransceiver::_proc_rxThread 调用。
        // -----------------------------------------------------------------------
        sl_result waitForDataExt(size_t& size_hint, sl_u32 timeoutInMs)
        {
            u_result ans;
            size_hint = 0;
            // 等待 TCP 套接字有数据可读，超时由参数指定
            ans = _binded_socket->waitforData(timeoutInMs);

            switch (ans) {
            case RESULT_OK:
                size_hint = 1024; //dummy value
                // TCP 流式套接字无法预知可读字节数，使用固定值 1024 作为提示
                // 接收线程会根据此值预分配缓冲区，实际读取量以 read() 返回为准
                break;
            }

            return ans;
        }

        // -----------------------------------------------------------------------
        // 等待指定数量的数据到达（同步等待版）
        //   size       - 需要等待的数据字节数（TCP 通道中未实际使用此参数）
        //   timeoutInMs - 超时时间（毫秒）
        //   actualReady - 输出参数，实际可读字节数（直接设为 size）
        // 返回 true 表示数据已就绪
        // -----------------------------------------------------------------------
        bool waitForData(size_t size, sl_u32 timeoutInMs, size_t* actualReady)
        {
            if (actualReady)
                *actualReady = size;  // TCP 无法精确获取，直接设为请求大小
            return (_binded_socket->waitforData(timeoutInMs) == RESULT_OK);

        }

        // -----------------------------------------------------------------------
        // 发送数据：通过 TCP 套接字发送字节流（用于发送请求报文）
        //   data - 待发送数据指针
        //   size - 待发送数据字节数
        // 返回实际发送的字节数
        //
        // 【通信协议 LR001 P.6】请求报文通过此方法发送至 RPLIDAR。报文格式与串口通道
        //   完全一致：起始标志(0xA5)+命令字+负载长度+负载数据+校验和。
        // -----------------------------------------------------------------------
        int write(const void* data, size_t size)
        {
            return _binded_socket->send(data, size);
        }

        // -----------------------------------------------------------------------
        // 接收数据：从 TCP 套接字读取字节流（用于接收应答报文）
        //   buffer - 接收缓冲区指针
        //   size   - 期望读取的最大字节数
        // 返回实际读取的字节数
        //
        // 【通信协议 LR001 P.7-8】应答报文以起始应答报文(0xA5 0x5A)开头，随后是
        //   数据应答报文。此方法读取原始字节，交由上层解码器解析。
        // -----------------------------------------------------------------------
        int read(void* buffer, size_t size)
        {
            size_t lenRec = 0;
            _binded_socket->recv(buffer, size, lenRec);  // 从 TCP 套接字接收数据
            return (int)lenRec;
        }

        // -----------------------------------------------------------------------
        // 清除读取缓存：TCP 通道的空实现
        // -----------------------------------------------------------------------
        void clearReadCache() {}

        // -----------------------------------------------------------------------
        // 设置通道状态标志：TCP 通道的空实现
        //   flag - 状态标志位（未使用）
        // -----------------------------------------------------------------------
        void setStatus(_u32 flag){}

        // -----------------------------------------------------------------------
        // 获取通道类型：返回 TCP 通道类型标识
        // 用于上层判断当前使用的通道类型
        // -----------------------------------------------------------------------
        int getChannelType() {
            return CHANNEL_TYPE_TCP;
        }
    private:
        rp::net::StreamSocket * _binded_socket;  // TCP 流式套接字指针（管理 TCP 连接）
        rp::net::SocketAddress _socket;           // 目标地址对象（RPLIDAR 设备的 IP 和端口）
        std::string _ip;                          // 目标 IP 地址字符串
        int _port;                                // 目标端口号
    };

    // -----------------------------------------------------------------------
    // 工厂函数：创建 TCP 通信通道对象
    //   ip   - RPLIDAR 设备的 IP 地址
    //   port - RPLIDAR 设备的 TCP 端口号
    // 返回 IChannel 指针（实际类型为 TcpChannel）
    // 上层通过此函数创建 TCP 通道，再传给 AsyncTransceiver::openChannelAndBind 使用
    // -----------------------------------------------------------------------
    Result<IChannel*> createTcpChannel(const std::string& ip, int port)
    {
        return new  TcpChannel(ip, port);
    }
}
