#include <string.h>

#include "screens.h"
#include "core/lv_obj_style_gen.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"
#include "../rs485_comm.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;

// 日志行数计数器
static uint32_t log_row_count = 0;
#define MAX_LOG_ROWS 100

// 添加日志记录
static void add_log_entry(const char *button_name) {
    if (objects.tb_logs == NULL) {
        return;
    }
    
    
    
    // 添加新日志
    char log_text[128];
    snprintf(log_text, sizeof(log_text), "%s 被点击", button_name);
    lv_table_set_cell_value(objects.tb_logs, log_row_count, 0, log_text);
    log_row_count++;
    
    // 更新表格行数
    lv_table_set_row_count(objects.tb_logs, log_row_count);
}

void create_screen_main() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        
        // 标题banner
        {
            lv_obj_t *banner = lv_obj_create(parent_obj);
            lv_obj_set_pos(banner, 0, 0);
            lv_obj_set_size(banner, 800, 60);
            lv_obj_set_style_bg_color(banner, lv_color_hex(0x003366), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(banner, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(banner, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            
            lv_obj_t *title_label = lv_label_create(banner);
            lv_label_set_text(title_label, "川锅智慧安灯系统");
            lv_obj_set_style_text_font(title_label, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_center(title_label);
        }
        
        // 左侧按钮区域
        {
            lv_obj_t *button_panel = lv_obj_create(parent_obj);
            lv_obj_set_pos(button_panel, 0, 70);
            lv_obj_set_size(button_panel, 380, 400);
            lv_obj_set_style_bg_color(button_panel, lv_color_hex(0x1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(button_panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(button_panel, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(button_panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            // 禁用所有滚动
            lv_obj_set_scroll_dir(button_panel, LV_DIR_NONE);
            lv_obj_clear_flag(button_panel, LV_OBJ_FLAG_SCROLLABLE);
            
            lv_obj_t *parent_obj = button_panel;
            int btn_width = 110;
            int btn_height = 45;
            int btn_spacing_x = 10;
            int btn_spacing_y = 10;
            int start_x = 0;
            int start_y = 0;
            
            // 第一行：常亮按钮
            {
                // 红灯常亮
                lv_obj_t *btn = lv_button_create(parent_obj);
                objects.btn_red_on = btn;
                lv_obj_set_pos(btn, start_x, start_y);
                lv_obj_set_size(btn, btn_width, btn_height);
                lv_obj_set_style_text_font(btn, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(btn, on_btn_red_on_clicked, LV_EVENT_CLICKED, NULL);
                lv_obj_t *label = lv_label_create(btn);
                lv_label_set_text(label, "红灯常亮");
                lv_obj_center(label);
                
                // 黄灯常亮
                btn = lv_button_create(parent_obj);
                objects.btn_yellow_on = btn;
                lv_obj_set_pos(btn, start_x + btn_width + btn_spacing_x, start_y);
                lv_obj_set_size(btn, btn_width, btn_height);
                lv_obj_set_style_text_font(btn, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(btn, on_btn_yellow_on_clicked, LV_EVENT_CLICKED, NULL);
                label = lv_label_create(btn);
                lv_label_set_text(label, "黄灯常亮");
                lv_obj_center(label);
                
                // 绿灯常亮
                btn = lv_button_create(parent_obj);
                objects.btn_green_on = btn;
                lv_obj_set_pos(btn, start_x + (btn_width + btn_spacing_x) * 2, start_y);
                lv_obj_set_size(btn, btn_width, btn_height);
                lv_obj_set_style_text_font(btn, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(btn, on_btn_green_on_clicked, LV_EVENT_CLICKED, NULL);
                label = lv_label_create(btn);
                lv_label_set_text(label, "绿灯常亮");
                lv_obj_center(label);
            }
            
            // 第二行：慢闪按钮
            start_y += btn_height + btn_spacing_y;
            {
                // 红灯慢闪
                lv_obj_t *btn = lv_button_create(parent_obj);
                objects.btn_red_slow = btn;
                lv_obj_set_pos(btn, start_x, start_y);
                lv_obj_set_size(btn, btn_width, btn_height);
                lv_obj_set_style_text_font(btn, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(btn, on_btn_red_slow_clicked, LV_EVENT_CLICKED, NULL);
                lv_obj_t *label = lv_label_create(btn);
                lv_label_set_text(label, "红灯慢闪");
                lv_obj_center(label);
                
                // 黄灯慢闪
                btn = lv_button_create(parent_obj);
                objects.btn_yellow_slow = btn;
                lv_obj_set_pos(btn, start_x + btn_width + btn_spacing_x, start_y);
                lv_obj_set_size(btn, btn_width, btn_height);
                lv_obj_set_style_text_font(btn, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(btn, on_btn_yellow_slow_clicked, LV_EVENT_CLICKED, NULL);
                label = lv_label_create(btn);
                lv_label_set_text(label, "黄灯慢闪");
                lv_obj_center(label);
                
                // 绿灯慢闪
                btn = lv_button_create(parent_obj);
                objects.btn_green_slow = btn;
                lv_obj_set_pos(btn, start_x + (btn_width + btn_spacing_x) * 2, start_y);
                lv_obj_set_size(btn, btn_width, btn_height);
                lv_obj_set_style_text_font(btn, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(btn, on_btn_green_slow_clicked, LV_EVENT_CLICKED, NULL);
                label = lv_label_create(btn);
                lv_label_set_text(label, "绿灯慢闪");
                lv_obj_center(label);
            }
            
            // 第三行：爆闪按钮
            start_y += btn_height + btn_spacing_y;
            {
                // 红灯爆闪
                lv_obj_t *btn = lv_button_create(parent_obj);
                objects.btn_red_burst = btn;
                lv_obj_set_pos(btn, start_x, start_y);
                lv_obj_set_size(btn, btn_width, btn_height);
                lv_obj_set_style_text_font(btn, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(btn, on_btn_red_burst_clicked, LV_EVENT_CLICKED, NULL);
                lv_obj_t *label = lv_label_create(btn);
                lv_label_set_text(label, "红灯爆闪");
                lv_obj_center(label);
                
                // 黄灯爆闪
                btn = lv_button_create(parent_obj);
                objects.btn_yellow_burst = btn;
                lv_obj_set_pos(btn, start_x + btn_width + btn_spacing_x, start_y);
                lv_obj_set_size(btn, btn_width, btn_height);
                lv_obj_set_style_text_font(btn, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(btn, on_btn_yellow_burst_clicked, LV_EVENT_CLICKED, NULL);
                label = lv_label_create(btn);
                lv_label_set_text(label, "黄灯爆闪");
                lv_obj_center(label);
                
                // 绿灯爆闪
                btn = lv_button_create(parent_obj);
                objects.btn_green_burst = btn;
                lv_obj_set_pos(btn, start_x + (btn_width + btn_spacing_x) * 2, start_y);
                lv_obj_set_size(btn, btn_width, btn_height);
                lv_obj_set_style_text_font(btn, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(btn, on_btn_green_burst_clicked, LV_EVENT_CLICKED, NULL);
                label = lv_label_create(btn);
                lv_label_set_text(label, "绿灯爆闪");
                lv_obj_center(label);
            }
            
            // 第四行：关闭按钮和查询按钮
            start_y += btn_height + btn_spacing_y;
            {
                // 警灯关闭
                lv_obj_t *btn = lv_button_create(parent_obj);
                objects.btn_light_off = btn;
                lv_obj_set_pos(btn, start_x, start_y);
                lv_obj_set_size(btn, btn_width, btn_height);
                lv_obj_set_style_text_font(btn, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(btn, on_btn_light_off_clicked, LV_EVENT_CLICKED, NULL);
                lv_obj_t *label = lv_label_create(btn);
                lv_label_set_text(label, "警灯关闭");
                lv_obj_center(label);
                
                // 查询在线设备
                btn = lv_button_create(parent_obj);
                objects.btn_query_devices = btn;
                lv_obj_set_pos(btn, start_x + btn_width + btn_spacing_x, start_y);
                lv_obj_set_size(btn, btn_width, btn_height);
                lv_obj_set_style_text_font(btn, &ui_font_chinese_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_add_event_cb(btn, on_btn_query_devices_clicked, LV_EVENT_CLICKED, NULL);
                label = lv_label_create(btn);
                lv_label_set_text(label, "查询设备");
                lv_obj_center(label);
            }
        }
        
        // 右侧日志区域
        {
            lv_obj_t *log_panel = lv_obj_create(parent_obj);
            lv_obj_set_pos(log_panel, 400, 70);
            lv_obj_set_size(log_panel, 400, 400);
            lv_obj_set_style_bg_color(log_panel, lv_color_hex(0x1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(log_panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(log_panel, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(log_panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            // 禁用横向滚动
            lv_obj_set_scroll_dir(log_panel, LV_DIR_NONE);
            lv_obj_clear_flag(log_panel, LV_OBJ_FLAG_SCROLLABLE);
            
            // 日志表格
            lv_obj_t *log_table = lv_table_create(log_panel);
            objects.tb_logs = log_table;
            lv_obj_set_pos(log_table, 0, 0);
            lv_obj_set_size(log_table, 400, 400);
            lv_obj_set_style_border_width(log_table, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_table_set_column_count(log_table, 1);
            lv_table_set_column_width(log_table, 0, 400);
            lv_table_set_row_count(log_table, 0);
            
            // 设置表格背景为透明（与面板一致）
            lv_obj_set_style_bg_opa(log_table, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
            // 只允许垂直滚动
            lv_obj_set_scroll_dir(log_table, LV_DIR_VER);
            
            // 设置表格单元格样式（LV_PART_ITEMS 控制单元格）
            lv_obj_set_style_bg_color(log_table, lv_color_hex(0x1a1a1a), LV_PART_ITEMS | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(log_table, LV_OPA_TRANSP, LV_PART_ITEMS | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(log_table, 0, LV_PART_ITEMS | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(log_table, 5, LV_PART_ITEMS | LV_STATE_DEFAULT);
            // 重要：表格单元格的文本颜色和字体必须设置到 LV_PART_ITEMS
            lv_obj_set_style_text_color(log_table, lv_color_hex(0xffffff), LV_PART_ITEMS | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(log_table, &ui_font_chinese_18, LV_PART_ITEMS | LV_STATE_DEFAULT);
            log_row_count = 0;
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
}


// RS485 按钮事件处理函数
void on_btn_red_on_clicked(lv_event_t *e) {
    (void)e;
    if (rs485_send_command(RS485_CMD_RED_ON)) {
        add_log_entry("红灯常亮");
    }
}

void on_btn_yellow_on_clicked(lv_event_t *e) {
    (void)e;
    if (rs485_send_command(RS485_CMD_YELLOW_ON)) {
        add_log_entry("黄灯常亮");
    }
}

void on_btn_green_on_clicked(lv_event_t *e) {
    (void)e;
    if (rs485_send_command(RS485_CMD_GREEN_ON)) {
        add_log_entry("绿灯常亮");
    }
}

void on_btn_red_slow_clicked(lv_event_t *e) {
    (void)e;
    if (rs485_send_command(RS485_CMD_RED_SLOW_FLASH)) {
        add_log_entry("红灯慢闪");
    }
}

void on_btn_yellow_slow_clicked(lv_event_t *e) {
    (void)e;
    if (rs485_send_command(RS485_CMD_YELLOW_SLOW_FLASH)) {
        add_log_entry("黄灯慢闪");
    }
}

void on_btn_green_slow_clicked(lv_event_t *e) {
    (void)e;
    if (rs485_send_command(RS485_CMD_GREEN_SLOW_FLASH)) {
        add_log_entry("绿灯慢闪");
    }
}

void on_btn_red_burst_clicked(lv_event_t *e) {
    (void)e;
    if (rs485_send_command(RS485_CMD_RED_BURST_FLASH)) {
        add_log_entry("红灯爆闪");
    }
}

void on_btn_yellow_burst_clicked(lv_event_t *e) {
    (void)e;
    if (rs485_send_command(RS485_CMD_YELLOW_BURST_FLASH)) {
        add_log_entry("黄灯爆闪");
    }
}

void on_btn_green_burst_clicked(lv_event_t *e) {
    (void)e;
    if (rs485_send_command(RS485_CMD_GREEN_BURST_FLASH)) {
        add_log_entry("绿灯爆闪");
    }
}

void on_btn_light_off_clicked(lv_event_t *e) {
    (void)e;
    if (rs485_send_command(RS485_CMD_LIGHT_OFF)) {
        add_log_entry("警灯关闭");
    }
}

void on_btn_query_devices_clicked(lv_event_t *e) {
    (void)e;
    if (rs485_query_devices()) {
        add_log_entry("查询设备");
    }
}

static const char *screen_names[] = { "Main" };
static const char *object_names[] = { 
    "main", "btn_light", "lab_light", "tb_logs",
    "btn_red_on", "btn_yellow_on", "btn_green_on",
    "btn_red_slow", "btn_yellow_slow", "btn_green_slow",
    "btn_red_burst", "btn_yellow_burst", "btn_green_burst",
    "btn_light_off", "btn_query_devices"
};


typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    eez_flow_init_screen_names(screen_names, sizeof(screen_names) / sizeof(const char *));
    eez_flow_init_object_names(object_names, sizeof(object_names) / sizeof(const char *));
    
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
}
