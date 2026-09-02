/*
 *  Slamtec LIDAR SDK
 *
 *  Copyright (c) 2014 - 2023 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */
 /*
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted that the following conditions are met:
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
// 本文件实现 RPLidarProtocolCodec，即 RPLIDAR 通讯协议的编解码逻辑。
//
// onEncodeData：编码（请求方向，外部系统 -> RPLIDAR）
//   将 ProtocolMessage(cmd+payload) 编码为请求报文：
//     起始标志(0xA5) + 命令 + 负载长度 + 负载数据 + 校验和
//   【通信协议 LR001 P.6】请求报文格式图表2-4。
//
// onDecodeData：解码（应答方向，RPLIDAR -> 外部系统）
//   从字节流按状态机解码起始应答报文头 + 数据应答报文负载：
//     STATUS_WAIT_SYNC1(找0xA5) -> STATUS_WAIT_SYNC2(找0x5A)
//     -> STATUS_WAIT_SIZE_FLAG(收4字节:30位长度+2位应答模式)
//     -> STATUS_WAIT_TYPE(收1字节数据类型) -> STATUS_RECV_PAYLOAD(收负载)
//   【通信协议 LR001 P.8】起始应答报文结构：起始标志1(0xA5)+起始标志2(0x5A)
//     +数据应答报文长度(30bits)+应答模式(2bits)+数据类型(1byte)。
//   当应答模式为 LOOP(0x1) 时置 STATUS_LOOP_MODE_FLAG，进入多次应答模式，
//   接收完一个数据应答报文后继续等待下一个（扫描测距数据）。
//   【通信协议 LR001 P.5】单次请求-多次应答模式。
// ===================================================================



#include "sdkcommon.h"
// 字节序转换工具（小端<->CPU序），用于解析 size_q30_subtype 等32位字段。
#include "hal/byteorder.h"
#include "hal/abs_rxtx.h"
#include "hal/thread.h"
#include "hal/types.h"
#include "hal/assert.h"
#include "hal/locker.h"
#include "hal/socket.h"
#include "hal/event.h"

// RPLIDAR 驱动与协议常量（RPLIDAR_CMD_SYNC_BYTE/RPLIDAR_ANS_SYNC_BYTE1 等）。
#include "sl_lidar_driver.h"
// CRC32 校验（HQ 模式报文校验）。
#include "sl_crc.h" 
#include <algorithm>

// 异步收发器层（IAsyncProtocolCodec/ProtocolMessage/message_autoptr_t）。
#include "sl_async_transceiver.h"
// 本类的声明。
#include "sl_lidarprotocol_codec.h"



namespace sl { namespace internal {



// 构造函数：初始化编解码器。
// 监听器置空，操作锁初始化为可用(autocreate=true)，并调用 onDecodeReset
// 将解码状态机置于初始状态(STATUS_WAIT_SYNC1)。
RPLidarProtocolCodec::RPLidarProtocolCodec()
    : IAsyncProtocolCodec()
    , _listener(NULL)
    , _op_locker(true)
{
    // 重置解码状态机为初始状态。
    onDecodeReset();
}

// 退出多次应答(LOOP)模式。
// 直接重置解码状态机，清除 STATUS_LOOP_MODE_FLAG，使解码器不再连续等待
// 下一个数据应答报文。对应外部系统发送 STOP 等命令要求 RPLIDAR 离开
// 扫描测距（多次应答）模式的场景。
// 【通信协议 LR001 P.5】外部系统可以通过发送停止请求或者其他类型的请求
//   要求 RPLIDAR 离开多次应答模式。
void RPLidarProtocolCodec::exitLoopMode() {
    onDecodeReset();
}


// 设置协议消息解码监听器。解码出完整报文后通过该监听器回调上层。
//   listener - 监听器指针
void RPLidarProtocolCodec::setMessageListener(IProtocolMessageListener* listener)
{
    // 加锁保护 _listener 的线程安全访问。
    rp::hal::AutoLocker l(_op_locker);
    _listener = listener;
}

// 估算给定消息编码后的请求报文总长度（字节）。
// 请求报文 = 1(起始标志0xA5) + 1(命令) + [若带负载: 1(负载长度) + 负载 + 1(校验和)]
// 【通信协议 LR001 P.6】请求报文格式：起始标志1byte + 请求命令1byte +
//   负载长度1byte + 请求负载数据0-255bytes + 校验和1byte。
//   message - 待编码的协议消息
//   返回值  - 编码后的字节流总长度
size_t RPLidarProtocolCodec::estimateLength(message_autoptr_t& message)
{
    // 至少包含起始标志(1B)和命令字(1B)，共2字节。
    size_t actualSize = 2; //1-byte's sync byte, 1-byte's cmd byte

    // 若命令带有负载标志(SL_LIDAR_CMDFLAG_HAS_PAYLOAD=0x80)，则还需附带
    // 负载长度(1B)、负载数据本身、校验和(1B)。
    if (message->cmd & RPLIDAR_CMDFLAG_HAS_PAYLOAD) {
        // 负载数据长度（取低8位，协议负载长度字段为1字节，最大255）。
        actualSize += (message->getPayloadSize() & 0xFF);
        // 负载长度字段(1B) + 校验和字段(1B)。
        actualSize += 2; //1-byte for size field, 1-byte for checksum
    }

    return actualSize;
}


// 编码：将 ProtocolMessage 编码为请求报文字节流，写入 buffer。
// 报文布局：[0]=0xA5(起始标志) [1]=cmd(命令) [2]=size(负载长度) [3..]=payload [末]=checksum
// 校验和公式：checksum = 0 ⨁ 0xA5 ⨁ CmdType ⨁ PayloadSize ⨁ Payload[0] ⨁ … ⨁ Payload[n]
// 实现方式：从 checksum=0 开始，对每个待发送字节依次异或累计；
//   当写到最后一个字节（校验和位置）时，把累计的 checksum 写入该位置。
// 【通信协议 LR001 P.6】请求报文发送格式图表2-4；校验和计算公式。
//   message - 待编码的协议消息
//   buffer  - 预分配的发送缓冲区
//   size    - 输入为缓冲区容量，输出为实际编码后的字节数
void RPLidarProtocolCodec::onEncodeData(message_autoptr_t& message, _u8* buffer, size_t* size)
{
    // 校验和初值为0，按协议公式 checksum = 0 ⨁ 0xA5 ⨁ CmdType ⨁ ...
    _u8 checksum = 0;
    // 实际写入字节数取缓冲区容量与估算长度的较小值。
    size_t writeSize = std::min<size_t>(*size, estimateLength(message));
    size_t currentPos = 0;

    // 逐字节填充请求报文。
    while (currentPos < writeSize) {
        _u8 currentTxByte;
        switch (currentPos) {
        case 0: // 起始标志字节 0xA5。
            // 【通信协议 LR001 P.6】每个请求报文均以固定的 0xA5 作为开始字节。
            currentTxByte = RPLIDAR_CMD_SYNC_BYTE;
            break;
        case 1: // 命令字节（含可能的 HAS_PAYLOAD 标志位）。
            // 【通信协议 LR001 P.6】所有请求报文都必须包含一个字节长度的请求命令字段。
            currentTxByte = message->cmd;
            break;
        case 2: // 负载长度字节（仅当命令带负载时存在）。
            currentTxByte = (_u8)message->getPayloadSize();
            break;
        default:
        {
            // 从第4字节起为负载数据，最后1字节为校验和。
            size_t payloadPos = currentPos - 3;
            if (payloadPos == message->getPayloadSize()) {
                // 校验和字节：写入此前累计的 checksum 值。
                // 此时 checksum = 0 ⨁ 0xA5 ⨁ cmd ⨁ size ⨁ payload[0..n]，与协议公式一致。
                // 【通信协议 LR001 P.6】checksum = 0 ⨁ 0xA5 ⨁ CmdType ⨁ PayloadSize
                //   ⨁ Payload[0] ⨁ … ⨁ Payload[n]
                currentTxByte = checksum;
                assert(currentPos + 1 == writeSize);
            }
            else {
                // 负载数据字节。
                currentTxByte = message->getDataBuf()[payloadPos];
            }
        }
        }


        // 对当前字节继续异或累计到 checksum。
        // 注意：当当前字节为校验和本身时，checksum ^= checksum 会清零，
        // 但此时 currentTxByte 已保存了正确的校验和值并写入 buffer，不影响结果。
        checksum ^= currentTxByte;
        buffer[currentPos++] = currentTxByte;
    } while (0);

    // 输出实际编码后的字节数。
    *size = currentPos;
}

// 解码重置：清空正在解码的消息并重置状态机为初始状态。
// AsyncTransceiver 的解码线程启动时调用；exitLoopMode 也复用此实现。
void   RPLidarProtocolCodec::onDecodeReset() {
    // 加锁保护解码状态相关数据。
    rp::hal::AutoLocker autolock(_op_locker);
    // 清空正在解码的半截报文数据。
    // flush the pending data
    _decodingMessage.cleanData();
    // 重置接收位置计数器。
    // reset to initial state
    _rx_pos = 0;
    // 重置状态机为等待起始标志1(0xA5)。
    _working_states = STATUS_WAIT_SYNC1;
}


// 解码数据：将接收到的字节流喂入状态机进行协议解析。
// 状态机逐字节处理起始应答报文：
//   STATUS_WAIT_SYNC1(找0xA5) -> STATUS_WAIT_SYNC2(找0x5A)
//   -> STATUS_WAIT_SIZE_FLAG(收4字节size_q30_subtype)
//   -> STATUS_WAIT_TYPE(收1字节type) -> STATUS_RECV_PAYLOAD(按长度收负载)
// 完整解码出一条报文后通过 IProtocolMessageListener 回调通知上层。
// 若处于 LOOP 模式（应答模式=0x1），则收完一个报文后继续等待下一个。
// 【通信协议 LR001 P.8】起始应答报文结构图表2-7。
// 【通信协议 LR001 P.5】单次请求-多次应答模式（扫描数据连续接收）。
//   buffer - 接收到的数据指针
//   size   - 数据长度（字节数）
void   RPLidarProtocolCodec::onDecodeData(const void* buffer, size_t size)
{
    // 加锁保护解码状态机（接收线程/解码线程可能并发访问）。
    rp::hal::AutoLocker autolock(_op_locker);

    // 将输入数据视为字节流，data 指向当前处理位置，dataEnd 指向末尾。
    const _u8* data = reinterpret_cast<const _u8*>(buffer);
    const _u8* dataEnd = data + size;


    // 逐字节驱动状态机，直到处理完本批所有数据。
    while (data != dataEnd) {
        _u8 currentByte = *data;
        ++data;

        // 取状态机的低有效位（屏蔽 bit31 的 LOOP_MODE_FLAG），判断当前处于哪个解码阶段。
        switch (_working_states & ((_u32)STATUS_LOOP_MODE_FLAG - 1)) {
        // 状态1：等待起始标志1(0xA5)。
        // 【通信协议 LR001 P.8】起始标志1 为 1byte(0xA5)。
        case STATUS_WAIT_SYNC1:
            if (currentByte == RPLIDAR_ANS_SYNC_BYTE1) {
                // 收到 0xA5，转入等待起始标志2。
                _working_states = STATUS_WAIT_SYNC2;
            }
            // 未匹配则停留在本状态继续寻找。
            break;
        // 状态2：等待起始标志2(0x5A)。
        // 【通信协议 LR001 P.8】起始标志为2个字节固定数据：0xA5 0x5A。
        case STATUS_WAIT_SYNC2:
            if (currentByte == RPLIDAR_ANS_SYNC_BYTE2) {
                // 收到 0x5A，确认报文开始，转入接收 size_q30_subtype 字段。
                _working_states = STATUS_WAIT_SIZE_FLAG;
                // 初始化接收计数器，用于收集4字节的 size_q30_subtype。
                _rx_pos = 0; // init rx pos for recv size and flag
            }
            else {
                // 未收到 0x5A，说明前一个 0xA5 并非报文起始，回退到初始状态重新寻找。
                // reset to the initial state
                _working_states = STATUS_WAIT_SYNC1;
            }
            break;
        // 状态3：接收 size_q30_subtype 字段（4字节，小端）。
        // 【通信协议 LR001 P.8】数据应答报文长度(30bits) + 应答模式(2bits)。
        case STATUS_WAIT_SIZE_FLAG:
        {
            // 断言 len 字段至少4字节，用于暂存接收到的4字节 size_q30_subtype。
            assert(sizeof(_decodingMessage.len) >= 4);
            // 逐字节填入 len 字段的内存（按小端序逐字节收集）。
            _u8* byteArr = reinterpret_cast<_u8*>(&_decodingMessage.len);
            byteArr[_rx_pos++] = currentByte;

            // 收满4字节后解析长度与应答模式。
            if (_rx_pos == 4) {
                // 转入接收数据类型字段。
                _working_states = STATUS_WAIT_TYPE;
                // 将小端的4字节转为CPU本地序的32位整数。
                _decodingMessage.len = le32_to_cpu(_decodingMessage.len);

                // 30bit size + 2bit flag has been received
                // 取高2位（右移30位）作为应答模式。
                // 【通信协议 LR001 P.8】2bits 应答模式：0x0单次、0x1多次、0x2/0x3保留。
                _u32 flagbits = (_u32)(_decodingMessage.len >> RPLIDAR_ANS_HEADER_SUBTYPE_SHIFT);
                if (flagbits & RPLIDAR_ANS_PKTFLAG_LOOP) {
                    // 应答模式为 LOOP(0x1)，置位多次应答标志。
                    // 【通信协议 LR001 P.5】单次请求-多次应答模式（扫描测距）。
                    _working_states |= STATUS_LOOP_MODE_FLAG;
                }
                // 取低30位作为数据应答报文长度（字节数）。
                // 【通信协议 LR001 P.8】数据应答报文长度为30bits，记录随后发送的单个数据应答报文长度。
                _decodingMessage.len = (_decodingMessage.len & RPLIDAR_ANS_HEADER_SIZE_MASK);
                // 预分配负载缓冲区（按解析出的长度）。
                // alloc buffer
                _decodingMessage.fillData(NULL, _decodingMessage.getPayloadSize());
                // 重置接收计数器，准备接收数据类型字段。
                _rx_pos = 0;
            }
        }
        break;
        // 状态4：接收数据类型字段（1字节）。
        // 【通信协议 LR001 P.8】数据类型1byte，表示数据应答报文发送内容的类型。
        case STATUS_WAIT_TYPE:
            // 将数据类型保存到消息的 cmd 字段（复用 cmd 字段存放 type）。
            // save the type field as a cmd 
            _decodingMessage.cmd = currentByte;

            // 转入接收负载状态，保留 LOOP_MODE_FLAG（若已置位）。
            // recv payload...
            _working_states = (_working_states & STATUS_LOOP_MODE_FLAG)
                | STATUS_RECV_PAYLOAD;

            // 若负载长度为0（零负载报文），直接完成解码。
            if (!_decodingMessage.getPayloadSize()) {
                // zero payload packet? 
                // 负载为0，重置为等待下一个报文起始（非LOOP模式则回到SYNC1）。
                _working_states = STATUS_WAIT_SYNC1;
            }
            break;
        // 状态5：接收负载数据，按 size_q30_subtype 低30位指定的长度逐字节接收。
        case STATUS_RECV_PAYLOAD:
            // 将当前字节写入负载缓冲区对应位置。
            _decodingMessage.getDataBuf()[_rx_pos++] = currentByte;

            // 负载接收完整。
            if ((size_t)_rx_pos == _decodingMessage.getPayloadSize()) {
                if (_working_states & STATUS_LOOP_MODE_FLAG) {
                    // 多次应答模式：收完一个数据应答报文后，重置接收计数器，
                    // 保持 LOOP 标志，回到 WAIT_SYNC1 继续等待下一个报文。
                    // 【通信协议 LR001 P.5】RPLIDAR 将连续发送多个应答数据报文。
                    // rewind to the payload recv status in loop mode
                    _rx_pos = 0;
                }
                else {
                    // 单次应答模式：收完唯一一个数据应答报文后，重置解码器。
                    // reset the decoder
                    _working_states = STATUS_WAIT_SYNC1;
                }

                // 缓存监听器指针（防止回调期间被修改）。
                IProtocolMessageListener* cachedLister = _listener;

                // 解锁操作锁，防止回调中再次访问本解码器导致死锁。
                // unlock the oplock to prevent deadlock
                autolock.forceUnlock(); //unlock the oplock to prevent deadlock


                // 通过监听器回调通知上层：一条完整报文已解码完成。
                if (cachedLister) {
                    cachedLister->onProtocolMessageDecoded(_decodingMessage);
                }

                // 回调返回后重新加锁，继续后续字节处理。
                _op_locker.lock(); // relock it
            }
            break;
        }

    }
}




}}
