#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "esp_camera.h"

#define TAG "CAM"

#define CAM_PIN_PWDN 15
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 12
#define CAM_PIN_SIOD 14
#define CAM_PIN_SIOC 13
#define CAM_PIN_D7 9
#define CAM_PIN_D6 8
#define CAM_PIN_D5 7
#define CAM_PIN_D4 5
#define CAM_PIN_D3 3
#define CAM_PIN_D2 1
#define CAM_PIN_D1 2
#define CAM_PIN_D0 4
#define CAM_PIN_VSYNC 11
#define CAM_PIN_HREF 10
#define CAM_PIN_PCLK 6

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_HD, // QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 12, // 0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 2,      // When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
};

#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO 40
#define PIN_NUM_MOSI 38
#define PIN_NUM_CLK 39
#define PIN_NUM_CS 37

int app_main(void)
{
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return ESP_FAIL;
    }
    
    // initialize the sdcard
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ESP_FAIL;
    }
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;
    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    sdmmc_card_print_info(stdout, card);



    const char *file_log = MOUNT_POINT "/log.txt";
    ESP_LOGI(TAG, "Opening file %s", file_log);
    FILE *f = fopen(file_log, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "%lld:Init:\n", esp_timer_get_time());
    fclose(f);
    ESP_LOGI(TAG, "File written");

    uint32_t pictureNumber = 0;
    char file_name[30];
    size_t writelen = 0;
    while (1)
    {
        ESP_LOGI(TAG, "Taking picture...");
        camera_fb_t *pic = esp_camera_fb_get();

        // First create a file.
        sprintf(file_name, MOUNT_POINT "/img%d.jpg", pictureNumber);
        ESP_LOGI(TAG, "Opening file %s", file_name);
        FILE *f = fopen(file_name, "w");
        if (f == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file for writing");
            return ESP_FAIL;
        }
        writelen = fwrite(pic->buf, sizeof(pic->buf), pic->len, f);
        if (writelen != pic->len)
        {
            ESP_LOGE(TAG, "img Write err");
        }
        else
        {
            ESP_LOGI(TAG, "write buff len %d byte", writelen);
            pictureNumber++;
        }
        fclose(f);
        ESP_LOGI(TAG, "img written");

        if (pictureNumber > 10)
        {
            esp_vfs_fat_sdcard_unmount(mount_point, card);
            ESP_LOGI(TAG, "Card unmounted");
            spi_bus_free(host.slot);
            esp_camera_fb_return(pic);
            return 0;
        }

        esp_camera_fb_return(pic);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
