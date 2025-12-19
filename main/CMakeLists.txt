set(UI_DIR "./eez_ui")
file(GLOB_RECURSE UI_SRCS ${UI_DIR}/*.c ${UI_DIR}/*.cpp)

idf_component_register(
    SRCS "waveshare_rgb_lcd_port.c" "main.cpp" "lvgl_port.c" "rs485_comm.c" ${UI_SRCS}
    INCLUDE_DIRS ".")
