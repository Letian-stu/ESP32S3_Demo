/*
 * @Author: letian
 * @Date: 2023-01-13 15:57
 * @LastEditors: letian
 * @LastEditTime: 2023-01-14 20:20
 * @FilePath: \take_photo_to_lvgl\main\main.c
 * @Description:
 * Copyright (c) 2023 by letian 1656733975@qq.com, All Rights Reserved.
 */
#include <esp_log.h>
#include <esp_system.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_init.h"
#include "cam_task.h"

#define TAG "main"

lv_img_dsc_t img_dsc = {
    .header.always_zero = 0,
    .header.w = 320,
    .header.h = 240,
    .data_size = 320 * 240 * 2,
    .header.cf = LV_IMG_CF_TRUE_COLOR,
    .data = NULL,
};
camera_fb_t *pic;
extern lv_obj_t *img_cam; 

int app_main(void)
{
    cam_config_init();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    xTaskCreatePinnedToCore(gui_task, "gui_task", 1024 * 8, NULL, 1, NULL, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    while (1)
    {
        static int64_t last_frame = 0;
        if (!last_frame)
        {
            last_frame = esp_timer_get_time();
        }
        pic = esp_camera_fb_get();
        img_dsc.data = pic->buf;
        lv_img_set_src(img_cam, &img_dsc);
        esp_camera_fb_return(pic);

        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        ESP_LOGI("cam", "MJPG:  %ums (%.1ffps)", (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
    }
    return 0;
}


