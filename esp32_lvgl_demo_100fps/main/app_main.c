/*
 * @Author: Letian-stu
 * @Date: 2023-05-05 09:18
 * @LastEditors: Letian-stu
 * @LastEditTime: 2023-05-05 12:39
 * @FilePath: /esp32_lvgl_demo/main/app_main.c
 * @Description: 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lvgl_helpers.h"
#include "esp_freertos_hooks.h"
#include "lv_demo_benchmark.h"

#define LV_TICK_PERIOD_MS 1

SemaphoreHandle_t xGuiSemaphore;

static void lv_tick_task(void *arg)
{
   (void)arg;
   lv_tick_inc(LV_TICK_PERIOD_MS);
}

static lv_disp_draw_buf_t draw_buf;

static lv_color_t buf1[DISP_BUF_SIZE];
static lv_color_t buf2[DISP_BUF_SIZE];

void app_main(void)
{
   xGuiSemaphore = xSemaphoreCreateMutex();
   lv_init();          //lvgl内核初始化
   lvgl_driver_init(); //lvgl显示接口初始化
   lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DISP_BUF_SIZE ); 

   static lv_disp_drv_t disp_drv;         /*A variable to hold the drivers. Must be static or global.*/
   lv_disp_drv_init(&disp_drv);           /*Basic initialization*/
   disp_drv.draw_buf = &draw_buf;         /*Set an initialized buffer*/
   disp_drv.flush_cb = disp_driver_flush; /*Set a flush callback to draw to the display*/
   disp_drv.hor_res = 240;                /*Set the horizontal resolution in pixels*/
   disp_drv.ver_res = 280;                /*Set the vertical resolution in pixels*/
   disp_drv.offset_y = 20;
   lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/

   const esp_timer_create_args_t periodic_timer_args = {
		.callback = &lv_tick_task,
		.name = "periodic_gui"};
	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

   lv_demo_benchmark();
   while (1)
   {
      vTaskDelay(pdMS_TO_TICKS(10));
      if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
      {
         lv_timer_handler();
         xSemaphoreGive(xGuiSemaphore);
      }
   }
}

