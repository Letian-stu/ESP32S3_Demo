idf_component_register(
        SRCS    "main.c" 
        "lvgl_task/lvgl_init.c" 
        "lvgl_task/lv_port_indev.c" 
        "cam_task/cam_task.c" 

        INCLUDE_DIRS    "."
        "lvgl_task/include/"
        "cam_task/"
)