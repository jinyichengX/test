/*
 *  Slamtec LIDAR SDK
 *
 *  Copyright (c) 2014 - 2023 Shanghai Slamtec Co., Ltd.
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
// 本文件声明了 RPLidarProtocolCodec 类，实现 IAsyncProtocolCodec 接口，
// 负责 RPLIDAR 通讯协议的编解码。
//
// 编码(onEncodeData)：将 ProtocolMessage 编码为请求报文
//   起始标志(0xA5) + 命令 + 负载长度 + 负载数据 + 校验和
//   【通信协议 LR001 P.6】请求报文格式。
//
// 解码(onDecodeData)：从字节流解码应答报文，使用状态机处理：
//   STATUS_WAIT_SYNC1 -> STATUS_WAIT_SYNC2 -> STATUS_WAIT_SIZE_FLAG
//   -> STATUS_WAIT_TYPE -> STATUS_RECV_PAYLOAD
//   【通信协议 LR001 P.8】起始应答报文结构：
//     起始标志1(0xA5) + 起始标志2(0x5A) + 数据应答报文长度(30bits)
//     + 应答模式(2bits) + 数据类型(1byte)
//   当应答模式为 LOOP(0x1) 时，表示多次应答模式（扫描数据），
//   STATUS_LOOP_MODE_FLAG 标记多次应答模式，接收完一个报文后继续接收下一个。
//   【通信协议 LR001 P.5】单次请求-多次应答模式。
// ===================================================================

#pragma once

// 引入异步收发器层，获得 IAsyncProtocolCodec 接口、ProtocolMessage、
// message_autoptr_t 等定义。AsyncTransceiver 通过该接口与具体协议编解码实现解耦。
#include "sl_async_transceiver.h"

namespace sl { namespace internal {


// 协议消息解码监听器接口。
// 当 onDecodeData 从字节流中完整解码出一条应答报文时，通过本接口回调通知上层
// （如 RPLIDAR 驱动），将解码得到的 ProtocolMessage 传递给业务逻辑处理。
class IProtocolMessageListener {
public:
    // 解码完成回调：当一条完整的协议消息被解码出来后调用。
    //   msg - 解码得到的协议消息（const 引用，包含数据类型/负载等）
    virtual void onProtocolMessageDecoded(const ProtocolMessage&) = 0;
};


// RPLIDAR 协议编解码器。实现 IAsyncProtocolCodec 接口。
// 负责：
//   - 编码：将上层 ProtocolMessage（命令+负载）编码为线上的请求报文字节流
//   - 解码：将通道接收到的字节流按状态机还原为起始应答报文+数据应答报文
class RPLidarProtocolCodec : public IAsyncProtocolCodec
{
public:

    // 解码状态机的状态枚举。
    // 对应通信协议 PDF 第8页起始应答报文结构的逐字节解析过程。
    enum {
        // 等待起始标志1(0xA5)：状态机初始状态，逐字节寻找起始应答报文的首字节。
        // 【通信协议 LR001 P.8】起始标志1 为 1byte(0xA5)。
        STATUS_WAIT_SYNC1 = 0x0,
        // 等待起始标志2(0x5A)：收到0xA5后，等待第二个同步字节0x5A以确认报文开始。
        // 【通信协议 LR001 P.8】起始标志为2个字节固定数据：0xA5 0x5A。
        STATUS_WAIT_SYNC2 = 0x1,
        // 等待并接收 size_q30_subtype 字段（4字节）：
        //   低30位=数据应答报文长度；高2位=应答模式(0单次/1多次)。
        // 【通信协议 LR001 P.8】数据应答报文长度30bits + 应答模式2bits。
        STATUS_WAIT_SIZE_FLAG = 0x2,
        // 等待并接收数据类型字段（1字节）：表示后续数据应答报文的类型
        // （如0x81=标准测距、0x82=传统高速、0x04=设备信息等）。
        // 【通信协议 LR001 P.8】数据类型 1byte，与请求报文类型相对应。
        STATUS_WAIT_TYPE = 0x3,
        // 接收负载数据：根据 size_q30_subtype 低30位指定的长度接收数据应答报文主体。
        STATUS_RECV_PAYLOAD = 0x4,
        // 多次应答模式标志位(bit31)：当应答模式为 LOOP(0x1) 时置位，
        // 表示当前处于扫描测距的"单次请求-多次应答"模式，接收完一个数据应答报文后
        // 不结束解码，而是继续等待下一个报文的起始标志1(0xA5)。
        // 【通信协议 LR001 P.5】单次请求-多次应答模式：外部系统只需发送单次请求，
        //   并开始连续接收来自RPLIDAR的多个应答数据报文。
        // 【通信协议 LR001 P.8】应答模式0x1=多次应答模式，RPLIDAR将发送一个或多个应答报文。
        STATUS_LOOP_MODE_FLAG = 0x80000000,
    };

    // 默认构造函数：初始化解码状态机为等待起始标志1状态。
    RPLidarProtocolCodec();

    // 退出多次应答(LOOP)模式。
    // 清除 STATUS_LOOP_MODE_FLAG 标志，使解码器在接收完当前报文后停止继续等待
    // 下一个报文。对应外部系统发送 STOP 等命令要求 RPLIDAR 离开多次应答模式的场景。
    // 【通信协议 LR001 P.5】当工作在多次应答通讯模式时，外部系统可以通过发送停止
    //   请求或者其他类型的请求要求 RPLIDAR 离开多次应答模式。
    void exitLoopMode();


    // 估算给定消息编码后的请求报文总长度（字节）。
    // 请求报文 = 1(起始标志0xA5) + 1(命令) + 1(负载长度) + 负载大小 + 1(校验和)。
    // AsyncTransceiver::sendMessage 据此预分配发送缓冲区。
    // 【通信协议 LR001 P.6】请求报文格式：起始标志1byte + 请求命令1byte + 负载长度1byte
    //   + 请求负载数据0-255bytes + 校验和1byte。
    virtual size_t estimateLength(message_autoptr_t& message);


    // 编码：将 ProtocolMessage 编码为请求报文字节流写入 txbuffer。
    // 编码内容：0xA5 + cmd + size + payload[0..n] + checksum
    // 校验和公式：checksum = 0 ⨁ 0xA5 ⨁ CmdType ⨁ PayloadSize ⨁ Payload[0] ⨁ … ⨁ Payload[n]
    // 【通信协议 LR001 P.6】请求报文发送格式图表2-4；校验和计算公式。
    //   message  - 待编码的协议消息（cmd 为命令字，data 为负载数据）
    //   txbuffer - 预分配的发送缓冲区
    //   size     - 输入为缓冲区容量，输出为实际编码后的字节数
    virtual void onEncodeData(message_autoptr_t& message, _u8* txbuffer, size_t* size);

    // 解码重置：重置解码状态机为初始状态(STATUS_WAIT_SYNC1)，清空正在解码的消息。
    // AsyncTransceiver 的解码线程启动时调用。
    virtual void   onDecodeReset();
    // 解码数据：将新接收到的字节流喂入状态机进行协议解析。
    // 状态机逐字节处理：先找0xA5，再找0x5A，再收4字节size_q30_subtype，
    // 再收1字节type，再按长度接收payload。完整解码出一条报文后通过
    // IProtocolMessageListener::onProtocolMessageDecoded 回调通知上层。
    // 若处于LOOP模式，则继续等待下一个报文。
    // 【通信协议 LR001 P.8】起始应答报文结构图表2-7。
    // 【通信协议 LR001 P.5】单次请求-多次应答模式（扫描数据连续接收）。
    //   buffer - 接收到的数据指针
    //   size   - 数据长度（字节数）
    virtual void   onDecodeData(const void* buffer, size_t size);
    
    // 设置协议消息解码监听器，用于接收解码完成回调。
    //   l - 监听器指针
    void setMessageListener(IProtocolMessageListener* l);

protected:

    // 协议消息解码监听器指针，解码出完整报文后通过其 onProtocolMessageDecoded 回调。
    IProtocolMessageListener* _listener;
    // 正在解码的协议消息缓冲。状态机将解析出的数据类型/负载逐步填入此对象。
    ProtocolMessage          _decodingMessage;
    // 操作互斥锁：保护解码状态机相关数据的线程安全
    // （接收线程与解码线程分离时需保护共享状态）。
    rp::hal::Locker          _op_locker;
                            
    // 解码状态字：低4位为当前状态(STATUS_WAIT_SYNC1等)，
    // bit31 为 STATUS_LOOP_MODE_FLAG 多次应答模式标志。
    _u32                     _working_states;
    // 当前接收负载数据的写入位置（已接收字节数），用于 STATUS_RECV_PAYLOAD 状态
    // 判断负载是否接收完整。
    int                      _rx_pos;
};

}}

