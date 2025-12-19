#ifndef RS485_COMM_H
#define RS485_COMM_H

#include "driver/uart.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// RS485 命令类型枚举
typedef enum {
  RS485_CMD_RED_ON = 0x11,             // 红灯常亮
  RS485_CMD_YELLOW_ON = 0x12,          // 黄灯常亮
  RS485_CMD_GREEN_ON = 0x13,           // 绿灯常亮
  RS485_CMD_RED_SLOW_FLASH = 0x21,     // 红灯慢闪
  RS485_CMD_YELLOW_SLOW_FLASH = 0x22,  // 黄灯慢闪
  RS485_CMD_GREEN_SLOW_FLASH = 0x23,   // 绿灯慢闪
  RS485_CMD_RED_BURST_FLASH = 0x31,    // 红灯爆闪
  RS485_CMD_YELLOW_BURST_FLASH = 0x32, // 黄灯爆闪
  RS485_CMD_GREEN_BURST_FLASH = 0x33,  // 绿灯爆闪
  RS485_CMD_LIGHT_OFF = 0x60           // 警灯关闭
} rs485_cmd_t;

/**
 * @brief 初始化 RS485 通信
 * @param uart_num UART 端口号
 * @param tx_pin TX 引脚
 * @param rx_pin RX 引脚
 * @param baud_rate 波特率
 * @return true 成功, false 失败
 */
bool rs485_init(uart_port_t uart_num, int tx_pin, int rx_pin, int baud_rate);

/**
 * @brief 计算 CRC16 校验码 (Modbus CRC16)
 * @param data 数据缓冲区
 * @param length 数据长度
 * @return CRC16 校验值
 */
uint16_t rs485_calculate_crc16(const uint8_t *data, uint16_t length);

/**
 * @brief 发送 RS485 命令
 * @param cmd 命令类型
 * @return true 成功, false 失败
 */
bool rs485_send_command(rs485_cmd_t cmd);

/**
 * @brief 发送原始 RS485 数据
 * @param data 数据缓冲区
 * @param length 数据长度
 * @return true 成功, false 失败
 */
bool rs485_send_data(const uint8_t *data, size_t length);

/**
 * @brief 反初始化 RS485 通信
 */
void rs485_deinit(void);

/**
 * @brief 查询在线设备
 * @return true 成功, false 失败
 */
bool rs485_query_devices(void);

/**
 * @brief 接收 RS485 数据（带超时）
 * @param buffer 接收缓冲区
 * @param buffer_size 缓冲区大小
 * @param timeout_ms 超时时间（毫秒）
 * @return 接收到的字节数，-1 表示错误
 */
int rs485_receive_data(uint8_t *buffer, size_t buffer_size,
                       uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif // RS485_COMM_H
