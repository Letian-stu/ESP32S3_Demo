/*
 * @Author: letian
 * @Date: 2022-11-22 21:00
 * @LastEditors: letian
 * @LastEditTime: 2022-12-02 11:53
 * @FilePath: \speak-no-wakeup\main\app_main.c
 * @Description:
 * Copyright (c) 2022 by letian 1656733965@qq.com, All Rights Reserved.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "app_speech_if.h"

static const char *TAG = "Main";

static TaskHandle_t g_breath_light_task_handle = NULL;

static void breath_light_task(void *arg)
{
    printf("你好呀，我的朋友\n");
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG,"Listening");
    }
}

//唤醒回调
static void sr_wake(void *arg)
{
    /**< Turn on the breathing light */
    xTaskCreate(breath_light_task, "breath_light_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, &g_breath_light_task_handle);
}
//命令回调
static void sr_cmd(void *arg)
{
    if (NULL != g_breath_light_task_handle)
    {
        vTaskDelete(g_breath_light_task_handle);
    }

    int32_t cmd_id = (int32_t)arg;

    switch (cmd_id)
    {
    case 0:
        break;
    case 1:printf("开灯\n");
        break;
    case 2:printf("关灯\n");
        break;
    case 3:printf("打开风扇\n");
        break;
    case 4:printf("关闭风扇\n");
        break;
    default:break;
    }
}
//结束回调
static void sr_cmd_exit(void *arg)
{
    if (NULL != g_breath_light_task_handle)
    {
        vTaskDelete(g_breath_light_task_handle);
    }
}

void app_main(void)
{
    /**< Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    speech_recognition_init();

    //安装回调函数
    sr_handler_install(SR_CB_TYPE_WAKE, sr_wake, NULL);
    sr_handler_install(SR_CB_TYPE_CMD, sr_cmd, NULL);
    sr_handler_install(SR_CB_TYPE_CMD_EXIT, sr_cmd_exit, NULL);
    while(1)
    {
        //printf("hello world \n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }   
}
