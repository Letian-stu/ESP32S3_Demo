/*
 * @Author: StuTian
 * @Date: 2022-09-03 22:14
 * @LastEditors: letian
 * @LastEditTime: 2023-01-14 07:59
 * @FilePath: \take_photo_to_lvgl\main\lvgl_task\src\lvgl_init.c
 * @Description:
 * Copyright (c) 2022 by StuTian 1656733975@qq.com, All Rights Reserved.
 */
#include "lvgl_init.h"
#include "button.h"
#include "lv_fs_if.h"
/******************************************************************
 *  STATIC PROTOTYPES
 ******************************************************************/
static const char *TAG = "lvgl";

lv_ui guider_ui;
SemaphoreHandle_t xGuiSemaphore;

void lv_tick_task(void *arg)
{
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

void appguiTask(void *pvParameter)
{
    xGuiSemaphore = xSemaphoreCreateMutex();
    lv_init();
    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();

    lv_fs_if_init();
    // lv_png_init();
    // lv_split_jpeg_init();

    lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    /* Use double buffered when not working with monochrome displays */
    lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);

    static lv_disp_draw_buf_t disp_buf;
    uint32_t size_in_px = DISP_BUF_SIZE;
    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);
    
    // lv_port_indev_init();//indev init

    ESP_LOGI(TAG, "Init indev success");
    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
    ESP_LOGI(TAG, "Start APP UI Init ");

    setup_ui(&guider_ui); // 基础UI渲染函数

    while (1)
    {
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
        {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    /* A task should NEVER return */
    // free(buf1);
    // free(buf2);
    // vTaskDelete(NULL);
}