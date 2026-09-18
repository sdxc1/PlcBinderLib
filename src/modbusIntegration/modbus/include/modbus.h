/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX‑License‑Identifier: LGPL‑2.1‑or‑later
 */
#ifndef MODBUS_H
#define MODBUS_H

 // clang-format off
 /* Unix系统相关头文件兼容 */
#if (defined(__unix__) || defined(unix)) && !defined(USG)
# include <sys/param.h>
#endif

// 跨平台stdint，MSVC老版本没有自带stdint.h
#ifndef _MSC_VER
# include <stdint.h>
#else
# include "stdint.h"
#endif

#include "modbus-version.h"

#if defined(_MSC_VER)
# define MODBUS_API
#else
# define MODBUS_API
#endif

#ifdef  __cplusplus
// C++调用C库：防止C++名字修饰，保证符号链接正确
# define MODBUS_BEGIN_DECLS  extern "C" {
# define MODBUS_END_DECLS    }
#else
# define MODBUS_BEGIN_DECLS
# define MODBUS_END_DECLS
#endif
// clang-format on

MODBUS_BEGIN_DECLS

// 基础布尔宏定义
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef OFF
#define OFF 0
#endif
#ifndef ON
#define ON 1
#endif

/* ===================== Modbus标准功能码 ===================== */
#define MODBUS_FC_READ_COILS               0x01      // 读线圈（DO，位）
#define MODBUS_FC_READ_DISCRETE_INPUTS     0x02      // 读离散输入（DI，位）
#define MODBUS_FC_READ_HOLDING_REGISTERS   0x03      // 读保持寄存器（最常用，读写参数）
#define MODBUS_FC_READ_INPUT_REGISTERS     0x04      // 读输入寄存器（只读）
#define MODBUS_FC_WRITE_SINGLE_COIL        0x05      // 写单个线圈
#define MODBUS_FC_WRITE_SINGLE_REGISTER    0x06      // 写单个保持寄存器
#define MODBUS_FC_READ_EXCEPTION_STATUS    0x07      // 读异常状态
#define MODBUS_FC_WRITE_MULTIPLE_COILS     0x0F      // 写多组线圈
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS 0x10      // 写多保持寄存器
#define MODBUS_FC_REPORT_SLAVE_ID          0x11      // 读取从站设备信息
#define MODBUS_FC_MASK_WRITE_REGISTER      0x16      // 掩码写寄存器
#define MODBUS_FC_WRITE_AND_READ_REGISTERS 0x17      // 读写寄存器（一次请求同时读+写）

#define MODBUS_BROADCAST_ADDRESS 0     // RTU广播地址，所有从站接收，无应答返回

/* ===================== 协议单次请求最大数量限制（协议硬限制） ===================== */
// 读线圈最大数量2000;写线圈最大1968
#define MODBUS_MAX_READ_BITS  2000
#define MODBUS_MAX_WRITE_BITS 1968

// 寄存器限制:读最多125;批量写最多123;读写复合写最多121
#define MODBUS_MAX_READ_REGISTERS     125
#define MODBUS_MAX_WRITE_REGISTERS    123
#define MODBUS_MAX_WR_WRITE_REGISTERS 121
#define MODBUS_MAX_WR_READ_REGISTERS  125

#define MODBUS_MAX_PDU_LENGTH 253       // PDU协议数据单元最大长度
#define MODBUS_MAX_ADU_LENGTH 260       // ADU完整帧最大，RTU/TCP通用，用于报文缓冲区分配

#define MODBUS_ENOBASE 112345678        // libmodbus自定义错误码基准偏移，避免与系统errno冲突

/* ===================== Modbus协议异常码（从站返回错误） ===================== */
enum {
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 0x01,        // 非法功能码
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,           // 寄存器地址非法
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,              // 寄存器数值非法
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE,         // 从站设备故障
    MODBUS_EXCEPTION_ACKNOWLEDGE,                     // 确认
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY,           // 从站忙，请稍后重试
    MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE,            // 否定确认
    MODBUS_EXCEPTION_MEMORY_PARITY,                   // 内存奇偶校验错误
    MODBUS_EXCEPTION_NOT_DEFINED,
    MODBUS_EXCEPTION_GATEWAY_PATH,                    // 网关路径不可用
    MODBUS_EXCEPTION_GATEWAY_TARGET,                  // 网关目标设备无响应
    MODBUS_EXCEPTION_MAX
};
// 将协议异常码转为libmodbus内部错误号
#define EMBXILFUN  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_FUNCTION)
#define EMBXILADD  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS)
#define EMBXILVAL  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE)
#define EMBXSFAIL  (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE)
#define EMBXACK    (MODBUS_ENOBASE + MODBUS_EXCEPTION_ACKNOWLEDGE)
#define EMBXSBUSY  (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY)
#define EMBXNACK   (MODBUS_ENOBASE + MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE)
#define EMBXMEMPAR (MODBUS_ENOBASE + MODBUS_EXCEPTION_MEMORY_PARITY)
#define EMBXGPATH  (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_PATH)
#define EMBXGTAR   (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_TARGET)

/* libmodbus自定义底层错误码 */
#define EMBBADCRC   (EMBXGTAR + 1)     // CRC校验错误（RTU）
#define EMBBADDATA  (EMBXGTAR + 2)     // 收到非法数据
#define EMBBADEXC   (EMBXGTAR + 3)     // modbus协议异常
#define EMBUNKEXC   (EMBXGTAR + 4)     // 未知异常
#define EMBMDATA    (EMBXGTAR + 5)     // 数据缺失
#define EMBBADSLAVE (EMBXGTAR + 6)     // 非法从站地址

// 库版本号
extern const unsigned int libmodbus_version_major;
extern const unsigned int libmodbus_version_minor;
extern const unsigned int libmodbus_version_micro;

typedef struct _modbus modbus_t;   // modbus上下文句柄，不直接访问内部成员

/**
 * @brief modbus从站内存映射结构体，【做slave从站模拟器才用，上位机主站不用】
 * 保存线圈、离散输入、输入寄存器、保持寄存器的内存缓冲区
 */
typedef struct _modbus_mapping_t {
    int nb_bits;
    int start_bits;
    int nb_input_bits;
    int start_input_bits;
    int nb_input_registers;
    int start_input_registers;
    int nb_registers;
    int start_registers;
    uint8_t* tab_bits;
    uint8_t* tab_input_bits;
    uint16_t* tab_input_registers;
    uint16_t* tab_registers;
} modbus_mapping_t;

/**
 * @brief 错误恢复模式，通信出错自动重试策略
 */
typedef enum {
    MODBUS_ERROR_RECOVERY_NONE = 0,                 // 关闭自动恢复，出错直接返回
    MODBUS_ERROR_RECOVERY_LINK = (1 << 1),           // 链路层错误自动恢复
    MODBUS_ERROR_RECOVERY_PROTOCOL = (1 << 2)       // 协议层错误自动恢复
} modbus_error_recovery_mode;

/**
 * @brief 特殊兼容标记，适配部分非标bug设备
 */
typedef enum {
    MODBUS_QUIRK_NONE = 0,
    MODBUS_QUIRK_MAX_SLAVE = (1 << 1),
    MODBUS_QUIRK_REPLY_TO_BROADCAST = (1 << 2),
    MODBUS_QUIRK_ALL = 0xFF
} modbus_quirks;


/**
 * @brief 设置本次通信目标从站ID
 * @param ctx modbus上下文
 * @param slave 从站号1‑247；填0为广播
 * @return 成功0，失败‑1
 * @note 每次收发前可切换从站号，不用重建ctx
 */
MODBUS_API int modbus_set_slave(modbus_t* ctx, int slave);
MODBUS_API int modbus_get_slave(modbus_t* ctx);

/**
 * @brief 设置错误自动恢复策略，断线/异常自动重试
 */
MODBUS_API int modbus_set_error_recovery(modbus_t* ctx,
    modbus_error_recovery_mode error_recovery);

// 内部socket句柄设置获取，极少使用
MODBUS_API int modbus_set_socket(modbus_t* ctx, int s);
MODBUS_API int modbus_get_socket(modbus_t* ctx);

/**
 * @brief 设置应答超时时间（最重要！阻塞读写等待从站回复最大时间）
 * @param to_sec 秒；to_usec 微秒
 */
MODBUS_API int
modbus_get_response_timeout(modbus_t* ctx, uint32_t* to_sec, uint32_t* to_usec);
MODBUS_API int
modbus_set_response_timeout(modbus_t* ctx, uint32_t to_sec, uint32_t to_usec);

/**
 * @brief 字节间隔超时：两个字节到达之间最大间隔，RTU常用，检测帧结束
 */
MODBUS_API int
modbus_get_byte_timeout(modbus_t* ctx, uint32_t* to_sec, uint32_t* to_usec);
MODBUS_API int modbus_set_byte_timeout(modbus_t* ctx, uint32_t to_sec, uint32_t to_usec);

MODBUS_API int
modbus_get_indication_timeout(modbus_t* ctx, uint32_t* to_sec, uint32_t* to_usec);
MODBUS_API int
modbus_set_indication_timeout(modbus_t* ctx, uint32_t to_sec, uint32_t to_usec);

MODBUS_API int modbus_get_header_length(modbus_t* ctx);

/**
 * @brief 打开链路（TCP发起连接 / RTU打开串口）
 * @note new_tcp/new_rtu只创建对象，connect才真正打开硬件链路
 * @return 成功0，失败‑1
 */
MODBUS_API int modbus_connect(modbus_t* ctx);

/**
 * @brief 关闭链路（关闭socket / 关闭串口，不释放对象内存）
 */
MODBUS_API void modbus_close(modbus_t* ctx);

/**
 * @brief 释放modbus_t上下文对象内存，close之后调用
 */
MODBUS_API void modbus_free(modbus_t* ctx);

/**
 * @brief 清空接收缓冲区，丢弃残留字节
 */
MODBUS_API int modbus_flush(modbus_t* ctx);

/**
 * @brief 开启调试打印，1开启，0关闭；会把收发原始报文打印到stdout
 */
MODBUS_API int modbus_set_debug(modbus_t* ctx, int flag);

/**
 * @brief 将错误码转为可读错误字符串，类似strerror()
 */
MODBUS_API const char* modbus_strerror(int errnum);

//====================【主站读写核心阻塞API】====================
/**
 * @brief 读线圈(0x01)
 * @param addr 起始位地址；nb读取数量；dest输出缓存
 * @return >0成功返回读到位数；‑1失败
 */
MODBUS_API int modbus_read_bits(modbus_t* ctx, int addr, int nb, uint8_t* dest);

/**
 * @brief 读离散输入(0x02)
 */
MODBUS_API int modbus_read_input_bits(modbus_t* ctx, int addr, int nb, uint8_t* dest);

/**
 * @brief 读保持寄存器0x03 上位机最常用！伺服PLC参数读取
 * @param addr寄存器起始；nb寄存器数量；dest输出uint16数组
 * @return >0成功返回读到寄存器数量；‑1失败
 */
MODBUS_API int modbus_read_registers(modbus_t* ctx, int addr, int nb, uint16_t* dest);

/**
 * @brief 读输入寄存器0x04（只读）
 */
MODBUS_API int
modbus_read_input_registers(modbus_t* ctx, int addr, int nb, uint16_t* dest);

/**
 * @brief 写单个线圈 0x05
 * @param coil_addr线圈地址；status TRUE/FALSE 通断
 */
MODBUS_API int modbus_write_bit(modbus_t* ctx, int coil_addr, int status);

/**
 * @brief 写单个保持寄存器0x06
 * @param reg_addr寄存器地址；value写入的16位数值
 */
MODBUS_API int modbus_write_register(modbus_t* ctx, int reg_addr, const uint16_t value);

/**
 * @brief 批量写线圈0x0F
 */
MODBUS_API int modbus_write_bits(modbus_t* ctx, int addr, int nb, const uint8_t* data);

/**
 * @brief 批量写保持寄存器0x10【上位机常用，写多组伺服参数】
 */
MODBUS_API int
modbus_write_registers(modbus_t* ctx, int addr, int nb, const uint16_t* data);

/**
 * @brief 掩码写寄存器0x16
 */
MODBUS_API int modbus_mask_write_register(modbus_t* ctx, int addr, uint16_t and_mask, uint16_t or_mask);

/**
 * @brief 读写寄存器 0x17，一条请求同时写+读
 */
MODBUS_API int
modbus_write_and_read_registers(modbus_t* ctx,
    int write_addr,
    int write_nb,
    const uint16_t* src,
    int read_addr,
    int read_nb,
    uint16_t* dest);

/**
 * @brief 获取从站ID信息0x11
 */
MODBUS_API int modbus_report_slave_id(modbus_t* ctx, int max_dest, uint8_t* dest);

//=========下面API全部是【做modbus从站模拟器】使用，普通上位机主站不用 =========
MODBUS_API modbus_mapping_t*
modbus_mapping_new_start_address(unsigned int start_bits,
    unsigned int nb_bits,
    unsigned int start_input_bits,
    unsigned int nb_input_bits,
    unsigned int start_registers,
    unsigned int nb_registers,
    unsigned int start_input_registers,
    unsigned int nb_input_registers);
MODBUS_API modbus_mapping_t* modbus_mapping_new(int nb_bits,
    int nb_input_bits,
    int nb_registers,
    int nb_input_registers);
MODBUS_API void modbus_mapping_free(modbus_mapping_t* mb_mapping);

MODBUS_API int
modbus_send_raw_request(modbus_t* ctx, const uint8_t* raw_req, int raw_req_length);
MODBUS_API int modbus_receive(modbus_t* ctx, uint8_t* req);
MODBUS_API int modbus_receive_confirmation(modbus_t* ctx, uint8_t* rsp);
MODBUS_API int modbus_reply(modbus_t* ctx,
    const uint8_t* req,
    int req_length,
    modbus_mapping_t* mb_mapping);
MODBUS_API int
modbus_reply_exception(modbus_t* ctx, const uint8_t* req, unsigned int exception_code);
MODBUS_API int modbus_enable_quirks(modbus_t* ctx, unsigned int quirks_mask);
MODBUS_API int modbus_disable_quirks(modbus_t* ctx, unsigned int quirks_mask);

//====================工具宏与工具函数（字节序转换、浮点数转换，非常实用）====================
// 取16位数据高低字节
#define MODBUS_GET_HIGH_BYTE(data) (((data) >> 8) & 0xFF)
#define MODBUS_GET_LOW_BYTE(data)  ((data) &0xFF)

// 从uint16寄存器数组组装64/32位整数；注意modbus大端，部分设备字节序颠倒
#define MODBUS_GET_INT64_FROM_INT16(tab_int16, index)                                \
  (((int64_t) tab_int16[(index)] << 48) | ((int64_t) tab_int16[(index) + 1] << 32) | \
   ((int64_t) tab_int16[(index)] << 16) | (int64_t) tab_int16[(index) + 3])

#define MODBUS_GET_INT32_FROM_INT16(tab_int16, index) \
  (((int32_t) tab_int16[(index)] << 16) | (int32_t) tab_int16[(index) + 1])

#define MODBUS_GET_INT16_FROM_INT8(tab_int8, index) \
  (((int16_t) tab_int8[(index)] << 8) | (int16_t) tab_int8[(index) + 1])

// 将整数拆字节存入数组
#define MODBUS_SET_INT16_TO_INT8(tab_int8, index, value)        \
  do {                                                          \
    ((int8_t *) (tab_int8))[(index)] = (int8_t) ((value) >> 8); \
    ((int8_t *) (tab_int8))[(index) + 1] = (int8_t) (value);    \
  } while (0)
#define MODBUS_SET_INT32_TO_INT16(tab_int16, index, value)          \
  do {                                                              \
    ((int16_t *) (tab_int16))[(index)] = (int16_t) ((value) >> 16); \
    ((int16_t *) (tab_int16))[(index) + 1] = (int16_t) ((value));     \
  } while (0)
#define MODBUS_SET_INT64_TO_INT16(tab_int16, index, value)              \
  do {                                                                  \
    ((int16_t *) (tab_int16))[(index)] = (int16_t) ((value) >> 48);     \
    ((int16_t *) (tab_int16))[(index) + 1] = (int16_t) ((value) >> 32); \
    ((int16_t *) (tab_int16))[(index) + 2] = (int16_t) ((value) >> 16); \
    ((int16_t *) (tab_int16))[(index) + 3] = (int16_t) ((value));         \
  } while (0)

// 位操作工具函数
MODBUS_API void modbus_set_bits_from_byte(uint8_t* dest, int idx, const uint8_t value);
MODBUS_API void modbus_set_bits_from_bytes(uint8_t* dest,
    int idx,
    unsigned int nb_bits,
    const uint8_t* tab_byte);
MODBUS_API uint8_t modbus_get_byte_from_bits(const uint8_t* src,
    int idx,
    unsigned int nb_bits);

/**
 * @brief float浮点数转换，4种字节序abcd/dcba/badc/cdab，工控设备字节序五花八门！
 * modbus标准是abcd，很多伺服PLC是dcba，读出来数值乱码就要换其他接口
 */
MODBUS_API float modbus_get_float(const uint16_t* src);
MODBUS_API float modbus_get_float_abcd(const uint16_t* src);
MODBUS_API float modbus_get_float_dcba(const uint16_t* src);
MODBUS_API float modbus_get_float_badc(const uint16_t* src);
MODBUS_API float modbus_get_float_cdab(const uint16_t* src);

MODBUS_API void modbus_set_float(float f, uint16_t* dest);
MODBUS_API void modbus_set_float_abcd(float f, uint16_t* dest);
MODBUS_API void modbus_set_float_dcba(float f, uint16_t* dest);
MODBUS_API void modbus_set_float_badc(float f, uint16_t* dest);
MODBUS_API void modbus_set_float_cdab(float f, uint16_t* dest);

// 自动引入rtu、tcp头文件
#include "modbus-rtu.h"
#include "modbus-tcp.h"

MODBUS_END_DECLS
#endif /* MODBUS_H */
