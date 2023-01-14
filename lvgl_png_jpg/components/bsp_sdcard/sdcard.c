/* SD card and FAT filesystem 驱动初始化
   sdcard.c
   SD卡的盘符为S
*/

#include "sdcard.h"

static const char *TAG = "SDCARD";

esp_err_t sd_card_init(void){
    esp_err_t ret;

    //SD卡分区与格式化
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    //初始化SPI
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return 0;
    }

    //SD卡挂载与初始化
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return 0;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // SD卡初始化完毕打印存储卡信息
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;

    // // All done, unmount partition and disable SPI peripheral
    // esp_vfs_fat_sdcard_unmount(mount_point, card);
    // ESP_LOGI(TAG, "Card unmounted");
    // //deinitialize the bus after all devices are removed
    // spi_bus_free(host.slot);

    // // Print FAT FS size information
	// size_t bytes_total, bytes_free;
	// get_fatfs_usage(&bytes_total, &bytes_free);
    // fatfs_
	// printf("%s->FAT FS Total: %d MB, Free: %d MB \r\n",TAG, bytes_total / 1024, bytes_free / 1024);

}