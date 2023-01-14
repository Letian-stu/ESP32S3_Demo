/*
 * @Author: letian
 * @Date: 2023-01-13 22:12
 * @LastEditors: letian
 * @LastEditTime: 2023-01-13 22:29
 * @FilePath: \take_photo_to_lvgl\components\bsp_sdcard\sdcard.h
 * @Description: 
 * Copyright (c) 2023 by letian 1656733975@qq.com, All Rights Reserved. 
 */
/**
 * @file sdcard.h
 *
 */

#ifndef SDCARD_H
#define SDCARD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO 40
#define PIN_NUM_MOSI 38
#define PIN_NUM_CLK 39
#define PIN_NUM_CS 37


esp_err_t sd_card_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif /*SDCARD_H*/
