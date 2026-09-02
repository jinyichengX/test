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
// rplidar_driver.cpp — 旧版 RPLIDAR 驱动兼容包装器
//
// 本文件是旧版 API (rp::standalone::rplidar 命名空间) 到新版 API
// (sl:: 命名空间) 的兼容适配层。所有方法均直接委托给 SL_LidarDriver。
//
// 旧版 API 采用 RPlidarDriver 类，新版 API 采用 SL_LidarDriver 类。
// 新项目应直接使用 sl::SL_LidarDriver，本文件仅供向后兼容。
//
// 工作原理：
//   connect() → 根据 channelType 创建 IChannel (串口/TCP/UDP)
//            → 创建 SL_LidarDriver 并通过它连接雷达
//   所有其他方法 → 直接调用 _lidarDrv 的同名方法
//
// 通信协议关系：
//   【通信协议 LR001】本文件不直接处理协议编解码，相关逻辑在新版
//   SL_LidarDriver 和 sl_lidarprotocol_codec 中实现。
// ===================================================================

#include "sdkcommon.h"
#include "hal/abs_rxtx.h"
#include "hal/thread.h"
#include "hal/types.h"
#include "hal/assert.h"
#include "hal/locker.h"
#include "hal/socket.h"
#include "hal/event.h"
#include "rplidar_driver.h"
#include "sl_crc.h"
#include <algorithm>

namespace rp { namespace standalone{ namespace rplidar {

    // 默认构造函数 — 不初始化通道类型，需后续调用 connect
    RPlidarDriver::RPlidarDriver(){}

    // 指定通道类型的构造函数
    // channelType: CHANNEL_TYPE_SERIALPORT(串口) / CHANNEL_TYPE_TCP / CHANNEL_TYPE_UDP
    RPlidarDriver::RPlidarDriver(sl_u32 channelType)
        :_channelType(channelType)
    {
    }

    RPlidarDriver::~RPlidarDriver() {}

    // 工厂方法：创建驱动实例
    // drivertype 即通道类型，决定 connect() 时创建哪种 IChannel
    RPlidarDriver * RPlidarDriver::CreateDriver(_u32 drivertype)
    {
        return  new RPlidarDriver(drivertype);
    }

    // 工厂方法：销毁驱动实例
    void RPlidarDriver::DisposeDriver(RPlidarDriver * drv)
    {
        delete drv;
    }

    // 连接雷达设备 — 根据 _channelType 创建对应通信通道
    // path: 串口路径或IP地址; portOrBaud: 波特率或端口号
    // 【通信协议 LR001 P.3】RPLIDAR 通过 UART/TTL 与外部系统通讯
    u_result RPlidarDriver::connect(const char *path, _u32 portOrBaud, _u32 flag)
    {
        switch (_channelType)
        {
        case CHANNEL_TYPE_SERIALPORT:
            _channel = (*createSerialPortChannel(path, portOrBaud));
            break;
        case CHANNEL_TYPE_TCP:
            _channel = *createTcpChannel(path, portOrBaud);
            break;
        case CHANNEL_TYPE_UDP:
            _channel = *createUdpChannel(path, portOrBaud);
            break;
        }
        if (!(bool)_channel) return SL_RESULT_OPERATION_FAIL;

        _lidarDrv = *createLidarDriver();

        if (!(bool)_lidarDrv) return SL_RESULT_OPERATION_FAIL;

        sl_result ans =(_lidarDrv)->connect(_channel);
        return ans;
    }

    // 断开连接 — 委托给 SL_LidarDriver
    void RPlidarDriver::disconnect()
    {
        (_lidarDrv)->disconnect();
    }

    // 查询是否已连接
    bool RPlidarDriver::isConnected()
    {
        return (_lidarDrv)->isConnected();
    }

    // 软重启 — 【通信协议 LR001 P.13】RESET(0x40) 命令，无应答，延时≥2ms
    u_result RPlidarDriver::reset(_u32 timeout)
    {
        return (_lidarDrv)->reset();
    }

    // 获取所有支持的扫描模式 — 内部通过 GET_LIDAR_CONF(0x84) 获取
    // 【通信协议 LR001 P.36】配置字段 0x70(SCAN_MODE_COUNT) 获取模式数量
    u_result RPlidarDriver::getAllSupportedScanModes(std::vector<RplidarScanMode>& outModes, _u32 timeoutInMs)
    {
        return (_lidarDrv)->getAllSupportedScanModes(outModes, timeoutInMs);
    }

    // 获取典型扫描模式 — 【通信协议 LR001 P.38】配置字段 0x7C(SCAN_MODE_TYPICAL)
    u_result RPlidarDriver::getTypicalScanMode(_u16& outMode, _u32 timeoutInMs)
    {
        return (_lidarDrv)->getTypicalScanMode(outMode, timeoutInMs);
    }

    // 开始扫描 — useTypicalScan=true 时使用典型模式, 否则使用标准 SCAN(0x20)
    // 【通信协议 LR001 P.14-16】SCAN 命令, 多次应答, 每点5字节(angle_q6+distance_q2)
    // useTypicalScan=false 时使用 EXPRESS_SCAN(0x82) 高速采样
    // 【通信协议 LR001 P.17-28】EXPRESS_SCAN 有三种版本: 传统(84B)/扩展(132B)/密实(84B)
    u_result RPlidarDriver::startScan(bool force, bool useTypicalScan, _u32 options, RplidarScanMode* outUsedScanMode)
    {
        return (_lidarDrv)->startScan(force, useTypicalScan, options, outUsedScanMode);
    }

    // 开始高速扫描 — scanMode 指定工作模式编号
    // 【通信协议 LR001 P.17】EXPRESS_SCAN(0x82), 负载5字节(working_mode+Reserved×4)
    u_result RPlidarDriver::startScanExpress(bool force, _u16 scanMode, _u32 options, RplidarScanMode* outUsedScanMode, _u32 timeout)
    {
        return (_lidarDrv)->startScanExpress(force, scanMode, options, outUsedScanMode, timeout);
    }

    // 获取健康状态 — 【通信协议 LR001 P.31】GET_HEALTH(0x52), 单次应答, 3字节(status+error_code)
    u_result RPlidarDriver::getHealth(rplidar_response_device_health_t & health, _u32 timeout)
    {
        return (_lidarDrv)->getHealth(health, timeout);
    }

    // 获取设备信息 — 【通信协议 LR001 P.29-30】GET_INFO(0x50), 单次应答, 20字节
    u_result RPlidarDriver::getDeviceInfo(rplidar_response_device_info_t & info, _u32 timeout)
    {
        return (_lidarDrv)->getDeviceInfo(info, timeout);
    }

    // 设置电机PWM转速 — 对应 MOTOR_SPEED_CTRL(0xA8), 仅S系列
    // 【通信协议 LR001 P.39】MOTOR_SPEED_CTRL 负载2字节(RPM), 设为0进入空闲
    u_result RPlidarDriver::setMotorPWM(_u16 pwm)
    {
        return (_lidarDrv)->setMotorSpeed(pwm);
    }

    // 检查电机控制支持 — 返回设备是否支持电机转速控制
    u_result RPlidarDriver::checkMotorCtrlSupport(bool & support, _u32 timeout)
    {
        MotorCtrlSupport motorSupport;
        u_result ans = (_lidarDrv)->checkMotorCtrlSupport(motorSupport, timeout);
        if (motorSupport == MotorCtrlSupportNone)
            support = false;
        return ans;
    }

    // 设置雷达IP配置 — 用于网络连接的雷达
    u_result RPlidarDriver::setLidarIpConf(const rplidar_ip_conf_t& conf, _u32 timeout)
    {
        return (_lidarDrv)->setLidarIpConf(conf, timeout);
    }

    // 获取雷达IP配置
    u_result RPlidarDriver::getLidarIpConf(rplidar_ip_conf_t& conf, _u32 timeout)
    {
        return (_lidarDrv)->getLidarIpConf(conf, timeout);
    }

    // 获取设备MAC地址
    u_result RPlidarDriver::getDeviceMacAddr(_u8* macAddrArray, _u32 timeoutInMs)
    {
        return (_lidarDrv)->getDeviceMacAddr(macAddrArray, timeoutInMs);
    }

    // 停止扫描 — 【通信协议 LR001 P.13】STOP(0x25), 无应答, 延时≥1ms
    u_result RPlidarDriver::stop(_u32 timeout)
    {
        return (_lidarDrv)->stop(timeout);
    }

    // 获取一圈扫描数据(HQ格式) — 阻塞直到收到完整一圈或超时
    // HQ节点含: angle_z_q14(Q14角度) + dist_mm_q2(Q2距离) + quality + flag
    u_result RPlidarDriver::grabScanDataHq(rplidar_response_measurement_node_hq_t * nodebuffer, size_t & count, _u32 timeout)
    {
        return (_lidarDrv)->grabScanDataHq(nodebuffer, count, timeout);
    }

    // 对扫描数据进行升序排列(按角度排序)
    u_result RPlidarDriver::ascendScanData(rplidar_response_measurement_node_hq_t * nodebuffer, size_t count)
    {
        return (_lidarDrv)->ascendScanData(nodebuffer, count);
    }

    // 旧版按间隔获取扫描数据 — 不支持, 返回操作不支持
    u_result RPlidarDriver::getScanDataWithInterval(rplidar_response_measurement_node_t * nodebuffer, size_t & count)
    {
        return RESULT_OPERATION_NOT_SUPPORT;
    }

    // 按间隔获取HQ扫描数据 — 实时获取最新采样点
    u_result RPlidarDriver::getScanDataWithIntervalHq(rplidar_response_measurement_node_hq_t * nodebuffer, size_t & count)
    {
        return (_lidarDrv)->getScanDataWithIntervalHq(nodebuffer, count);
    }

    // 启动电机 — 使用默认转速
    // 【通信协议 LR001 P.39】通过 MOTOR_SPEED_CTRL(0xA8) 设置默认转速
    u_result RPlidarDriver::startMotor()
    {
        return (_lidarDrv)->setMotorSpeed(DEFAULT_MOTOR_SPEED);
    }
    // 停止电机 — 转速设为0, 雷达进入空闲
    u_result RPlidarDriver::stopMotor()
    {
        return (_lidarDrv)->setMotorSpeed(0);
    }

}}}
