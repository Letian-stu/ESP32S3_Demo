/*
 * @Author: letian
 * @Date: 2022-12-29 20:55
 * @LastEditors: letian
 * @LastEditTime: 2022-12-29 21:15
 * @FilePath: \AHT20-ESP-IDF-main\main\main.c
 * @Description: 
 * Copyright (c) 2022 by letian 1656733975@qq.com, All Rights Reserved. 
 */
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "i2c_master.h"
#include "aht20_driver.h"

#define AHT20_ADDR     0x38

void app_main(void)
{
    float H, T;
    I2cMaster_handle_t i2c_0 = I2cMaster_Init(I2C_NUM_0, 41, 42, 400000);
    AHT20_handle_t aht20 = AHT20_Init(i2c_0, AHT20_ADDR);

    while(1){
        AHT20_GetRawData(aht20);
        AHT20_StandardUnitCon(aht20, &H, &T);
        printf("humidity = %.1f    temperature = %.1f   \n", H, T);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
