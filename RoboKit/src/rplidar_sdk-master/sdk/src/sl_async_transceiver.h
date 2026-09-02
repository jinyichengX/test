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
// 文件说明：异步收发器（AsyncTransceiver）层头文件
// -----------------------------------------------------------------------------
// 本文件定义了 RPLIDAR SDK 中异步数据收发的核心抽象层。该层位于通信通道层
// (IChannel) 之上，负责将以太网/串口等底层通道读取到的原始字节流，通过解码器
// (IAsyncProtocolCodec) 还原为结构化的协议消息(ProtocolMessage)，并将上层待发送
// 的协议消息编码为字节流后通过通道写出。
//
// 【通信协议 LR001 P.4-5】RPLIDAR 通讯采用三种请求/应答模式：
//   1. 单次请求-单次应答（用于获取设备信息、健康状态等）
//   2. 单次请求-多次应答（用于扫描测距，RPLIDAR 连续发送采样点应答包）
//   3. 单次请求-无应答（用于 STOP/RESET 等控制命令）
// AsyncTransceiver 的双线程（RX线程 + 解码线程）架构正是为了高效支持"单次请求-
// 多次应答"模式而设计：扫描测距时 RPLIDAR 会持续、高速地发送应答数据，需要独立
// 线程持续接收，避免数据丢失。
// =============================================================================

#pragma once

#include <list>
#include <memory>

namespace sl { namespace internal {


// =============================================================================
// ProtocolMessage —— 协议消息封装类
// -----------------------------------------------------------------------------
// 该类封装了一条 RPLIDAR 通讯协议消息的核心字段：命令字(cmd)、负载数据(data)
// 和负载长度(len)。
// 【通信协议 LR001 P.6】请求报文格式为：起始标志(0xA5) + 请求命令(1byte) +
//   负载长度(1byte) + 请求负载数据(0~255bytes) + 校验和(1byte)。
// 这里的 cmd 对应"请求命令"字段，data/len 对应"请求负载数据"及其长度。
// ProtocolMessage 本身不含起始标志和校验和——这些由编解码器(IAsyncProtocolCodec)
// 在编码/解码时负责填充和校验。
//
// 该类使用 _single_thread 标注，表示其设计上不保证线程安全，调用者需在同一线程
// 中访问，或通过外部锁保护。
// =============================================================================
class _single_thread ProtocolMessage {

public:
	size_t len;			// 负载数据的有效长度（字节数）。对应协议报文中负载长度字段所描述的数据量。
	_u8 cmd;            // 请求命令字（1字节）。例如 0x20=SCAN扫描, 0x25=STOP停止, 0x50=GET_INFO获取设备信息
                        // 【通信协议 LR001 P.12】请求命令总览中列出了所有支持的命令值。
protected:
	_u8* data;          // 指向负载数据缓冲区的指针。存放请求/应答的负载数据内容。
	size_t _databufsize;// 数据缓冲区的实际分配容量（字节数）。可能大于 len，用于支持缓冲区复用以减少内存分配。
                        // 设计目的：当新数据比旧数据略大或略小时，可复用已有缓冲区，避免频繁 new/delete。

public:
	// 默认构造函数：初始化为空消息（长度0、命令0），并分配初始缓冲区
	ProtocolMessage();
	// 带参构造函数：根据给定的命令字和负载数据创建消息
	//   cmd    - 请求命令字
	//   buffer - 指向负载数据的指针（可为NULL，表示有负载长度但暂无数据）
	//   size   - 负载数据长度
	ProtocolMessage(_u8 cmd, const void* buffer, size_t size);
	// 拷贝构造函数：深拷贝源消息的命令字和负载数据（独立分配新缓冲区）
	ProtocolMessage(const ProtocolMessage& srcMsg);
	// 虚析构函数：清理负载数据缓冲区，释放内存
	virtual ~ProtocolMessage();

	// 赋值运算符：先清空自身数据，再深拷贝源消息的命令字和负载数据
	ProtocolMessage& operator=(const ProtocolMessage& srcMessage);

	// 设置外部数据缓冲区（零拷贝模式）
	// 注意：避免使用此方法，请优先使用 fillData。
	// 该方法直接将外部传入的 buffer 指针作为数据缓冲区，不进行拷贝，因此外部
	// 必须保证该缓冲区在 ProtocolMessage 使用期间有效。_usingOutterData 会被置
	// 为 true，析构时不会 delete 该缓冲区。
	//   buffer - 外部数据缓冲区指针
	//   size   - 数据长度
	void setDataBuf(_u8* buffer, size_t size);

	// 获取内部数据缓冲区的直接指针（供编解码器直接读写负载数据）
	_u8* getDataBuf() { return data; }

	// 填充负载数据：根据需要调整缓冲区大小，并将 buffer 内容拷贝进来
	//   buffer - 数据源指针
	//   size   - 数据长度
	void fillData(const void* buffer, size_t size);
	// 清空负载数据：释放内部缓冲区（若非外部数据），并将长度置为1
	void cleanData();

	// 获取负载数据的大小（即 len 字段的值）
	// 编解码器在估算编码后报文长度时会调用此方法
	size_t getPayloadSize() const
	{
		return len;
	}

protected:

	// 调整数据缓冲区大小以适应当前负载长度(len)
	// 策略：尽量复用已有缓冲区以减少内存分配开销。
	//   - 若新大小与当前容量相同，则直接返回不做操作
	//   - 若新大小小于当前容量的一半以下，则释放过大缓冲区以节省内存
	//   - 若 force_compact 为 true，则即使可复用也强制重新分配紧凑缓冲区
	// 注意：调用后原有负载数据会丢失（因为会先 cleanData 再重新分配）
	//   force_compact - 是否强制压缩缓冲区到刚好适配的大小
	void _changeBufSize(bool force_compact = false);
	bool _usingOutterData;  // 是否使用外部数据缓冲区（true=外部管理内存，析构时不释放；false=内部管理）
};



// ProtocolMessage 的智能指针类型定义
// 使用 std::shared_ptr 管理协议消息的生命周期，便于在异步收发线程间传递
// 消息对象时自动管理内存
typedef std::shared_ptr<ProtocolMessage> message_autoptr_t;


// =============================================================================
// IAsyncProtocolCodec —— 异步协议编解码器接口
// -----------------------------------------------------------------------------
// 定义了 AsyncTransceiver 与具体协议实现之间的解耦接口。
// 上层（如 RPLIDAR 驱动）需实现此接口，提供：
//   1. 编码：将 ProtocolMessage 编码为字节流（onEncodeData）
//   2. 解码：将接收到的字节流还原为协议消息（onDecodeData）
//   3. 估算编码后报文长度（estimateLength）
//   4. 解码状态重置（onDecodeReset）
//   5. 通道错误回调（onChannelError）
//
// 【通信协议 LR001 P.6-8】编码时需按照协议报文格式填充起始标志(0xA5)、命令字、
// 负载长度、负载数据和校验和；解码时需识别起始应答报文(0xA5 0x5A)并提取30bit
// 长度、2bit应答模式、1byte数据类型等信息。
// =============================================================================
class IAsyncProtocolCodec {
public:
	// 默认构造函数
	IAsyncProtocolCodec() {}
	// 虚析构函数（确保派生类对象通过基类指针删除时正确析构）
	virtual ~IAsyncProtocolCodec()  {}

	// 通道错误回调：当下层通道发生读写错误时由 AsyncTransceiver 调用
	// 上层可在此实现错误处理逻辑（如标记设备离线、触发重连等）
	//   errCode - 错误码（u_result 类型）
	virtual void   onChannelError(u_result errCode) {}

	// 解码重置：在开始新一轮解码前重置解码器内部状态
	// 例如清空已缓存的半截报文、重置状态机等。AsyncTransceiver 的解码线程启动
	// 时会调用此方法。
	virtual void   onDecodeReset() {}
	// 解码数据：将新接收到的字节流喂给解码器进行协议解析
	// 解码器内部维护状态机，逐字节解析协议报文。当完整报文解析完成时，应通过
	// IProtocolMessageListener 回调通知上层。
	// 【通信协议 LR001 P.5】扫描测距模式下 RPLIDAR 会连续发送多个应答数据包，
	// 解码器需持续处理流入的字节流，逐个还原出采样点数据。
	//   buffer - 接收到的数据指针
	//   size   - 数据长度（字节数）
	virtual void   onDecodeData(const void* buffer, size_t size) = 0;


	// 估算给定消息编码后的字节流总长度（含起始标志、命令字、负载、校验和等）
	// AsyncTransceiver::sendMessage 依赖此方法预先分配发送缓冲区
	// 【通信协议 LR001 P.6】请求报文总长度 = 1(起始标志) + 1(命令) + 1(负载长度) +
	//   负载大小 + 1(校验和)，具体长度取决于该请求是否携带负载数据
	//   message - 待发送的协议消息
	//   返回值  - 编码后的字节流总长度
	virtual size_t estimateLength(message_autoptr_t& message) = 0;
	// 编码消息：将协议消息编码为可发送的字节流
	// 【通信协议 LR001 P.6】编码时需按小端序填充报文，起始标志0xA5、命令字、
	//   负载长度、负载数据、校验和（checksum = 0 ^ 0xA5 ^ CmdType ^ PayloadSize ^ Payload[0..n]）
	//   message  - 待编码的协议消息
	//   txbuffer - 预分配的发送缓冲区（由调用者根据 estimateLength 的返回值分配）
	//   size     - 输入为缓冲区容量，输出为实际编码后的字节数
	virtual void   onEncodeData(message_autoptr_t& message, _u8* txbuffer, size_t* size) = 0;

};

// =============================================================================
// AsyncTransceiver —— 异步收发器
// -----------------------------------------------------------------------------
// 核心异步通信引擎，采用"双线程+队列"架构管理数据的收发：
//   - _rxThread（接收线程）：高优先级，持续从 IChannel 读取原始字节流，存入
//     _rxQueue 队列。对应 RPLIDAR 连续发送应答数据的场景。
//   - _decoderThread（解码线程）：高优先级，从 _rxQueue 取出数据块，交给
//     IAsyncProtocolCodec 解码。解码后的完整消息通过回调通知上层。
//
// 【通信协议 LR001 P.4-5】该双线程架构专为支持"单次请求-多次应答"模式而设计。
//   外部系统发送一次 SCAN 请求后，RPLIDAR 将开始连续的扫描测距，并持续发送
//   采样点应答数据。接收线程负责不间断地搬运数据，解码线程负责协议解析，
//   两者通过 _rxQueue 解耦，保证高速数据流不丢失。
//
// 【通信协议 LR001 P.10】扫描采样状态下，RPLIDAR 会在扫描电机转速稳定后才开始
//   输出测距数据，此后持续发送直到收到 STOP 请求或发生故障。
// =============================================================================
class AsyncTransceiver {
public:

	// 工作状态标志位枚举（使用位掩码，可组合）
	enum working_flag_t
	{
		// 接收(RX)功能已禁用：接收线程已退出或异常停止
		WORKING_FLAG_RX_DISABLED = 0x1L << 0,
		// 发送(TX)功能已禁用：发送功能不可用
		WORKING_FLAG_TX_DISABLED = 0x1L << 1,

		// 通道发生错误：接收或发送过程中出现异常，需上层处理
		// 使用最高位，避免与低位标志冲突
		WORKING_FLAG_ERROR = 0x1L << 31,
	};


	// 构造函数：绑定一个协议编解码器(codec)
	//   codec - 编解码器引用（由上层实现，负责具体的协议编解码逻辑）
	AsyncTransceiver(IAsyncProtocolCodec& codec);
	// 析构函数：自动解绑通道并关闭，停止所有工作线程
	~AsyncTransceiver();



	// 打开通信通道并绑定，随后启动接收线程和解码线程
	// 流程：打开通道 -> 刷新缓冲区 -> 启动解码线程 -> 启动接收线程
	// 【通信协议 LR001 P.40】典型工作流程中，外部系统在发送请求前需先建立通讯连接
	//   并确保通道可用。
	//   channel - 待打开并绑定的通信通道（串口/TCP/UDP等IChannel实现）
	//   返回值  - RESULT_OK 成功；RESULT_INVALID_DATA 通道为空；RESULT_OPERATION_FAIL 打开失败
	u_result openChannelAndBind(IChannel* channel);
	// 解绑通道并关闭：停止工作线程、关闭通道、清空接收队列
	void     unbindAndClose();

	// 获取当前绑定的通信通道指针
	IChannel* getBindedChannel() const {
		return _bindedChannel;
	}

	// 发送协议消息：通过编解码器将消息编码为字节流后写入通道
	// 【通信协议 LR001 P.6】编码后的请求报文需在5秒内完整发送至RPLIDAR，否则
	//   协议栈将认为通讯超时并丢弃该报文。因此发送操作应在合理时间内完成。
	//   msg - 待发送的协议消息（智能指针）
	//   返回值 - RESULT_OK 成功；RESULT_OPERATION_NOT_SUPPORT 收发器未工作；
	//            RESULT_INSUFFICIENT_MEMORY 内存不足；RESULT_OPERATION_FAIL 写入失败
	u_result sendMessage(message_autoptr_t& msg);

protected:

	// 接收(RX)线程主函数：高优先级线程，持续从通道读取数据放入接收队列
	// 【通信协议 LR001 P.5】RPLIDAR 在扫描测距模式下会连续发送多个应答数据包，
	//   该线程负责不间断地从通道搬运这些数据，确保不丢失。
	sl_result _proc_rxThread();
	// 解码线程主函数：高优先级线程，从接收队列取出数据交给编解码器解析
	// 解码后的完整协议消息通过 IProtocolMessageListener 回调通知上层
	sl_result _proc_decoderThread();

protected:


	rp::hal::Locker _opLocker;  // 操作互斥锁：保护 openChannelAndBind/unbindAndClose/sendMessage 等操作的线程安全
	rp::hal::Locker _rxLocker;  // 接收队列互斥锁：保护 _rxQueue 的线程安全访问（接收线程写、解码线程读）
	rp::hal::Event  _dataEvt;   // 数据到达事件：接收线程放入数据后触发(set)，解码线程在队列空时等待(wait)

	IChannel* _bindedChannel;           // 当前绑定的通信通道指针（串口/TCP/UDP等）
	IAsyncProtocolCodec& _codec;        // 协议编解码器引用（由上层注入，负责具体协议解析）


	bool _isWorking;    // 收发器是否正在工作（true=线程运行中，false=已停止/未启动）
	_u32 _workingFlag;  // 工作状态标志位（working_flag_t 的位掩码组合，记录RX/TX/ERROR状态）

	rp::hal::Thread _rxThread;      // 接收线程对象：从通道读取原始数据
	rp::hal::Thread _decoderThread; // 解码线程对象：解析协议数据

	// 接收数据缓冲块结构：封装一块从通道读取到的原始数据
	// 接收线程每次读取数据后创建一个 Buffer 实例并入队，解码线程取出并处理后删除
	struct Buffer {
		size_t size;  // 该缓冲块中有效数据的字节数
		_u8* data;    // 指向数据缓冲区的指针


		// 默认构造函数：初始化为空缓冲区
		Buffer() : size(0), data(NULL){}

		// 析构函数：释放数据缓冲区内存
		~Buffer() {
			if (data) {
				delete[] data;
				data = NULL;
			}
		}
	};
	// 接收数据队列：接收线程生产(入队)，解码线程消费(出队)
	// 使用 std::list 实现，支持在尾部高效插入、头部高效取出
	std::list< Buffer * > _rxQueue;
};


}}
