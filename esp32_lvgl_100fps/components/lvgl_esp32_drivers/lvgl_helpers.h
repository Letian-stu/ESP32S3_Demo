/*
 * @Author: Letian-stu
 * @Date: 2023-05-05 09:21
 * @LastEditors: Letian-stu
 * @LastEditTime: 2023-05-06 19:11
 * @FilePath: /esp32_lvgl_100fps/components/lvgl_esp32_drivers/lvgl_helpers.h
 * @Description: 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
/**
 * @file lvgl_helpers.h
 */

#ifndef LVGL_HELPERS_H
#define LVGL_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdbool.h>

#include "lvgl_spi_conf.h"
#include "lvgl_tft/disp_driver.h"
#include "lvgl_tft/esp_lcd_backlight.h"
#include "lvgl_touch/touch_driver.h"

/*********************
 *      DEFINES
 *********************/
#define LV_HOR_RES_MAX  240
#define LV_VER_RES_MAX  280

#define DISP_BUF_SIZE  (LV_HOR_RES_MAX * 60)

#define SPI_HOST_MAX 3
/* DISP_BUF_SIZE value doesn't have an special meaning, but it's the size
 * of the buffer(s) passed to LVGL as display buffers. The default values used
 * were the values working for the contributor of the display controller.
 *
 * As LVGL supports partial display updates the DISP_BUF_SIZE doesn't
 * necessarily need to be equal to the display size.
 *
 * When using RGB displays the display buffer size will also depends on the
 * color format being used, for RGB565 each pixel needs 2 bytes.
 * When using the mono theme, the display pixels can be represented in one bit,
 * so the buffer size can be divided by 8, e.g. see SSD1306 display size. */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lvgl_i2c_locking(void* leader);

/* Initialize detected SPI and I2C bus and devices */
void lvgl_driver_init(void);

/* Initialize SPI master  */
bool lvgl_spi_driver_init(int host, int miso_pin, int mosi_pin, int sclk_pin,
    int max_transfer_sz, int dma_channel, int quadwp_pin, int quadhd_pin);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LVGL_HELPERS_H */
