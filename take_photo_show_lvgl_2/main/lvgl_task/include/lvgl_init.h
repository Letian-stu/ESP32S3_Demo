/*
 * @Author: StuTian
 * @Date: 2022-09-03 22:14
 * @LastEditors: letian
 * @LastEditTime: 2023-01-14 20:20
 * @FilePath: \take_photo_to_lvgl\main\lvgl_task\include\lvgl_init.h
 * @Description: 
 * Copyright (c) 2022 by StuTian 1656733975@qq.com, All Rights Reserved. 
 */
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "lvgl.h"
#include "freertos/semphr.h"
#include "esp_system.h"

/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif


#include "lvgl_helpers.h"
#include "lvgl/src/hal/lv_hal_disp.h"
#include "lv_port_indev.h"
#include "esp_log.h"

#define LV_TICK_PERIOD_MS 1

void lv_tick_task(void *arg);
void gui_task(void *pvParameter);

