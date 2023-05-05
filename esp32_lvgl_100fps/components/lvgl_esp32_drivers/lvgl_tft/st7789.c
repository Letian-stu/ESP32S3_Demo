/**
 * @file st7789.c
 *
 * Mostly taken from lbthomsen/esp-idf-littlevgl github.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "esp_log.h"

#include "st7789.h"

#include "disp_spi.h"
#include "driver/gpio.h"

/*********************
 *      DEFINES
 *********************/
#define TAG "st7789"
/**********************
 *      TYPEDEFS
 **********************/

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void st7789_set_orientation(uint8_t orientation);

static void st7789_send_color(void *data, size_t length);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void st7789_init(void)
{
    lcd_init_cmd_t st7789_init_cmds[] = {
      {0x11, {0}, 0x08},

      {0x3A, {0x05}, 1},
      {0xC5, {0x1A}, 1},
      {0x36, {0x00}, 1},

      {0xB2, {0x05, 0x05, 0x00, 0x33, 0x33}, 5},
      {0xB7, {0x05}, 1},

      {0xBB, {0x3F}, 1},
      {0xC0, {0x2C}, 1},
      {0xC2, {0x01}, 1},
      {0xC3, {0x0F}, 1},
      {0xC4, {0x20}, 1},
      {0xC6, {0X01}, 1},  // Set to 0x28 if your display is flipped
      {0xD0, {0xa4, 0xa1}, 2},

      {0xE8, {0x03}, 1},
      {0xE9, {0x09, 0x09, 0x08}, 3},

      {0xE0,
       {0xD0, 0x05, 0x09, 0x09, 0x08, 0x14, 0x28, 0x33, 0x3F, 0x07, 0x13, 0x14,
        0x28, 0x30},
       14},
      {0xE1,
       {0xD0, 0x05, 0x09, 0x09, 0x08, 0x03, 0x24, 0x32, 0x32, 0x3B, 0x14, 0x13,
        0x28, 0x2F},
       14},

      {0x21, {0}, 0},
      {0x29, {0}, 0},

        {0, {0}, 0xff},


    };

    //Initialize non-SPI GPIOs
    gpio_pad_select_gpio(ST7789_DC);
    gpio_set_direction(ST7789_DC, GPIO_MODE_OUTPUT);

#if !defined(ST7789_SOFT_RST)
    gpio_pad_select_gpio(ST7789_RST);
    gpio_set_direction(ST7789_RST, GPIO_MODE_OUTPUT);
#endif

    //Reset the display
#if !defined(ST7789_SOFT_RST)
    gpio_set_level(ST7789_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(ST7789_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
#else
    st7789_send_cmd(ST7789_SWRESET);
#endif

    printf("ST7789 initialization.\n");

    //Send all the commands
    uint16_t cmd = 0;
    while (st7789_init_cmds[cmd].databytes!=0xff) {
        st7789_send_cmd(st7789_init_cmds[cmd].cmd);
        st7789_send_data(st7789_init_cmds[cmd].data, st7789_init_cmds[cmd].databytes&0x1F);
        if (st7789_init_cmds[cmd].databytes & 0x80) {
                vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }

    st7789_set_orientation(0);
}

/* The ST7789 display controller can drive up to 320*240 displays, when using a 240*240 or 240*135
 * displays there's a gap of 80px or 40/52/53px respectively. 52px or 53x offset depends on display orientation.
 * We need to edit the coordinates to take into account those gaps, this is not necessary in all orientations. */
void st7789_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map)
{
    uint8_t data[4] = {0};

    uint16_t offsetx1 = area->x1;
    uint16_t offsetx2 = area->x2;
    uint16_t offsety1 = area->y1;
    uint16_t offsety2 = area->y2;

#if (CONFIG_LV_TFT_DISPLAY_OFFSETS)
    offsetx1 += CONFIG_LV_TFT_DISPLAY_X_OFFSET;
    offsetx2 += CONFIG_LV_TFT_DISPLAY_X_OFFSET;
    offsety1 += CONFIG_LV_TFT_DISPLAY_Y_OFFSET;
    offsety2 += CONFIG_LV_TFT_DISPLAY_Y_OFFSET;

#elif (LV_HOR_RES_MAX == 240) && (LV_VER_RES_MAX == 240)
    #if (CONFIG_LV_DISPLAY_ORIENTATION_PORTRAIT)
        offsetx1 += 80;
        offsetx2 += 80;
    #elif (CONFIG_LV_DISPLAY_ORIENTATION_LANDSCAPE_INVERTED)
        offsety1 += 80;
        offsety2 += 80;
    #endif
#elif (LV_HOR_RES_MAX == 240) && (LV_VER_RES_MAX == 135)
    #if (CONFIG_LV_DISPLAY_ORIENTATION_PORTRAIT) || \
        (CONFIG_LV_DISPLAY_ORIENTATION_PORTRAIT_INVERTED)
        offsetx1 += 40;
        offsetx2 += 40;
        offsety1 += 53;
        offsety2 += 53;
    #endif
#elif (LV_HOR_RES_MAX == 135) && (LV_VER_RES_MAX == 240)
    #if (CONFIG_LV_DISPLAY_ORIENTATION_LANDSCAPE) || \
        (CONFIG_LV_DISPLAY_ORIENTATION_LANDSCAPE_INVERTED)
        offsetx1 += 52;
        offsetx2 += 52;
        offsety1 += 40;
        offsety2 += 40;
    #endif
#endif

    /*Column addresses*/
    st7789_send_cmd(ST7789_CASET);
    data[0] = (offsetx1 >> 8) & 0xFF;
    data[1] = offsetx1 & 0xFF;
    data[2] = (offsetx2 >> 8) & 0xFF;
    data[3] = offsetx2 & 0xFF;
    st7789_send_data(data, 4);

    /*Page addresses*/
    st7789_send_cmd(ST7789_RASET);
    data[0] = (offsety1 >> 8) & 0xFF;
    data[1] = offsety1 & 0xFF;
    data[2] = (offsety2 >> 8) & 0xFF;
    data[3] = offsety2 & 0xFF;
    st7789_send_data(data, 4);

    /*Memory write*/
    st7789_send_cmd(ST7789_RAMWR);

    size_t size = (size_t)lv_area_get_width(area) * (size_t)lv_area_get_height(area);

    st7789_send_color((void*)color_map, size * 2);

}

/**********************
 *   STATIC FUNCTIONS
 **********************/
void st7789_send_cmd(uint8_t cmd)
{
    disp_wait_for_pending_transactions();
    gpio_set_level(ST7789_DC, 0);
    disp_spi_send_data(&cmd, 1);
}

void st7789_send_data(void * data, uint16_t length)
{
    disp_wait_for_pending_transactions();
    gpio_set_level(ST7789_DC, 1);
    disp_spi_send_data(data, length);
}

static void st7789_send_color(void * data, size_t length)
{
    disp_wait_for_pending_transactions();
    gpio_set_level(ST7789_DC, 1);
    disp_spi_send_colors(data, length);
}

static void st7789_set_orientation(uint8_t orientation) {
  const char *orientation_str[] = {"PORTRAIT", "PORTRAIT_INVERTED", "LANDSCAPE",
                                   "LANDSCAPE_INVERTED"};
    ESP_LOGI(TAG, "Display orientation: %s", orientation_str[orientation]);
  uint8_t data[] = {0x00, 0xC0, 0x60, 0xA0};
    ESP_LOGI(TAG, "0x36 command value: 0x%02X", data[orientation]);

    st7789_send_cmd(ST7789_MADCTL);
    st7789_send_data((void *) &data[orientation], 1);
}

