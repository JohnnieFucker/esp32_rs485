#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
  lv_obj_t *main;
  lv_obj_t *btn_light;
  lv_obj_t *lab_light;
  lv_obj_t *tb_logs;
  // RS485 控制按钮
  lv_obj_t *btn_red_on;
  lv_obj_t *btn_yellow_on;
  lv_obj_t *btn_green_on;
  lv_obj_t *btn_red_slow;
  lv_obj_t *btn_yellow_slow;
  lv_obj_t *btn_green_slow;
  lv_obj_t *btn_red_burst;
  lv_obj_t *btn_yellow_burst;
  lv_obj_t *btn_green_burst;
  lv_obj_t *btn_light_off;
  lv_obj_t *btn_query_devices; // 查询在线设备按钮
} objects_t;

extern objects_t objects;

enum ScreensEnum {
  SCREEN_ID_MAIN = 1,
};

void create_screen_main();
void tick_screen_main();

// RS485 按钮事件处理函数
void on_btn_red_on_clicked(lv_event_t *e);
void on_btn_yellow_on_clicked(lv_event_t *e);
void on_btn_green_on_clicked(lv_event_t *e);
void on_btn_red_slow_clicked(lv_event_t *e);
void on_btn_yellow_slow_clicked(lv_event_t *e);
void on_btn_green_slow_clicked(lv_event_t *e);
void on_btn_red_burst_clicked(lv_event_t *e);
void on_btn_yellow_burst_clicked(lv_event_t *e);
void on_btn_green_burst_clicked(lv_event_t *e);
void on_btn_light_off_clicked(lv_event_t *e);
void on_btn_query_devices_clicked(lv_event_t *e);

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/