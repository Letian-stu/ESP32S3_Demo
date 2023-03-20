#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lvgl_helpers.h"
#include "esp_freertos_hooks.h"

#include "lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"
#include "lv_examples/src/lv_demo_music/lv_demo_music.h"
#include "lv_examples/src/lv_demo_benchmark/lv_demo_benchmark.h"


static void lv_tick_task(void *arg)
{
   (void)arg;
   lv_tick_inc(10);
}


SemaphoreHandle_t xGuiSemaphore;

static void gui_task(void *arg)
{
   xGuiSemaphore = xSemaphoreCreateMutex();
   lv_init();          //lvgl内核初始化
   lvgl_driver_init(); //lvgl显示接口初始化

   static lv_disp_draw_buf_t draw_buf;
   
   lv_color_t *buf1 = heap_caps_malloc(DLV_HOR_RES_MAX * 90, MALLOC_CAP_DMA);
   lv_color_t *buf2 = heap_caps_malloc(DLV_HOR_RES_MAX * 90, MALLOC_CAP_DMA);

   lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DLV_HOR_RES_MAX * 90 / 2); 

   static lv_disp_drv_t disp_drv;         /*A variable to hold the drivers. Must be static or global.*/
   lv_disp_drv_init(&disp_drv);           /*Basic initialization*/
   disp_drv.draw_buf = &draw_buf;         /*Set an initialized buffer*/
   disp_drv.flush_cb = disp_driver_flush; /*Set a flush callback to draw to the display*/
   disp_drv.hor_res = 280;                /*Set the horizontal resolution in pixels*/
   disp_drv.ver_res = 240;                /*Set the vertical resolution in pixels*/
   lv_disp_drv_register(&disp_drv);       /*Register the driver and save the created display objects*/
   
	const esp_timer_create_args_t periodic_timer_args = {
		.callback = &lv_tick_task,
		.name = "periodic_gui"};
	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));

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
void app_main(void)
{
   xTaskCreatePinnedToCore(gui_task, "gui task", 1024 * 4, NULL, 1, NULL, 0);
}
