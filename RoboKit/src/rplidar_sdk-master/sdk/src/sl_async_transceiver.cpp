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
// 文件说明：异步收发器（AsyncTransceiver）实现文件
// -----------------------------------------------------------------------------
// 本文件实现了 sl_async_transceiver.h 中声明的 ProtocolMessage（协议消息封装）
// 和 AsyncTransceiver（异步收发器）两个类的全部方法。
//
// 【通信协议 LR001 P.4-5】AsyncTransceiver 的双线程架构（RX线程 + 解码线程）专为
//   支持"单次请求-多次应答"模式而设计。该模式用于 RPLIDAR 进行扫描测距的场景：
//   外部系统在发送开始扫描的请求后，RPLIDAR 将开始连续的扫描测距，并持续发送
//   应答数据至外部系统。双线程通过接收队列(_rxQueue)解耦，保证高速数据流不丢失。
//
// 【通信协议 LR001 P.5】"单次请求-多次应答模式用于 RPLIDAR 进行扫描测距的模式下。
//   外部系统在发送开始扫描的请求后，RPLIDAR 将开始连续的扫描测距。"
// =============================================================================



#include "sdkcommon.h"
#include "hal/abs_rxtx.h"
#include "hal/thread.h"
#include "hal/types.h"
#include "hal/assert.h"
#include "hal/locker.h"
#include "hal/socket.h"
#include "hal/event.h"

#include "sl_async_transceiver.h"



namespace sl { namespace internal {




// =============================================================================
// ProtocolMessage 类方法实现
// =============================================================================
// -----------------------------------------------------------------------
// 默认构造函数：初始化为空消息
// len=0（无负载）、cmd=0（无命令）、data=NULL（无缓冲区）、_databufsize=0
// 随后调用 _changeBufSize() 分配初始缓冲区
// -----------------------------------------------------------------------
ProtocolMessage::ProtocolMessage()
        : len(0)
        , cmd(0)
        , data(NULL)
        , _databufsize(0)
		, _usingOutterData(false)
{
    _changeBufSize();
}

// -----------------------------------------------------------------------
// 带参构造函数：根据命令字和负载数据创建消息
//   cmd    - 请求命令字（如 0x20=SCAN, 0x25=STOP）
//   buffer - 负载数据指针（可为NULL，表示该命令无负载数据）
//   size   - 负载数据长度
// 【通信协议 LR001 P.6】部分请求命令（如 EXPRESS_SCAN、GET_LIDAR_CONF）需要
//   携带负载数据，而 STOP/RESET/SCAN 等命令无负载。
// -----------------------------------------------------------------------
ProtocolMessage::ProtocolMessage(_u8 cmd, const void* buffer, size_t size)
	: len(size)
	, cmd(cmd)
	, data(NULL)
    , _databufsize(0)
	, _usingOutterData(false)
{
    _changeBufSize();
	if (buffer)
	{
		memcpy(data, buffer, size);  // 将负载数据拷贝到内部缓冲区
	}
}

// -----------------------------------------------------------------------
// 拷贝构造函数：深拷贝源消息
// 独立分配新缓冲区并拷贝数据，确保两个消息对象互不影响
// force_compact=true 强制分配刚好适配的紧凑缓冲区
// -----------------------------------------------------------------------
ProtocolMessage::ProtocolMessage(const ProtocolMessage& srcMsg)
	: len(srcMsg.len)
	, cmd(srcMsg.cmd)
	, data(NULL)
    , _databufsize(0)
	, _usingOutterData(false)
{
    _changeBufSize( true );  // 强制紧凑分配
	if (srcMsg.data && len)
	{
		memcpy(data, srcMsg.data, len);  // 深拷贝负载数据
	}
}

// -----------------------------------------------------------------------
// 析构函数：清理负载数据，释放内部缓冲区内存
// -----------------------------------------------------------------------
ProtocolMessage::~ProtocolMessage()
{
	this->cleanData();
}

// -----------------------------------------------------------------------
// 赋值运算符：先清空自身，再深拷贝源消息的命令字和负载数据
// -----------------------------------------------------------------------
ProtocolMessage& ProtocolMessage::operator =(const ProtocolMessage& srcMessage)
{
	this->cleanData();  // 先释放自身的缓冲区


	this->len = srcMessage.len;  // 拷贝负载长度
	this->cmd = srcMessage.cmd;  // 拷贝命令字

    _changeBufSize( true );  // 强制紧凑分配新缓冲区
	if (srcMessage.data && len)
	{
		memcpy(data, srcMessage.data, len);  // 深拷贝负载数据
	}

	return *this;
}

// -----------------------------------------------------------------------
// 设置外部数据缓冲区（零拷贝模式）
// 直接使用外部传入的 buffer 指针，不进行数据拷贝。
// 注意：外部必须保证该缓冲区在 ProtocolMessage 使用期间有效。
// _usingOutterData 置为 true，析构/cleanData 时不会 delete 该缓冲区。
// -----------------------------------------------------------------------
void ProtocolMessage::setDataBuf(_u8 *buffer, size_t size)
{
	this->cleanData();  // 先清理原有缓冲区

	len = size;
	data = buffer;
    _databufsize = size;
	_usingOutterData = true;  // 标记为外部数据，不负责释放
}

// -----------------------------------------------------------------------
// 填充负载数据：调整缓冲区大小后拷贝数据
//   buffer - 数据源指针
//   size   - 数据长度
// -----------------------------------------------------------------------
void ProtocolMessage::fillData(const void * buffer, size_t size)
{
	len = size;
    _changeBufSize();  // 根据新长度调整缓冲区（可能复用已有缓冲区）
    if (buffer)
	    memcpy(data, buffer, size);  // 拷贝数据
}

// -----------------------------------------------------------------------
// 清空负载数据：释放内部缓冲区
// 若 _usingOutterData 为 true（外部数据），则不 delete，仅置空指针。
// 注意：清空后 len 被设为1而非0，这是为了兼容某些协议解析逻辑中对
// 最小负载长度的预期。
// -----------------------------------------------------------------------
void ProtocolMessage::cleanData()
{
	if (data)
	{
		if (!_usingOutterData)  // 仅释放内部管理的缓冲区
		{
			delete [] data;
		}
		data = NULL;
		len = 1;              // 重置长度（保留为1而非0，兼容性考虑）
        _databufsize = 0;
	}
}

// -----------------------------------------------------------------------
// 调整数据缓冲区大小（内部方法）
// 策略：尽量复用已有缓冲区以减少 new/delete 开销，提升性能
//   - 若新大小与当前容量相同：无需操作，直接返回
//   - 若新大小小于当前容量的一半以上：当前缓冲区过大，需释放以节省内存
//   - 若新大小略小于当前容量（不到一半）：可复用，除非 force_compact=true
//   - 若新大小大于当前容量：需重新分配更大缓冲区
// 注意：重新分配会先 cleanData（导致原有数据丢失），再恢复 len 并分配新缓冲区
// -----------------------------------------------------------------------
void ProtocolMessage::_changeBufSize( bool force_compact)
{
    size_t actual_size  = getPayloadSize();  // 当前负载长度

    size_t new_buf_size = actual_size;       // 目标缓冲区大小


    if (!_usingOutterData)  // 仅对内部管理的缓冲区进行调整
    {
        // nothing to do
        if ( new_buf_size == _databufsize ) return;  // 大小相同，无需调整

        if ( new_buf_size < _databufsize){  // 新大小小于当前容量

            if ( (_databufsize >> 1) < new_buf_size)  // 当前容量的一半仍小于新大小
            {
                // reuse the current buffer
                // 当前缓冲区虽略大但可复用，避免频繁分配
                if (!force_compact) return;  // 非强制压缩时直接复用
            }else
            {
                // the current buffer size is much bigger, we need to release it to save memory
                // 当前缓冲区远大于所需，释放以节省内存
            }
        }
    }

    // we need to change the buffer
    cleanData();  // 释放旧缓冲区（注意：cleanData 会重置 len）
    // the cleanData() will reset the length info, so we need to restore it
    len = actual_size;  // 恢复负载长度
    data = new _u8[new_buf_size];  // 分配新缓冲区
    _databufsize = new_buf_size;   // 更新缓冲区容量记录
}


// =============================================================================
// AsyncTransceiver 类方法实现
// =============================================================================
// -----------------------------------------------------------------------
// 构造函数：初始化异步收发器
//   codec - 协议编解码器引用（由上层注入，负责具体的协议编解码）
// 初始状态下通道为空、未工作、无标志位
// -----------------------------------------------------------------------
AsyncTransceiver::AsyncTransceiver(IAsyncProtocolCodec& codec)
	: _bindedChannel(NULL)
	, _codec(codec)
	, _isWorking(false)
    , _workingFlag(0)
{

}

// -----------------------------------------------------------------------
// 析构函数：自动解绑通道并关闭，确保线程安全退出
// -----------------------------------------------------------------------
AsyncTransceiver::~AsyncTransceiver()
{
    unbindAndClose();
}

// -----------------------------------------------------------------------
// 打开通信通道并绑定，启动接收线程和解码线程
// 流程：
//   1. 参数校验（通道非空）
//   2. 先解绑已有通道（unbindAndClose）
//   3. 加锁保护操作
//   4. 打开通道（channel->open）
//   5. 刷新通道缓冲区，清除残留数据
//   6. 设置工作状态标志
//   7. 启动解码线程（_proc_decoderThread）
//   8. 启动接收线程（_proc_rxThread）
//
// 【通信协议 LR001 P.40】典型工作流程中，外部系统需先建立通讯连接才能发送请求。
//   通道打开后即可开始发送 SCAN 等请求并接收应答数据。
// -----------------------------------------------------------------------
u_result AsyncTransceiver::openChannelAndBind(IChannel* channel)
{
    if (!channel) return RESULT_INVALID_DATA;  // 通道指针为空，参数非法

    unbindAndClose();  // 先清理已有绑定
	u_result ans = RESULT_OK;
	do
	{
		rp::hal::AutoLocker l(_opLocker);  // 加操作锁，保证线程安全

        // try to open the channel ...
        Result<nullptr_t> ans = SL_RESULT_OK;

        if (!channel->open()) {  // 打开通道（串口连接/TCP连接/UDP绑定）
            ans= RESULT_OPERATION_FAIL;  // 通道打开失败
            break;
        }


        // force a flush to clear any pending data
        // 强制刷新通道，清除任何残留的待处理数据
        // 避免上次会话的残留数据干扰本次通讯
        channel->flush();

		_dataEvt.set(false);  // 初始化数据事件为非触发状态

		_isWorking = true;    // 标记收发器开始工作
        _workingFlag = 0;     // 清除所有工作标志位
        _bindedChannel = channel;  // 绑定通道


        // 启动解码线程（先于接收线程启动，确保数据入队后能立即被消费）
		_decoderThread = CLASS_THREAD(AsyncTransceiver, _proc_decoderThread);
		// 启动接收线程（持续从通道读取数据）
		_rxThread = CLASS_THREAD(AsyncTransceiver, _proc_rxThread);




	} while (0);

	return ans;
}

// -----------------------------------------------------------------------
// 解绑通道并关闭：安全停止所有工作线程并释放资源
// 流程：
//   1. 加操作锁
//   2. 若未工作则直接返回
//   3. 设置 _isWorking=false，通知线程退出循环
//   4. 触发数据事件(_dataEvt.set())，唤醒可能阻塞在事件等待上的解码线程
//   5. 等待解码线程和接收线程退出(join)
//   6. 关闭并解绑通道
//   7. 清空接收队列中剩余的缓冲块
// -----------------------------------------------------------------------
void AsyncTransceiver::unbindAndClose()
{
	rp::hal::AutoLocker l(_opLocker);  // 加操作锁
	if (!_isWorking) return;  // 未在工作状态，无需清理

    assert(_bindedChannel);  // 断言通道已绑定


	_isWorking = false;      // 通知工作线程退出主循环
	_dataEvt.set();          // 触发数据事件，唤醒解码线程（可能在 wait 中阻塞）

	_decoderThread.join();   // 等待解码线程退出
	_rxThread.join();        // 等待接收线程退出


    _bindedChannel->close(); // 关闭通信通道

    _bindedChannel = NULL;   // 解绑通道指针


    // 清空接收队列中剩余的缓冲块，防止内存泄漏
    for (std::list< Buffer* >::iterator itr = _rxQueue.begin(); itr != _rxQueue.end(); ++itr)
    {
        delete [] *itr;  // 释放每个 Buffer 对象及其内部 data 数组
    }
    _rxQueue.clear();  // 清空队列

}

// -----------------------------------------------------------------------
// 发送协议消息：将消息编码为字节流后通过通道发送
// 流程：
//   1. 消息非空断言
//   2. 检查收发器是否在工作中
//   3. 加操作锁
//   4. 通过 codec 估算编码后的缓冲区大小
//   5. 分配发送缓冲区
//   6. 通过 codec 编码消息到缓冲区
//   7. 通过通道写出编码后的数据
//   8. 释放发送缓冲区
//
// 【通信协议 LR001 P.6】请求报文格式：起始标志(0xA5)+命令字+负载长度+负载数据+
//   校验和。编码器(onEncodeData)负责按此格式填充字节。
// 【通信协议 LR001 P.6】发送时序要求：完整的请求报文必须在5秒内完全发送至RPLIDAR，
//   否则协议栈将认为通讯超时并丢弃该报文。因此 write 操作不应长时间阻塞。
// -----------------------------------------------------------------------
u_result AsyncTransceiver::sendMessage(message_autoptr_t& msg)
{
    assert(msg);  // 消息指针非空断言

    if (!_isWorking) return RESULT_OPERATION_NOT_SUPPORT;  // 收发器未工作，不支持发送

    rp::hal::AutoLocker l(_opLocker);  // 加操作锁，防止与 unbindAndClose 并发

    // 估算编码后报文的总字节数（含起始标志、命令字、负载长度、负载、校验和等）
    size_t requiredBufferSize = _codec.estimateLength(msg);

    if (requiredBufferSize == 0) {
        // nothing to send
        return RESULT_OK;  // 编码后长度为0，无需发送（如空消息）
    }

    u_result ans = RESULT_OK;

    _u8* txBuffer = new _u8[requiredBufferSize];  // 分配发送缓冲区

    do {

        if (!txBuffer) {
            return RESULT_INSUFFICIENT_MEMORY;  // 内存分配失败
        }

        // 将协议消息编码为字节流（填充起始标志、命令字、负载、校验和等）
        // 【通信协议 LR001 P.6】checksum = 0 ^ 0xA5 ^ CmdType ^ PayloadSize ^ Payload[0..n]
        _codec.onEncodeData(msg, txBuffer, &requiredBufferSize);

        // 通过通道发送编码后的字节流
        // 【通信协议 LR001 P.6】报文需在5秒内完整发送
        int txSize = _bindedChannel->write(txBuffer, requiredBufferSize);

        if (txSize < 0) ans = RESULT_OPERATION_FAIL;  // 写入失败

    } while (0);


    delete[] txBuffer;  // 释放发送缓冲区
    return ans;
}

// -----------------------------------------------------------------------
// 接收(RX)线程主函数：高优先级线程，持续从通道读取原始数据放入接收队列
//
// 工作流程：
//   1. 设置线程优先级为 HIGH（确保及时接收数据，避免缓冲区溢出）
//   2. 循环直到 _isWorking 变为 false
//   3. 调用 waitForDataExt 等待通道有数据可读（超时1秒）
//   4. 超时则继续循环；其他错误则设置错误标志并通知 codec，退出线程
//   5. 分配 Buffer 并从通道读取数据
//   6. 加锁将 Buffer 入队，触发数据事件通知解码线程
//
// 【通信协议 LR001 P.5】RPLIDAR 在扫描测距模式下会连续发送多个应答数据包，
//   该线程负责不间断地从通道搬运这些数据。每个测距采样点的信息通过独立应答包
//   发送，因此数据流是持续且高速的，需要高优先级线程保证不丢失。
// 【通信协议 LR001 P.10】扫描采样状态下，RPLIDAR 在扫描电机转速稳定后才开始
//   输出测距数据，此后持续发送直到收到 STOP 请求或发生故障。
// -----------------------------------------------------------------------
sl_result AsyncTransceiver::_proc_rxThread()
{
    assert(_bindedChannel);  // 断言通道已绑定

    // 设置线程优先级为高，确保及时读取通道数据
    // 避免因线程调度延迟导致底层串口/网络缓冲区溢出而丢数据
    rp::hal::Thread::SetSelfPriority(rp::hal::Thread::PRIORITY_HIGH);

    u_result result;
    size_t hintedSize = 0;  // 通道提示的可读数据量
    while (_isWorking)
    {
        // 等待通道有数据可读，超时1秒
        // hintedSize 返回当前可读取的字节数（用于预分配接收缓冲区）
        result = _bindedChannel->waitForDataExt(hintedSize, 1000);

        if (IS_FAIL(result))
        {
            // timeout is allowed
            // 超时是允许的（RPLIDAR 可能暂时没有数据发送，如等待电机稳定期间）
            if (result == RESULT_OPERATION_TIMEOUT) {
                continue;  // 超时则继续下一轮等待
            }
            // 其他错误（如通道断开），设置错误标志并通知 codec
            if (_isWorking) {
                _workingFlag |= WORKING_FLAG_ERROR;  // 标记通道错误
                _codec.onChannelError(result);        // 回调通知上层处理错误
                break;  // 退出接收线程
            }
        }

        // no data in buffer, sleep and wait for the next round
        // 通道无可读数据（hintedSize为0），短暂让出CPU后继续等待
        if (!hintedSize)
        {
            continue;
        }


        // 分配接收缓冲块，根据 hintedSize 预分配内存
        Buffer* decodeBuffer = new Buffer();

        decodeBuffer->data = new _u8[hintedSize];  // 预分配数据缓冲区

        // 从通道读取数据到缓冲块
        // 实际读取的字节数可能小于 hintedSize
        decodeBuffer->size = _bindedChannel->read(decodeBuffer->data, hintedSize);
#ifdef _DEBUG_DUMP_PACKET
        printf("Revc: %d\n", decodeBuffer->size);  // 调试：打印接收到的字节数
#endif

        if  (!decodeBuffer->size) {
            // 读取到0字节，表示通道异常断开
            delete decodeBuffer;


            _workingFlag |= WORKING_FLAG_ERROR;  // 标记通道错误
            _codec.onChannelError(RESULT_OPERATION_ABORTED);  // 通知上层通道中断
            break;  // 退出接收线程
        }

        // 断言实际读取量不超过预分配大小
        assert(hintedSize >= decodeBuffer->size);


#ifdef _DEBUG_DUMP_PACKET
        // 调试：以十六进制转储接收到的原始数据包
        printf("=== Dump RX Packet, size = %d ===\n", decodeBuffer->size);
        for (int pos = 0; pos < decodeBuffer->size; pos++)
        {
            printf("%02x ", decodeBuffer->data[pos]);
        }
        printf("\n=== END ===\n");
#endif

        // 加锁将缓冲块放入接收队列，并触发数据事件唤醒解码线程
        _rxLocker.lock();
        _rxQueue.push_back(decodeBuffer);  // 数据入队
        _dataEvt.set();                     // 通知解码线程有新数据
        _rxLocker.unlock();


    }
    // 线程退出时标记RX功能已禁用
    _workingFlag |= WORKING_FLAG_RX_DISABLED;
    return RESULT_OK;
}

// -----------------------------------------------------------------------
// 解码线程主函数：高优先级线程，从接收队列取出数据交给编解码器解析
//
// 工作流程：
//   1. 设置线程优先级为 HIGH
//   2. 初始重置解码器状态(onDecodeReset)
//   3. 循环直到 _isWorking 变为 false
//   4. 加锁检查接收队列是否为空
//   5. 若队列为空，解锁并等待数据事件(超时1秒)
//   6. 从队列头部取出一个缓冲块
//   7. 调用 codec 的 onDecodeData 进行协议解码
//   8. 释放已处理的缓冲块
//
// 【通信协议 LR001 P.7-8】解码器需识别起始应答报文(0xA5 0x5A)，提取30bit数据
//   应答长度、2bit应答模式、1byte数据类型。然后按对应格式解析后续数据应答报文。
// 【通信协议 LR001 P.5】扫描测距模式下 RPLIDAR 会连续发送多个应答数据包，解码
//   线程持续从队列取出数据块并交给解码器逐个还原出采样点数据(距离、角度、质量等)。
// -----------------------------------------------------------------------
sl_result AsyncTransceiver::_proc_decoderThread()
{

    assert(_bindedChannel);  // 断言通道已绑定
    // 设置线程优先级为高，确保及时解码数据，避免接收队列堆积过多
    rp::hal::Thread::SetSelfPriority(rp::hal::Thread::PRIORITY_HIGH);
    // 解码器状态重置：清空残留的半截报文等内部状态
    _codec.onDecodeReset();


    while (_isWorking)
    {
        _rxLocker.lock();  // 加锁访问接收队列

        if (_rxQueue.empty())  // 队列为空，无数据可解码
        {
            _rxLocker.unlock();  // 解锁后再等待，避免持锁阻塞

            // 等待数据事件，超时1秒
            // 接收线程放入数据后会 set 事件唤醒此处
            if (_dataEvt.wait(1000))
                continue;  // 事件触发（有新数据），返回循环重新检查队列

            _rxLocker.lock();  // 重新加锁取数据
        }
        // 断言队列非空（被唤醒后应有数据）
        assert(!_rxQueue.empty());

        // 从队列头部取出一个缓冲块
        Buffer * bufferToDecode = _rxQueue.front();
        _rxQueue.pop_front();  // 出队

        _rxLocker.unlock();  // 解锁，允许接收线程继续入队

        //cout<<"decoding "<< bufferToDecode->size <<" bytes of data"<<endl;
        // 将数据交给编解码器进行协议解析
        // 解码器内部维护状态机，逐字节解析协议报文
        // 完整报文解析完成后，通过 IProtocolMessageListener 回调通知上层
        _codec.onDecodeData(bufferToDecode->data, bufferToDecode->size);


        delete bufferToDecode;  // 释放已处理的缓冲块
    }

    return RESULT_OK;

}


}}
