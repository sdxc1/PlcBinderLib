/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX‑License‑Identifier: LGPL‑2.1‑or‑later
 */
#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "modbus.h"   // modbus公共基础头，读写寄存器、释放接口全部在这里

MODBUS_BEGIN_DECLS  // C/C++混合编译兼容宏，防止C++改写C函数名

/*
 * Modbus协议文档：RS232/RS485‑RTU最大ADU帧长度
 * ADU = PDU253字节 + 从站地址1字节 + CRC校验2字节，合计256字节
 */
#define MODBUS_RTU_MAX_ADU_LENGTH 256

 /**
  * @brief 创建Modbus‑RTU(串口485/232)主站上下文对象
  * @param device 串口设备名 Windows填"COM3"；Linux填"/dev/ttyUSB0"
  * @param baud 波特率，如9600、115200
  * @param parity 校验位：'N'无校验，'O'奇校验，'E'偶校验
  * @param data_bit 数据位，常用8
  * @param stop_bit 停止位，常用1
  * @return modbus_t* 成功返回上下文；失败返回NULL
  * @note 只创建对象，**不打开串口**；调用modbus_connect(ctx)才真正打开串口
  * @note RTU上位机最核心接口
  */
    MODBUS_API modbus_t*
    modbus_new_rtu(const char* device, int baud, char parity, int data_bit, int stop_bit);

#define MODBUS_RTU_RS232  0   // 串口模式：RS232
#define MODBUS_RTU_RS485  1   // 串口模式：RS485（工控485半双工，绝大多数场景）
/**
 * @brief 设置串口硬件模式 RS232 / RS485
 * @param ctx modbus上下文
 * @param mode MODBUS_RTU_RS232 / MODBUS_RTU_RS485
 * @return 成功0，失败‑1
 */
MODBUS_API int modbus_rtu_set_serial_mode(modbus_t* ctx, int mode);
/**
 * @brief 获取当前串口硬件模式
 * @return 返回0(RS232)/1(RS485)；失败‑1
 */
MODBUS_API int modbus_rtu_get_serial_mode(modbus_t* ctx);

#define MODBUS_RTU_RTS_NONE  0   // 不操作RTS引脚
#define MODBUS_RTU_RTS_UP    1   // 发送前RTS置高，发送完毕拉低（485转换器常用）
#define MODBUS_RTU_RTS_DOWN  2   // 发送前RTS置低，发送完毕拉高
/**
 * @brief 设置RTS硬件流控模式，485半双工收发切换用
 * @param mode NONE / RTS_UP / RTS_DOWN
 * @return 成功0，失败‑1
 * @note USB转485大多不需要RTS；真正485板卡才需要配置收发切换
 */
MODBUS_API int modbus_rtu_set_rts(modbus_t* ctx, int mode);
/**
 * @brief 获取当前RTS配置模式
 */
MODBUS_API int modbus_rtu_get_rts(modbus_t* ctx);

/**
 * @brief 自定义RTS回调函数，自己接管485收发引脚电平控制
 * @param set_rts 回调函数指针，发送前后库会调用你来控制IO电平
 * @note 极少用，只有自己做485硬件电路才需要；普通USB转485不用
 */
MODBUS_API int modbus_rtu_set_custom_rts(modbus_t* ctx,
    void (*set_rts)(modbus_t* ctx, int on));

/**
 * @brief 设置RTS电平切换延时(微秒us)，收发切换等待时间，防止485时序错乱
 * @param us 微秒
 * @return 成功0，失败‑1
 */
MODBUS_API int modbus_rtu_set_rts_delay(modbus_t* ctx, int us);
/**
 * @brief 获取RTS切换延时
 */
MODBUS_API int modbus_rtu_get_rts_delay(modbus_t* ctx);

MODBUS_END_DECLS  // C/C++混合编译结束宏

#endif /* MODBUS_RTU_H */
