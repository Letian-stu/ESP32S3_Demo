/*
 * @Author: letian
 * @Date: 2023-01-13 15:57
 * @LastEditors: letian
 * @LastEditTime: 2023-01-14 21:54
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
#include "gui_guider.h"
#include "lv_lib_split_jpg/lv_sjpg.h"
#include "lv_lib_png/lv_png.h"
#include "sdcard.h"

#define TAG "CAM"

// lv_fs_file_t lv_f;
// lv_fs_res_t lv_res;
// uint32_t lv_resize;
// char readbuff[32];

int app_main(void)
{
    sd_card_init();
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(appguiTask, "App_Gui", 1024 * 8, NULL, 5, NULL, 1);
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // lv_res = lv_fs_open(&lv_f,"S:/log.txt", LV_FS_MODE_RD);
    // if (lv_res != LV_FS_RES_OK) {	
    //     ESP_LOGE(TAG,"open jpg ERR");
    // }else{
    //     lv_fs_read(&lv_f, readbuff, 10, &lv_resize);
    //     if(lv_resize != 10){
    //         ESP_LOGE(TAG,"write jpg ERR");
    //     }
    //     ESP_LOGI(TAG,"save jpg len %d buf %s",lv_resize,readbuff);    
    //     lv_res = lv_fs_close(&lv_f);
    //     if (lv_res != LV_FS_RES_OK) 
    //     {
    //         ESP_LOGE(TAG,"clone jpg ERR");
    //     }
    //     ESP_LOGI(TAG,"save jpg ok");
    // }

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    return 0;
}


