#include "rs485_comm.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "RS485";
static uart_port_t s_uart_num = UART_NUM_MAX;
static bool s_initialized = false;

// RS485 命令固定前缀 (根据图片描述: 01 06 00 C2 00)
#define RS485_CMD_PREFIX_0 0x01
#define RS485_CMD_PREFIX_1 0x06
#define RS485_CMD_PREFIX_2 0x00
#define RS485_CMD_PREFIX_3 0xC2
#define RS485_CMD_PREFIX_4 0x00

// 命令总长度（前缀5字节 + 命令1字节 + CRC2字节 + 固定后缀2字节00 00）
#define RS485_CMD_TOTAL_LENGTH 10

bool rs485_init(uart_port_t uart_num, int tx_pin, int rx_pin, int baud_rate) {
    if (s_initialized) {
        ESP_LOGW(TAG, "RS485 already initialized");
        return false;
    }

    ESP_LOGI(TAG, "=== RS485 Initialization Parameters ===");
    ESP_LOGI(TAG, "UART Port Number: UART_NUM_%d", uart_num);
    ESP_LOGI(TAG, "TX Pin: GPIO%d", tx_pin);
    ESP_LOGI(TAG, "RX Pin: GPIO%d", rx_pin);
    ESP_LOGI(TAG, "Baud Rate: %d", baud_rate);

    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_LOGI(TAG, "UART Configuration:");
    ESP_LOGI(TAG, "  Baud Rate: %d", uart_config.baud_rate);
    ESP_LOGI(TAG, "  Data Bits: %s", uart_config.data_bits == UART_DATA_5_BITS ? "5" :
                                      uart_config.data_bits == UART_DATA_6_BITS ? "6" :
                                      uart_config.data_bits == UART_DATA_7_BITS ? "7" :
                                      uart_config.data_bits == UART_DATA_8_BITS ? "8" :
                                      uart_config.data_bits == UART_DATA_BITS_MAX ? "MAX" : "UNKNOWN");
    ESP_LOGI(TAG, "  Parity: %s", uart_config.parity == UART_PARITY_DISABLE ? "DISABLE" : 
                                   uart_config.parity == UART_PARITY_EVEN ? "EVEN" : 
                                   uart_config.parity == UART_PARITY_ODD ? "ODD" : "UNKNOWN");
    ESP_LOGI(TAG, "  Stop Bits: %s", uart_config.stop_bits == UART_STOP_BITS_1 ? "1" :
                                      uart_config.stop_bits == UART_STOP_BITS_1_5 ? "1.5" :
                                      uart_config.stop_bits == UART_STOP_BITS_2 ? "2" : "UNKNOWN");
    ESP_LOGI(TAG, "  Flow Control: %s", uart_config.flow_ctrl == UART_HW_FLOWCTRL_DISABLE ? "DISABLE" :
                                          uart_config.flow_ctrl == UART_HW_FLOWCTRL_RTS ? "RTS" :
                                          uart_config.flow_ctrl == UART_HW_FLOWCTRL_CTS ? "CTS" :
                                          uart_config.flow_ctrl == UART_HW_FLOWCTRL_CTS_RTS ? "CTS_RTS" : "UNKNOWN");
    ESP_LOGI(TAG, "  Source Clock: %s", uart_config.source_clk == UART_SCLK_APB ? "APB" :
                                         uart_config.source_clk == UART_SCLK_RTC ? "RTC" :
                                         uart_config.source_clk == UART_SCLK_XTAL ? "XTAL" :
                                         uart_config.source_clk == UART_SCLK_DEFAULT ? "DEFAULT" : "UNKNOWN");

    // 中断分配标志（参照 uart_echo_example）
    int intr_alloc_flags = 0;
#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
    ESP_LOGI(TAG, "Interrupt Allocation: IRAM");
#else
    ESP_LOGI(TAG, "Interrupt Allocation: Default");
#endif

    ESP_LOGI(TAG, "UART Driver Install:");
    ESP_LOGI(TAG, "  RX Buffer Size: 1024 bytes");
    ESP_LOGI(TAG, "  TX Buffer Size: 1024 bytes");
    ESP_LOGI(TAG, "  Queue Size: 0 (no event queue)");
    ESP_LOGI(TAG, "  Interrupt Flags: 0x%X", intr_alloc_flags);

    esp_err_t ret = uart_driver_install(uart_num, 1024, 1024, 0, NULL, intr_alloc_flags);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver: %s", esp_err_to_name(ret));
        return false;
    }
    ESP_LOGI(TAG, "✓ UART driver installed successfully");

    ret = uart_param_config(uart_num, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART: %s", esp_err_to_name(ret));
        uart_driver_delete(uart_num);
        return false;
    }
    ESP_LOGI(TAG, "✓ UART parameters configured successfully");

    ret = uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins: %s", esp_err_to_name(ret));
        uart_driver_delete(uart_num);
        return false;
    }
    ESP_LOGI(TAG, "✓ UART pins set successfully");
    ESP_LOGI(TAG, "  RTS Pin: UART_PIN_NO_CHANGE");
    ESP_LOGI(TAG, "  CTS Pin: UART_PIN_NO_CHANGE");

    s_uart_num = uart_num;
    s_initialized = true;
    ESP_LOGI(TAG, "=== RS485 Initialization Complete ===");
    ESP_LOGI(TAG, "RS485 ready on UART%d, TX=GPIO%d, RX=GPIO%d, Baud=%d", 
             uart_num, tx_pin, rx_pin, baud_rate);
    
    return true;
}

uint16_t rs485_calculate_crc16(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
        
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

bool rs485_send_command(rs485_cmd_t cmd) {
    if (!s_initialized) {
        ESP_LOGE(TAG, "RS485 not initialized");
        return false;
    }

    // 构建命令数据（不含CRC和固定后缀）
    uint8_t cmd_data[6];  // 6字节：前缀5字节 + 命令1字节
    cmd_data[0] = RS485_CMD_PREFIX_0;
    cmd_data[1] = RS485_CMD_PREFIX_1;
    cmd_data[2] = RS485_CMD_PREFIX_2;
    cmd_data[3] = RS485_CMD_PREFIX_3;
    cmd_data[4] = RS485_CMD_PREFIX_4;
    cmd_data[5] = (uint8_t)cmd;

    // 计算CRC（对前6字节计算）
    uint16_t crc = rs485_calculate_crc16(cmd_data, 6);
    
    // 构建完整命令（包含CRC + 固定后缀00 00）
    uint8_t full_cmd[RS485_CMD_TOTAL_LENGTH];
    memcpy(full_cmd, cmd_data, 6);
    full_cmd[6] = (uint8_t)(crc & 0xFF);        // CRC低字节
    full_cmd[7] = (uint8_t)((crc >> 8) & 0xFF); // CRC高字节
    full_cmd[8] = 0x00;                          // 固定后缀：00
    full_cmd[9] = 0x00;                          // 固定后缀：00

    // 发送数据
    int bytes_written = uart_write_bytes(s_uart_num, full_cmd, RS485_CMD_TOTAL_LENGTH);
    
    if (bytes_written != RS485_CMD_TOTAL_LENGTH) {
        ESP_LOGE(TAG, "Failed to send command, written %d/%d bytes", bytes_written, RS485_CMD_TOTAL_LENGTH);
        return false;
    }

    // 等待发送完成（参照 demo，但添加超时保护）
    esp_err_t ret = uart_wait_tx_done(s_uart_num, pdMS_TO_TICKS(100));
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Timeout waiting for TX done: %s", esp_err_to_name(ret));
    }

    // 打印发送的命令详情（用于调试）
    ESP_LOGI(TAG, "Command sent: 0x%02X, Full cmd: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
             cmd, full_cmd[0], full_cmd[1], full_cmd[2], full_cmd[3],
             full_cmd[4], full_cmd[5], full_cmd[6], full_cmd[7], full_cmd[8], full_cmd[9]);
    return true;
}

bool rs485_send_data(const uint8_t *data, size_t length) {
    if (!s_initialized) {
        ESP_LOGE(TAG, "RS485 not initialized");
        return false;
    }

    if (data == NULL || length == 0) {
        ESP_LOGE(TAG, "Invalid data or length");
        return false;
    }

    int bytes_written = uart_write_bytes(s_uart_num, data, length);
    
    if (bytes_written != (int)length) {
        ESP_LOGE(TAG, "Failed to send data, written %d/%zu bytes", bytes_written, length);
        return false;
    }

    uart_wait_tx_done(s_uart_num, pdMS_TO_TICKS(100));
    return true;
}

void rs485_deinit(void) {
    if (s_initialized && s_uart_num < UART_NUM_MAX) {
        uart_driver_delete(s_uart_num);
        s_initialized = false;
        s_uart_num = UART_NUM_MAX;
        ESP_LOGI(TAG, "RS485 deinitialized");
    }
}

bool rs485_query_devices(void) {
    if (!s_initialized) {
        ESP_LOGE(TAG, "RS485 not initialized");
        return false;
    }

    // 查询在线设备命令: FF 03 00 3F 00 00 [CRC低] [CRC高] 00 00
    // FF: 设备地址
    // 03: 功能码（读保持寄存器）
    // 00 3F: 起始地址（0x003F = 63）
    // 00 00: 寄存器数量
    // 构建命令数据（不含CRC）
    uint8_t cmd_data[6] = {0xFF, 0x03, 0x00, 0x3F, 0x00, 0x00};
    
    // 计算CRC（对前6字节计算）
    uint16_t crc = rs485_calculate_crc16(cmd_data, 6);
    
    // 构建完整命令（包含CRC + 固定后缀00 00）
    uint8_t query_cmd[10];
    memcpy(query_cmd, cmd_data, 6);
    query_cmd[6] = (uint8_t)(crc & 0xFF);        // CRC低字节
    query_cmd[7] = (uint8_t)((crc >> 8) & 0xFF); // CRC高字节
    query_cmd[8] = 0x00;                          // 固定后缀：00
    query_cmd[9] = 0x00;                          // 固定后缀：00
    
    ESP_LOGI(TAG, "Query command CRC calculated: 0x%04X (low=0x%02X, high=0x%02X)", 
             crc, query_cmd[6], query_cmd[7]);

    // 发送查询命令
    int bytes_written = uart_write_bytes(s_uart_num, query_cmd, sizeof(query_cmd));
    
    if (bytes_written != sizeof(query_cmd)) {
        ESP_LOGE(TAG, "Failed to send query command, written %d/%zu bytes", bytes_written, sizeof(query_cmd));
        return false;
    }

    // 等待发送完成
    esp_err_t ret = uart_wait_tx_done(s_uart_num, pdMS_TO_TICKS(100));
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Timeout waiting for TX done: %s", esp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "Query command sent: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
             query_cmd[0], query_cmd[1], query_cmd[2], query_cmd[3],
             query_cmd[4], query_cmd[5], query_cmd[6], query_cmd[7], query_cmd[8], query_cmd[9]);

    // 清空接收缓冲区（在发送前）
    uart_flush_input(s_uart_num);
    
    // 给设备一些响应时间（RS485设备通常需要几毫秒处理）
    vTaskDelay(pdMS_TO_TICKS(10));

    // 等待并接收响应数据（增加超时时间到1000ms）
    uint8_t rx_buffer[256];
    ESP_LOGI(TAG, "Waiting for response (timeout: 1000ms)...");
    int len = rs485_receive_data(rx_buffer, sizeof(rx_buffer), 1000); // 1000ms超时
    
    if (len > 0) {
        ESP_LOGI(TAG, "✓ Received %d bytes:", len);
        
        
        // 打印为连续字符串格式
        char hex_str[512] = {0};
        for (int i = 0; i < len && i < 64; i++) { // 限制长度避免溢出
            char temp[4];
            snprintf(temp, sizeof(temp), "%02X ", rx_buffer[i]);
            strcat(hex_str, temp);
        }
        ESP_LOGI(TAG, "Response: %s", hex_str);
        
        return true;
    } else if (len == 0) {
        // 检查是否有数据在缓冲区中但没有读取完
        size_t buffered_size = 0;
        uart_get_buffered_data_len(s_uart_num, &buffered_size);
        if (buffered_size > 0) {
            ESP_LOGW(TAG, "Timeout but found %zu bytes in buffer, trying to read...", buffered_size);
            len = uart_read_bytes(s_uart_num, rx_buffer, 
                                  (buffered_size < sizeof(rx_buffer) - 1) ? buffered_size : sizeof(rx_buffer) - 1, 0);
            if (len > 0) {
                ESP_LOGI(TAG, "✓ Read %d bytes from buffer:", len);
                char hex_str[512] = {0};
                for (int i = 0; i < len && i < 64; i++) {
                    char temp[4];
                    snprintf(temp, sizeof(temp), "%02X ", rx_buffer[i]);
                    strcat(hex_str, temp);
                }
                ESP_LOGI(TAG, "Response: %s", hex_str);
                return true;
            }
        }
        ESP_LOGW(TAG, "✗ No response received (timeout after 1000ms)");
        ESP_LOGW(TAG, "Possible causes:");
        ESP_LOGW(TAG, "  1. Device not connected or powered off");
        ESP_LOGW(TAG, "  2. Wrong baud rate (current: check main.cpp)");
        ESP_LOGW(TAG, "  3. Wrong device address (sent: 0x%02X)", query_cmd[0]);
        ESP_LOGW(TAG, "  4. RS485 transceiver direction control issue");
        ESP_LOGW(TAG, "  5. TX/RX wires swapped");
        return false;
    } else {
        ESP_LOGE(TAG, "Error receiving data");
        return false;
    }
}

int rs485_receive_data(uint8_t *buffer, size_t buffer_size, uint32_t timeout_ms) {
    if (!s_initialized) {
        ESP_LOGE(TAG, "RS485 not initialized");
        return -1;
    }

    if (buffer == NULL || buffer_size == 0) {
        ESP_LOGE(TAG, "Invalid buffer or buffer size");
        return -1;
    }

    // 注意：不清空缓冲区，因为可能在查询函数中已经清空过了
    // 如果需要清空，应该在调用此函数前清空

    // 等待接收数据
    int len = uart_read_bytes(s_uart_num, buffer, buffer_size - 1, pdMS_TO_TICKS(timeout_ms));
    
    if (len > 0) {
        buffer[len] = '\0'; // 确保字符串结束
    } else if (len == 0) {
        // 超时，但没有错误
        ESP_LOGD(TAG, "Receive timeout after %lu ms", timeout_ms);
    }
    
    return len;
}
