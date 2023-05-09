/**
 * @file lvgl_spi_conf.h
 *
 */

#ifndef LVGL_SPI_CONF_H
#define LVGL_SPI_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
// DISPLAY PINS
#define DISP_SPI_MOSI CONFIG_LV_DISP_SPI_MOSI
#if defined (CONFIG_LV_DISPLAY_USE_SPI_MISO)
    #define DISP_SPI_MISO CONFIG_LV_DISP_SPI_MISO
    #define DISP_SPI_INPUT_DELAY_NS CONFIG_LV_DISP_SPI_INPUT_DELAY_NS
#else
    #define DISP_SPI_MISO (-1)
    #define DISP_SPI_INPUT_DELAY_NS (0)
#endif
#if defined(CONFIG_LV_DISP_SPI_IO2)
#define DISP_SPI_IO2 CONFIG_LV_DISP_SPI_IO2
#else
#define DISP_SPI_IO2 (-1)
#endif
#if defined(CONFIG_LV_DISP_SPI_IO3)
#define DISP_SPI_IO3 CONFIG_LV_DISP_SPI_IO3
#else
#define DISP_SPI_IO3 (-1)
#endif
#define DISP_SPI_CLK CONFIG_LV_DISP_SPI_CLK
#if defined (CONFIG_LV_DISPLAY_USE_SPI_CS)
    #define DISP_SPI_CS CONFIG_LV_DISP_SPI_CS
#else
    #define DISP_SPI_CS (-1)
#endif

/* Define TOUCHPAD PINS when selecting a touch controller */
#if !defined (CONFIG_LV_TOUCH_CONTROLLER_NONE)

/* Handle FT81X special case */
#if defined (CONFIG_LV_TFT_DISPLAY_CONTROLLER_FT81X) && \
    defined (CONFIG_LV_TOUCH_CONTROLLER_FT81X)

    #define TP_SPI_MOSI CONFIG_LV_DISP_SPI_MOSI
    #define TP_SPI_MISO CONFIG_LV_DISP_SPI_MISO
    #define TP_SPI_CLK  CONFIG_LV_DISP_SPI_CLK
    #define TP_SPI_CS   CONFIG_LV_DISP_SPI_CS
#else
    #define TP_SPI_MOSI CONFIG_LV_TOUCH_SPI_MOSI
    #define TP_SPI_MISO CONFIG_LV_TOUCH_SPI_MISO
    #define TP_SPI_CLK  CONFIG_LV_TOUCH_SPI_CLK
    #define TP_SPI_CS   CONFIG_LV_TOUCH_SPI_CS
#endif
#endif

#define ENABLE_TOUCH_INPUT  CONFIG_LV_ENABLE_TOUCH

#if defined (CONFIG_LV_TFT_DISPLAY_SPI2_HOST)
#define TFT_SPI_HOST SPI2_HOST
#elif defined (CONFIG_LV_TFT_DISPLAY_SPI3_HOST)
#define TFT_SPI_HOST SPI3_HOST
#endif

#if defined (CONFIG_LV_TFT_DISPLAY_SPI_HALF_DUPLEX)
#define DISP_SPI_HALF_DUPLEX
#else
#define DISP_SPI_FULL_DUPLEX
#endif

#if defined (CONFIG_LV_TFT_DISPLAY_SPI_TRANS_MODE_DIO)
#define DISP_SPI_TRANS_MODE_DIO
#elif defined (CONFIG_LV_TFT_DISPLAY_SPI_TRANS_MODE_QIO)
#define DISP_SPI_TRANS_MODE_QIO
#else
#define DISP_SPI_TRANS_MODE_SIO
#endif



/**********************
 *      TYPEDEFS
 **********************/

#define SPI_BUS_MAX_TRANSFER_SZ (LV_HOR_RES_MAX * 60 * 2)
#define SPI_TFT_CLOCK_SPEED_HZ ((80 * 1000 * 1000) / CONFIG_LV_TFT_CUSTOM_SPI_CLK_DIVIDER)
#define SPI_TFT_SPI_MODE    (2)
/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LVGL_SPI_CONF_H*/
