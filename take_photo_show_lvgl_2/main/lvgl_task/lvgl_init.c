/*
 * @Author: StuTian
 * @Date: 2022-09-03 22:14
 * @LastEditors: letian
 * @LastEditTime: 2023-01-14 20:52
 * @FilePath: \take_photo_to_lvgl\main\lvgl_task\lvgl_init.c
 * @Description:
 * Copyright (c) 2022 by StuTian 1656733975@qq.com, All Rights Reserved.
 */
#include "lvgl_init.h"

#define TAG  "lvgl"

SemaphoreHandle_t xGuiSemaphore;

void lv_tick_task(void *arg)
{
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

lv_obj_t *img_cam; //要显示图像

void gui_setup(void)
{
    img_cam = lv_img_create(lv_scr_act());
    lv_obj_set_pos(img_cam, 0, 0);
    lv_obj_set_size(img_cam, 320, 240);
}

void lvgl_dsip_init(void)
{
    static lv_disp_draw_buf_t draw_buf;
    lv_color_t *buf1 = heap_caps_malloc(DLV_HOR_RES_MAX * 10 * 2, MALLOC_CAP_DMA);
    lv_color_t *buf2 = heap_caps_malloc(DLV_HOR_RES_MAX * 10 * 2, MALLOC_CAP_DMA);

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DLV_HOR_RES_MAX * 10); /*Initialize the display buffer*/

    static lv_disp_drv_t disp_drv;         /*A variable to hold the drivers. Must be static or global.*/
    lv_disp_drv_init(&disp_drv);           /*Basic initialization*/
    disp_drv.draw_buf = &draw_buf;         /*Set an initialized buffer*/
    disp_drv.flush_cb = disp_driver_flush; /*Set a flush callback to draw to the display*/
    disp_drv.hor_res = DLV_HOR_RES_MAX;                /*Set the horizontal resolution in pixels*/
    disp_drv.ver_res = DLV_VER_RES_MAX;                /*Set the vertical resolution in pixels*/
    lv_disp_drv_register(&disp_drv);       /*Register the driver and save the created display objects*/

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));
}

void gui_task(void *pvParameter)
{
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();          
    lvgl_driver_init(); 
    lvgl_dsip_init();
    //lv_port_indev_init();
    gui_setup();
    while (1)
    {
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
        {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}