/*
 * @Author: letian
 * @Date: 2022-12-26 09:59
 * @LastEditors: letian
 * @LastEditTime: 2022-12-26 14:12
 * @FilePath: \lvgl_demo\main\main.c
 * @Description:
 * Copyright (c) 2022 by letian 1656733975@qq.com, All Rights Reserved.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "lvgl_init.h"
#include "gui_guider.h"

#include "button.h"

#define TAG "main"

void app_main(void)
{
    printf("Hello world\n");
    xTaskCreatePinnedToCore(appguiTask, "App_Gui", 4096 * 4, NULL, 10, NULL, 1);

}
