/*
 * @Author: letian
 * @Date: 2023-01-17 10:21
 * @LastEditors: letian
 * @LastEditTime: 2023-01-17 13:06
 * @FilePath: \ledc_fade\main\ledc_fade_example_main.c
 * @Description: 
 * Copyright (c) 2023 by letian 1656733975@qq.com, All Rights Reserved. 
 */
/* LEDC (LED Controller) fade example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#define LEDC_GPIO             (36)
#define LEDC_MAX_DUTY         (8000)
#define LEDC_MIN_DUTY         (0)
#define LEDC_FADE_TIME        (1000)

ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 0,
        .gpio_num   = LEDC_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_1,
        .flags.output_invert = 0
};

void LEDC_init(void)
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,           // timer mode
        .timer_num = LEDC_TIMER_1,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);
    ledc_fade_func_install(0);
}

void app_main(void)
{
    LEDC_init();
    while (1)
    {
        printf("1. LEDC fade up to duty = %d\n", LEDC_MAX_DUTY);
        ledc_set_fade_with_time(ledc_channel.speed_mode,ledc_channel.channel, LEDC_MAX_DUTY, LEDC_FADE_TIME);
        ledc_fade_start(ledc_channel.speed_mode,ledc_channel.channel, LEDC_FADE_NO_WAIT);
    
        printf("2. LEDC fade down to duty = %d\n", LEDC_MIN_DUTY);
        ledc_set_fade_with_time(ledc_channel.speed_mode,ledc_channel.channel, LEDC_MIN_DUTY, LEDC_FADE_TIME);
        ledc_fade_start(ledc_channel.speed_mode,ledc_channel.channel, LEDC_FADE_NO_WAIT);
    }
}
