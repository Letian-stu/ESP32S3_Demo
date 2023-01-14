/*
 * @Author: letian
 * @Date: 2023-01-14 20:10
 * @LastEditors: letian
 * @LastEditTime: 2023-01-14 20:12
 * @FilePath: \take_photo_to_lvgl\main\cam_task\cam_task.h
 * @Description: 
 * Copyright (c) 2023 by letian 1656733975@qq.com, All Rights Reserved. 
 */
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"

#define CAM_PIN_PWDN 15
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 12
#define CAM_PIN_SIOD 14
#define CAM_PIN_SIOC 13
#define CAM_PIN_D7 9
#define CAM_PIN_D6 8
#define CAM_PIN_D5 7
#define CAM_PIN_D4 5
#define CAM_PIN_D3 3
#define CAM_PIN_D2 1
#define CAM_PIN_D1 2
#define CAM_PIN_D0 4
#define CAM_PIN_VSYNC 11
#define CAM_PIN_HREF 10
#define CAM_PIN_PCLK 6

void cam_config_init(void);
