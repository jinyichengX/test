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
// 文件说明：UDP 网络通信通道（UdpChannel）实现文件
// -----------------------------------------------------------------------------
// 本文件实现了 IChannel 接口的 UDP 网络通信通道子类 UdpChannel。
// 部分 RPLIDAR 型号（如 LPX 系列）支持 UDP 接口，通过 UDP 协议进行通讯。
// 该类使用 rp::net::DGramSocket（数据报套接字）实现 UDP 通信。
//
// 与串口通道和 TCP 通道类似，UdpChannel 同样作为 AsyncTransceiver 的下层通道，
// 提供原始字节的读写能力。上层的协议编解码逻辑完全一致，实现了通讯接口与
// 协议解析的解耦。
//
// UDP 与 TCP 的主要区别：
//   - UDP 是无连接的数据报协议，通过 setPairAddress 预设对端地址
//   - UDP 发送使用 sendTo，接收使用 recvFrom
//   - UDP 不保证数据可靠性和顺序，但延迟更低
//
// 【通信协议 LR001 P.4-5】无论底层是串口、TCP 还是 UDP，RPLIDAR 通讯协议的
//   三种请求/应答模式保持不变。
// =============================================================================

#include "sl_lidar_driver.h"
#include "hal/abs_rxtx.h"
#include "hal/socket.h"


namespace sl {
	// =========================================================================
	// UdpChannel —— UDP 网络通信通道类
	// -------------------------------------------------------------------------
	// 继承自 IChannel，封装 UDP 网络通讯。
	// 使用 rp::net::DGramSocket 实现无连接的数据报传输。
	// 适用于支持 UDP 接口的 RPLIDAR 型号（如 LPX 系列）。
	// =========================================================================
	class UdpChannel : public IChannel
	{
	public:
		// -----------------------------------------------------------------------
		// 构造函数：创建 UDP 通道并初始化数据报套接字
		//   ip   - RPLIDAR 设备的 IP 地址（如 "192.168.1.100"）
		//   port - RPLIDAR 设备的 UDP 端口号
		// 通过 rp::net::DGramSocket::CreateSocket() 创建平台相关的 UDP 套接字
		// -----------------------------------------------------------------------
		UdpChannel(const std::string& ip, int port) : _binded_socket(rp::net::DGramSocket::CreateSocket()) {
            _ip = ip;       // 保存目标 IP 地址
            _port = port;   // 保存目标端口号
        }

		// -----------------------------------------------------------------------
		// 绑定目标地址：构造 SocketAddress 对象，记录 RPLIDAR 设备的网络地址
		//   ip   - 目标 IP 地址
		//   port - 目标端口号
		// 返回 true（地址构造始终成功）
		// -----------------------------------------------------------------------
		bool bind(const std::string & ip, sl_s32 port)
        {
            _socket = rp::net::SocketAddress(ip.c_str(), port);  // 构造目标地址对象
            return true;
        }

		// -----------------------------------------------------------------------
		// 打开 UDP 通道：先绑定地址，再设置对端地址（setPairAddress）
		// UDP 是无连接协议，不需要像 TCP 那样建立连接（connect），而是通过
		// setPairAddress 预设默认对端地址，之后 sendTo 时可传 nullptr 使用预设地址
		// 返回 true 表示设置成功
		// -----------------------------------------------------------------------
		bool open()
        {
            if(!bind(_ip, _port))  // 先绑定目标地址
                return false;
            // 设置 UDP 套接字的默认对端地址，后续 send/recv 可直接使用
            return SL_IS_OK(_binded_socket->setPairAddress(&_socket));
        }

		// -----------------------------------------------------------------------
		// 关闭 UDP 通道：释放套接字资源
		// dispose() 会关闭套接字并释放底层资源，之后将指针置空
		// -----------------------------------------------------------------------
		void close()
        {
            _binded_socket->dispose();  // 关闭并释放套接字
            _binded_socket = NULL;      // 置空指针防止重复释放
        }

		// -----------------------------------------------------------------------
		// 刷新缓冲区：清除接收缓存中的残留数据
		// UDP 通道通过 clearReadCache() 清空接收缓存
		// 在 openChannelAndBind 中调用，确保开始通讯前无残留数据
		// -----------------------------------------------------------------------
		void flush()
        {
            clearReadCache();
        }

		// -----------------------------------------------------------------------
		// 等待数据到达（扩展版）：带超时等待 UDP 数据报
		//   size_hint   - 输出参数，返回可读字节数提示（UDP 无法精确获取，使用固定值1024）
		//   timeoutInMs - 超时时间（毫秒）
		// 返回值：
		//   RESULT_OK               - 有数据可读
		//   RESULT_OPERATION_TIMEOUT - 超时无数据
		//   其他错误码               - 套接字异常
		//
		// 注意：UDP 通道无法精确获取可读字节数，因此 size_hint 使用固定值 1024 作为
		// 提示。实际读取量由 read() 返回。
		// 被 AsyncTransceiver::_proc_rxThread 调用。
		// -----------------------------------------------------------------------
		sl_result waitForDataExt(size_t& size_hint, sl_u32 timeoutInMs)
        {
            u_result ans;
            size_hint = 0;
            // 等待 UDP 套接字有数据可读，超时由参数指定
            ans = _binded_socket->waitforData(timeoutInMs);

            switch (ans) {
            case RESULT_OK:
                size_hint = 1024; //dummy value
                // UDP 数据报套接字无法预知可读字节数，使用固定值 1024 作为提示
                // 接收线程会根据此值预分配缓冲区，实际读取量以 read() 返回为准
                break;
            }

            return ans;
        }

		// -----------------------------------------------------------------------
		// 等待指定数量的数据到达（同步等待版）
		//   size       - 需要等待的数据字节数（UDP 通道中未实际使用此参数）
		//   timeoutInMs - 超时时间（毫秒）
		//   actualReady - 输出参数，实际可读字节数（直接设为 size）
		// 返回 true 表示数据已就绪
		// -----------------------------------------------------------------------
		bool waitForData(size_t size, sl_u32 timeoutInMs, size_t* actualReady)
        {
            if (actualReady)
                *actualReady = size;  // UDP 无法精确获取，直接设为请求大小
            return (_binded_socket->waitforData(timeoutInMs) == RESULT_OK);

        }

		// -----------------------------------------------------------------------
		// 发送数据：通过 UDP 套接字发送数据报（用于发送请求报文）
		//   data - 待发送数据指针
		//   size - 待发送数据字节数
		// 返回实际发送的字节数
		//
		// sendTo 的第一个参数为 nullptr，表示使用 open() 中通过 setPairAddress
		// 预设的默认对端地址发送数据。
		//
		// 【通信协议 LR001 P.6】请求报文通过此方法发送至 RPLIDAR。报文格式与串口/TCP
		//   通道完全一致：起始标志(0xA5)+命令字+负载长度+负载数据+校验和。
		// -----------------------------------------------------------------------
		int write(const void* data, size_t size)
        {
            return _binded_socket->sendTo(nullptr, data, size);
        }

		// -----------------------------------------------------------------------
		// 接收数据：从 UDP 套接字读取数据报（用于接收应答报文）
		//   buffer - 接收缓冲区指针
		//   size   - 期望读取的最大字节数
		// 返回实际读取的字节数（读取失败返回0）
		//
		// 【通信协议 LR001 P.7-8】应答报文以起始应答报文(0xA5 0x5A)开头，随后是
		//   数据应答报文。此方法读取原始字节，交由上层解码器解析。
		// -----------------------------------------------------------------------
		int read(void* buffer, size_t size)
        {
            size_t actualGet;

            // 从 UDP 套接字接收数据报，actualGet 返回实际接收的字节数
            u_result ans = _binded_socket->recvFrom(buffer, size, actualGet);
            if (IS_FAIL(ans)) return 0;  // 接收失败返回0
            return actualGet;

        }

		// -----------------------------------------------------------------------
		// 清除接收缓存：清空 UDP 套接字接收缓冲区中的残留数据
		// 防止上次会话的残留数据报干扰本次通讯
		// -----------------------------------------------------------------------
		void clearReadCache() {
            _binded_socket->clearRxCache();
        }

		// -----------------------------------------------------------------------
		// 设置通道状态标志：UDP 通道的空实现
		//   flag - 状态标志位（未使用）
		// -----------------------------------------------------------------------
		void setStatus(_u32 flag){}

		// -----------------------------------------------------------------------
		// 获取通道类型：返回 UDP 通道类型标识
		// 用于上层判断当前使用的通道类型
		// -----------------------------------------------------------------------
		int getChannelType() {
            return CHANNEL_TYPE_UDP;
        }

	// -----------------------------------------------------------------------
	// 私有成员变量
	// -----------------------------------------------------------------------
	private:
		rp::net::DGramSocket * _binded_socket;  // UDP 数据报套接字指针（管理 UDP 通信）
		rp::net::SocketAddress _socket;          // 目标地址对象（RPLIDAR 设备的 IP 和端口）
        std::string _ip;                         // 目标 IP 地址字符串
        int _port;                               // 目标端口号
	};

    // -----------------------------------------------------------------------
    // 工厂函数：创建 UDP 通信通道对象
    //   ip   - RPLIDAR 设备的 IP 地址
    //   port - RPLIDAR 设备的 UDP 端口号
    // 返回 IChannel 指针（实际类型为 UdpChannel）
    // 上层通过此函数创建 UDP 通道，再传给 AsyncTransceiver::openChannelAndBind 使用
    // -----------------------------------------------------------------------
    Result<IChannel*> createUdpChannel(const std::string& ip, int port)
    {
        return new  UdpChannel(ip, port);
    }
}
