/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX‑License‑Identifier: LGPL‑2.1‑or‑later
 */
#ifndef MODBUS_TCP_H
#define MODBUS_TCP_H

#include "modbus.h"   // 公共modbus基础头，读写、释放接口都在这里

MODBUS_BEGIN_DECLS  // C/C++混合编译兼容宏，防止C++对函数名做名字改编

// Windows平台适配：Windows socket错误码和标准POSIX不统一，做宏映射对齐
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <winsock2.h>
#if !defined(ECONNRESET)
#define ECONNRESET WSAECONNRESET   // 对端复位断开连接
#endif
#if !defined(ECONNREFUSED)
#define ECONNREFUSED WSAECONNREFUSED // 端口未开放，连接被拒绝
#endif
#if !defined(ETIMEDOUT)
#define ETIMEDOUT WSAETIMEDOUT     // 通信超时
#endif
#if !defined(ENOPROTOOPT)
#define ENOPROTOOPT WSAENOPROTOOPT
#endif
#if !defined(EINPROGRESS)
#define EINPROGRESS WSAEINPROGRESS // 非阻塞socket操作正在进行
#endif
#endif

#define MODBUS_TCP_DEFAULT_PORT 502   // Modbus‑TCP标准端口502
#define MODBUS_TCP_SLAVE        0xFF  // TCP协议MBAP头固定单元ID 0XFF，上层读写依旧填真实从站号

/*
 * Modbus官方协议文档V1.1b
 * ADU = MBAP头7字节 + PDU最大253字节，总长度最大260字节
 */
#define MODBUS_TCP_MAX_ADU_LENGTH 260

 /**
  * @brief 创建Modbus‑TCP主站(客户端)上下文对象
  * @param ip_address 从站设备IP字符串，例:"192.168.1.10"
  * @param port 端口，一般用 MODBUS_TCP_DEFAULT_PORT(502)
  * @return modbus_t* 成功返回上下文；失败NULL
  * @note 仅创建对象，**不会发起网络连接**；调用modbus_connect()才真正TCP连接
  * @note 上位机主站最常用接口
  */
	MODBUS_API modbus_t* modbus_new_tcp(const char* ip_address, int port);

/**
 * @brief TCP服务端(从站)：开启socket监听，等待外部客户端接入
 * @param ctx modbus上下文
 * @param nb_connection 最大支持并发连接数量
 * @return 成功返回监听socket fd；失败返回‑1
 * @note 上位机主站不用仅 做modbus模拟器/从站程序才使用
 */
MODBUS_API int modbus_tcp_listen(modbus_t* ctx, int nb_connection);

/**
 * @brief TCP服务端(从站)：阻塞等待接收客户端连接
 * @param ctx modbus上下文
 * @param s [out]输出参数，返回新建立连接的socket句柄
 * @return 成功0，失败‑1
 * @note 上位机主站不用-配合listen使用
 */
MODBUS_API int modbus_tcp_accept(modbus_t* ctx, int* s);

/**
 * @brief 创建TCP主站上下文，支持IPv4/IPv6双栈(域名解析)
 * @param node IP/域名；service端口字符串，例:"502"
 * @return modbus_t* 成功返回上下文；失败NULL
 * @note 普通设备直连优先用modbus_new_tcp；需要域名/IPv6场景选用
 */
MODBUS_API modbus_t* modbus_new_tcp_pi(const char* node, const char* service);

/**
 * @brief IPv6版本服务端监听接口
 * @note 【上位机主站不用】从站模拟器使用
 */
MODBUS_API int modbus_tcp_pi_listen(modbus_t* ctx, int nb_connection);

/**
 * @brief IPv6版本服务端接收连接接口
 * @note 【上位机主站不用】从站模拟器使用
 */
MODBUS_API int modbus_tcp_pi_accept(modbus_t* ctx, int* s);

MODBUS_END_DECLS  // C/C++混合编译结束宏

#endif /* MODBUS_TCP_H */
