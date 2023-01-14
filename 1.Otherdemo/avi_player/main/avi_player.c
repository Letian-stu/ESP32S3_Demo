/*
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-06-05 10:13:51
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-04-27 16:46:35
 * @FilePath: \SP_DEMO\14.avi_player\main\avi_player.c
 */
#include "avi_player.h"
#include "app_main.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>

#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"
#include "lv_port_indev.h"

#include <esp_system.h>
#include "esp_log.h"
#include "lv_port_indev.h"
#include "file_manager.h"

#include "avifile.h"
#include "vidoplayer.h"
#include "mjpeg.h"
#include "driver/i2s.h"

#define TAG "avi_player"

extern AVI_TypeDef AVI_file;
lv_obj_t *img_cam; //要显示图像
lv_img_dsc_t img_dsc = {
	.header.always_zero = 0,
	.header.w = 240,
	.header.h = 240,
	.data_size = 240 * 240 * 2,
	.header.cf = LV_IMG_CF_TRUE_COLOR,
	.data = NULL,
};
#define filename "/sdcard/badapple.avi"

#define IIS_SCLK 16
#define IIS_LCLK 7
#define IIS_DSIN 6
#define IIS_DOUT 15

#define I2S_NUM 1

void play_i2s_init(void)
{
	i2s_config_t i2s_config = {
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
		.sample_rate = 44100,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
		.communication_format = I2S_COMM_FORMAT_STAND_I2S,
		.intr_alloc_flags = ESP_INTR_FLAG_LOWMED,
		.dma_buf_count = 6,
		.dma_buf_len = 1024,
		.bits_per_chan = I2S_BITS_PER_SAMPLE_16BIT};
	i2s_pin_config_t pin_config = {
		.mck_io_num = -1,
		.bck_io_num = IIS_SCLK,	  // IIS_SCLK
		.ws_io_num = IIS_LCLK,	  // IIS_LCLK
		.data_out_num = IIS_DSIN, // IIS_DSIN
		.data_in_num = -1		  // IIS_DOUT
	};
	i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
	i2s_set_pin(I2S_NUM, &pin_config);
	i2s_zero_dma_buffer(I2S_NUM);
}

void Cam_Task(void *pvParameters)
{

	// /* 入口处检测一次 */
	ESP_LOGI(TAG, "Run Run uxHighWaterMark = %d", uxTaskGetStackHighWaterMark(NULL));
	FILE *fp = NULL;
	portTickType xLastWakeTime;
	fm_sdcard_init();
	vTaskDelay(100 / portTICK_PERIOD_MS);
	// char *dis_buf = malloc(320 * 240 * 2);

	FILE *avi_file;
	int ret;
	size_t BytesRD;
	uint32_t Strsize;
	uint32_t Strtype;
	uint8_t *pbuffer;
	uint32_t buffer_size = 22 * 1024;

	avi_file = fopen(filename, "rb");
	if (avi_file == NULL)
	{
		ESP_LOGE(TAG, "Cannot open %s", filename);
		return;
	}

	pbuffer = malloc(buffer_size);
	if (pbuffer == NULL)
	{
		ESP_LOGE(TAG, "Cannot alloc memory for palyer");
		fclose(avi_file);
		return;
	}

	BytesRD = fread(pbuffer, 20480, 1, avi_file);
	ret = AVI_Parser(pbuffer, BytesRD);
	if (0 > ret)
	{
		ESP_LOGE(TAG, "parse failed (%d)", ret);
		return;
	}
	// audio_init();
	// pwm_audio_set_param(AVI_file.auds_sample_rate, AVI_file.auds_bits, AVI_file.auds_channels);
	// pwm_audio_start();
	ESP_LOGI(TAG, "frame_rate=%d, ch=%d, width=%d", AVI_file.auds_sample_rate, AVI_file.auds_channels, AVI_file.auds_bits);
	play_i2s_init();
	i2s_set_clk(I2S_NUM, AVI_file.auds_sample_rate, AVI_file.auds_bits, AVI_file.auds_channels);

	uint16_t img_width = AVI_file.vids_width;
	uint16_t img_height = AVI_file.vids_height;
	uint8_t *img_rgb888 = heap_caps_malloc(img_width * img_height * 2, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);//MALLOC_CAP_SPIRAM
	if (NULL == img_rgb888)
	{
		ESP_LOGE(TAG, "malloc for rgb888 failed");
		// goto EXIT;
	}

	fseek(avi_file, AVI_file.movi_start, SEEK_SET); // 偏移到movi list
	Strsize = read_frame(avi_file, pbuffer, buffer_size, &Strtype);
	BytesRD = Strsize + 8;

	while (1)
	{ //播放循环
		if (BytesRD >= AVI_file.movi_size)
		{
			ESP_LOGI(TAG, "paly end");
			break;
		}
		if (Strtype == T_vids)
		{ //显示帧
			int64_t fr_end = esp_timer_get_time();
			mjpegdraw(pbuffer, Strsize, img_rgb888);

			// jpg2rgb565((const uint8_t *)pbuffer, Strsize, img_rgb888, JPG_SCALE_NONE);
			ESP_LOGI(TAG, "FPS %u", 1000 / ((uint32_t)((esp_timer_get_time() - fr_end) / 1000)));
			fr_end = esp_timer_get_time();
			img_dsc.data = (uint8_t *)img_rgb888;
			lv_img_set_src(img_cam, &img_dsc);
			// g_lcd.draw_bitmap(0, 0, img_width, img_height, img_rgb888);
			// ESP_LOGI(TAG, "draw %ums", (uint32_t)((esp_timer_get_time() - fr_end) / 1000));fr_end = esp_timer_get_time();

		} //显示帧
		else if (Strtype == T_auds)
		{ //音频输出
			size_t cnt;
			i2s_write(I2S_NUM, (uint8_t *)pbuffer, Strsize, &cnt, 2 / portTICK_PERIOD_MS);

			// pwm_audio_write((uint8_t *)pbuffer, Strsize, &cnt, 500 / portTICK_PERIOD_MS);
		}
		else
		{
			ESP_LOGE(TAG, "unknow frame");
			break;
		}
		Strsize = read_frame(avi_file, pbuffer, buffer_size, &Strtype); //读入整帧
		ESP_LOGD(TAG, "type=%x, size=%d", Strtype, Strsize);
		BytesRD += Strsize + 8;
	}
	// never reach
	while (1)
	{
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

void imgcam_init(void)
{

	img_cam = lv_img_create(lv_scr_act(), NULL);
	static lv_style_t style_img;
	lv_style_init(&style_img);

	//Write style state: LV_STATE_DEFAULT for style_img
	lv_style_set_image_recolor(&style_img, LV_STATE_DEFAULT, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_image_recolor_opa(&style_img, LV_STATE_DEFAULT, 0);
	lv_style_set_image_opa(&style_img, LV_STATE_DEFAULT, 255);
	lv_obj_add_style(img_cam, LV_IMG_PART_MAIN, &style_img);
	lv_obj_set_pos(img_cam, 0, 0);
	lv_obj_set_size(img_cam, 240, 240);
}
void avi_player_load()
{
	// app_camera_init();
	vTaskDelay(100);
	imgcam_init();
	vTaskDelay(1000);

	xTaskCreatePinnedToCore(&Cam_Task, "Cam_Task", 1024 * 5, NULL, 14, NULL, 0);

}

