/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "eez_ui/ui.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rs485_comm.h"
#include "waveshare_rgb_lcd_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// RS485 配置 - 参考demo项目配置 (02_RS485_Test)
// Demo使用: UART_NUM_2, TX=16, RX=15, 115200
#define RS485_UART_NUM UART_NUM_2
#define RS485_TX_PIN 16      // 参考demo配置
#define RS485_RX_PIN 15      // 参考demo配置
#define RS485_BAUD_RATE 9600 // 参考demo配置

// 测试任务配置
#define RS485_TEST_TASK_STACK_SIZE 4096 // 增加栈大小避免栈溢出
#define RS485_TEST_TASK_PRIORITY 5      // 降低优先级避免与系统任务冲突
#define RS485_BUF_SIZE 1024             // 参考demo配置

static const char *TAG_MAIN = "MAIN";

extern "C" void app_main() {
  // 先初始化其他系统组件（避免任务创建时的看门狗检查问题）
  waveshare_esp32_s3_rgb_lcd_init();
  if (lvgl_port_lock(-1)) {
    // lv_demo_stress();
    // lv_demo_benchmark();
    // lv_demo_music();
    // lv_demo_widgets();
    // example_lvgl_demo_ui();
    // Release the mutex
    ui_init();
    lvgl_port_unlock();
  }

  // 等待系统稳定
  vTaskDelay(pdMS_TO_TICKS(200));

  // 初始化 RS485 通信
  bool rs485_ok = false;
  if (!rs485_init(RS485_UART_NUM, RS485_TX_PIN, RS485_RX_PIN,
                  RS485_BAUD_RATE)) {
    ESP_LOGE(TAG_MAIN, "Failed to initialize RS485");
  } else {
    ESP_LOGI(TAG_MAIN, "RS485 initialized successfully");
    rs485_ok = true;
  }
}
