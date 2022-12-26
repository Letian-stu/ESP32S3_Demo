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

TaskHandle_t Key_Scan_Task_Handler;
QueueHandle_t Key_Num_Queue;

void KEY_Scan_thread_entry(void *pvParameters)
{
    Button_Init();
    while (1)
    {
        Button_Process();
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void Menu_Switch(void)
{
    switch (Button_Value)
    {
    case BT1_DOWN:
        //ESP_LOGI(TAG, "BT1 one");
        xQueueSend(Key_Num_Queue, &Button_Value, 0);
        break;
    case BT2_DOWN:
        //ESP_LOGI(TAG, "BT2 one");
        xQueueSend(Key_Num_Queue, &Button_Value, 0);
        break;

    case BT1_LONG:
        //ESP_LOGI(TAG, "BT1 long");
        xQueueSend(Key_Num_Queue, &Button_Value, 0);
        break;
    case BT2_LONG:
        //ESP_LOGI(TAG, "BT2 long");
        xQueueSend(Key_Num_Queue, &Button_Value, 0);
        break;
    // case BT1_DOUBLE:
    //     ESP_LOGI(TAG, "BT1 two");
    //     break;
    // case BT2_DOUBLE:
    //     ESP_LOGI(TAG, "BT2 two");
    //     break;
    default:
        break;
    }
    Button_Value = 0;
}

void app_main(void)
{
    printf("Hello world\n");
    Key_Num_Queue = xQueueCreate(2, sizeof(uint8_t));
    xTaskCreatePinnedToCore(appguiTask, "App_Gui", 4096 * 4, NULL, 4, NULL, 1);
    xTaskCreate(KEY_Scan_thread_entry, "Key_Scan", 4096, NULL, 5, &Key_Scan_Task_Handler);
    while (1)
    {
        //printf("task doing\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
