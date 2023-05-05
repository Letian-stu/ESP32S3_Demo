idf_component_register(
        SRCS    "main.c"
        "lvgl_task/src/lvgl_init.c" 
        "lvgl_task/src/gui_guider.c" 
        "lvgl_task/src/lv_port_indev.c" 


        INCLUDE_DIRS    "."
        "lvgl_task/include"
)
