#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "string.h"
#include "esp_log.h"
#include "file_manage.h"
#include "avi_def.h"
#include "esp_camera.h"

static const char *TAG = "avi recorder";

#define MEM_ALIGNMENT 4
#define MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) & ~(MEM_ALIGNMENT-1))

#define MAKE_FOURCC(a, b, c, d) ((uint32_t)(d)<<24 | (uint32_t)(c)<<16 | (uint32_t)(b)<<8 | (uint32_t)(a))

typedef struct {
    const char *fname;
    framesize_t rec_size;
    uint32_t rec_time;
}recorder_param_t;

typedef struct {
    char filename[64];  // filename for temporary
    FILE *avifile;      // avi file
    FILE *idxfile;      // storage the size of each image
    uint32_t nframes;   // the number of frame
    uint32_t totalsize; // all frame image size
} jpeg2avi_data_t;

typedef enum {
    REC_STATE_IDLE,
    REC_STATE_BUSY,
}record_state_t;

static uint8_t g_force_end=0;
static record_state_t g_state = REC_STATE_IDLE;

static int jpeg2avi_start(jpeg2avi_data_t *j2a, const char *filename)
{
    ESP_LOGI(TAG, "Starting an avi [%s]", filename);
    if (strlen(filename) > sizeof(j2a->filename)-5) {
        ESP_LOGE(TAG, "The given file name is too long");
        return ESP_ERR_INVALID_ARG;
    }
    
    memset(j2a->filename, 0, sizeof(j2a->filename));
    strcpy(j2a->filename, filename);

    j2a->avifile = fopen(j2a->filename, "w");
    if (j2a->avifile == NULL)  {
        ESP_LOGE(TAG, "Could not open %s", j2a->filename);
        return ESP_FAIL;
    }

    strcat(j2a->filename, ".idx");
    j2a->idxfile = fopen(j2a->filename, "w");
    if (j2a->idxfile == NULL)  {
        ESP_LOGE(TAG, "Could not open %s", j2a->filename);
        fclose(j2a->avifile);
        unlink(filename);
        return ESP_FAIL;
    }

    uint32_t offset1 = sizeof(AVI_LIST_HEAD);  //riff head大小
    uint32_t offset2 = sizeof(AVI_HDRL_LIST);  //hdrl list大小
    uint32_t offset3 = sizeof(AVI_LIST_HEAD);  //movi list head大小

    //AVI文件偏移量设置到movi list head后，从该位置向后依次写入JPEG数据
    int ret = fseek(j2a->avifile, offset1 + offset2 + offset3, SEEK_SET);
    if (0 != ret) {
        ESP_LOGE(TAG, "seek avi file failed");
        fclose(j2a->avifile);
        fclose(j2a->idxfile);
        unlink(filename);
        unlink(j2a->filename);
        return ESP_FAIL;
    }

    j2a->nframes = 0;
    j2a->totalsize = 0;
    return ESP_OK;
}

static int jpeg2avi_add_frame(jpeg2avi_data_t *j2a, void *data, uint32_t len)
{
    size_t ret;
    AVI_CHUNK_HEAD frame_head;
    uint32_t align_size = MEM_ALIGN_SIZE(len);/*JPEG图像大小4字节对齐*/

    frame_head.FourCC = MAKE_FOURCC('0', '0', 'd', 'c'); //00dc = 压缩的视频数据
    frame_head.size = align_size;
    ret = fwrite(&frame_head, sizeof(AVI_CHUNK_HEAD), 1, j2a->avifile);   //写入4字节对齐后的JPEG图像大小
    ret = fwrite(data, align_size, 1, j2a->avifile);        //写入真正的JPEG数据
    if (1 != ret) {
        ESP_LOGE(TAG, "frame chunk write failed");
        return ESP_FAIL;
    }
    /*将4字节对齐后的JPEG图像大小保存*/
    fwrite(&align_size, 4, 1, j2a->idxfile);

    j2a->nframes += 1;
    j2a->totalsize += align_size;
    return ESP_OK;
}

static int back_fill_data(jpeg2avi_data_t *j2a, uint32_t width, uint32_t height, uint32_t fps)
{
    size_t ret;

    AVI_LIST_HEAD riff_head = {
        .List = MAKE_FOURCC('R', 'I', 'F', 'F'),
        .size = 4 + sizeof(AVI_HDRL_LIST) + sizeof(AVI_LIST_HEAD) + j2a->nframes * 8 + j2a->totalsize + (sizeof(AVI_IDX1) * j2a->nframes)+8,
        .FourCC = MAKE_FOURCC('A', 'V', 'I', ' ')
    };

    AVI_HDRL_LIST hdrl_list = {
        {
            .List = MAKE_FOURCC('L', 'I', 'S', 'T'),
            .size = sizeof(AVI_HDRL_LIST) - 8,
            .FourCC = MAKE_FOURCC('h', 'd', 'r', 'l'),
        },
        {
            .FourCC = MAKE_FOURCC('a', 'v', 'i', 'h'),
            .size = sizeof(AVI_AVIH_CHUNK) - 8,
            .us_per_frame = 1000000 / fps,
            .max_bytes_per_sec = (width * height * 2) / 10,
            .padding = 0,
            .flags = 0,
            .total_frames = j2a->nframes,
            .init_frames = 0,
            .streams = 1,
            .suggest_buff_size = (width * height * 2),
            .width = width,
            .height = height,
            .reserved = {0, 0, 0, 0},
        },
        {
            {
                .List = MAKE_FOURCC('L', 'I', 'S', 'T'),
                .size = sizeof(AVI_STRL_LIST) - 8,
                .FourCC = MAKE_FOURCC('s', 't', 'r', 'l'),
            },
            {
                .FourCC = MAKE_FOURCC('s', 't', 'r', 'h'),
                .size = sizeof(AVI_STRH_CHUNK) - 8,
                .fourcc_type = MAKE_FOURCC('v', 'i', 'd', 's'),
                .fourcc_codec = MAKE_FOURCC('M', 'J', 'P', 'G'),
                .flags = 0,
                .priority = 0,
                .language = 0,
                .init_frames = 0,
                .scale = 1,
                .rate = fps, //rate / scale = fps
                .start = 0,
                .length = j2a->nframes,
                .suggest_buff_size = (width * height * 2),
                .quality = 1,
                .sample_size = 0,
                .rcFrame = {0, 0, width, height},
            },
            {
                .FourCC = MAKE_FOURCC('s', 't', 'r', 'f'),
                .size = sizeof(AVI_VIDS_STRF_CHUNK) - 8,
                .size1 = sizeof(AVI_VIDS_STRF_CHUNK) - 8,
                .width = width,
                .height = height,
                .planes = 1,
                .bitcount = 24,
                .fourcc_compression = MAKE_FOURCC('M', 'J', 'P', 'G'),
                .image_size = width *height * 3,
                .x_pixels_per_meter = 0,
                .y_pixels_per_meter = 0,
                .num_colors = 0,
                .imp_colors = 0,
            }
        }
    };

    AVI_LIST_HEAD movi_list_head = {
        .List = MAKE_FOURCC('L', 'I', 'S', 'T'),
        .size = 4 + j2a->nframes * 8 + j2a->totalsize,
        .FourCC = MAKE_FOURCC('m', 'o', 'v', 'i')
    };

    //定位到文件头，回填各块数据
    fseek(j2a->avifile, 0, SEEK_SET);
    fwrite(&riff_head, sizeof(AVI_LIST_HEAD), 1, j2a->avifile);
    fwrite(&hdrl_list, sizeof(AVI_HDRL_LIST), 1, j2a->avifile);
    ret = fwrite(&movi_list_head, sizeof(AVI_LIST_HEAD), 1, j2a->avifile);
    if (1 != ret) {
        ESP_LOGE(TAG, "avi head list write failed");
        return ESP_FAIL;
    }
    return ESP_OK;
}

static int write_index_chunk(jpeg2avi_data_t *j2a)
{
    size_t ret;
    size_t i;
    uint32_t index = MAKE_FOURCC('i', 'd', 'x', '1');  //索引块ID
    uint32_t index_chunk_size = sizeof(AVI_IDX1) * j2a->nframes;   //索引块大小
    uint32_t offset = 4;
    uint32_t frame_size;
    AVI_IDX1 idx;

    j2a->idxfile = fopen(j2a->filename, "rb");
    if (j2a->idxfile == NULL)  {
        ESP_LOGE(TAG, "%d:Could not open %s. discard the idx1 chunk", __LINE__, j2a->filename);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "frame number=%d", j2a->nframes);
    fwrite(&index, 4, 1, j2a->avifile);
    fwrite(&index_chunk_size, 4, 1, j2a->avifile);

    idx.FourCC = MAKE_FOURCC('0', '0', 'd', 'c'); //00dc = 压缩的视频数据
    for (i = 0; i < j2a->nframes; i++) {
        fread(&frame_size, 4, 1, j2a->idxfile); //Read size of each jpeg image
        idx.flags = 0x10;//0x10表示当前帧为关键帧
        idx.chunkoffset = offset;
        idx.chunklength = frame_size;
        ret = fwrite(&idx, sizeof(AVI_IDX1), 1, j2a->avifile);
        if (1 != ret) {
            break;
        }
        offset = offset + frame_size + 8;
    }
    fclose(j2a->idxfile);
    unlink(j2a->filename);
    if (i != j2a->nframes) {
        ESP_LOGE(TAG, "avi index write failed");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

static void jpeg2avi_end(jpeg2avi_data_t *j2a, int width, int height, int fps)
{
    ESP_LOGI(TAG, "video info: width=%d | height=%d | fps=%d", width, height, fps);
    fclose(j2a->idxfile);
    //写索引块
    write_index_chunk(j2a);

    //从文件头开始，回填各块数据
    back_fill_data(j2a, width, height, fps);
    fclose(j2a->avifile);

    ESP_LOGI(TAG, "avi recording completed");
}

static void recorder_task(void *args)
{
    int ret;
    recorder_param_t *rec_arg = (recorder_param_t*)args;

    jpeg2avi_data_t avi_recoder;
    camera_fb_t *image_fb = NULL;
    ret = jpeg2avi_start(&avi_recoder, "/sdcard/recorde.avi");
    if (0 != ret) {
        ESP_LOGE(TAG, "start failed");
        vTaskDelete(NULL);
    }

    g_state = REC_STATE_BUSY;
    sensor_t *cam_sensor = esp_camera_sensor_get();
    cam_sensor->set_pixformat(cam_sensor, rec_arg->rec_size);

    uint64_t fr_start = esp_timer_get_time()/1000;
    uint64_t end_time = rec_arg->rec_time*1000 + fr_start;
    uint64_t printf_time=fr_start;
    while (1) {
        image_fb = esp_camera_fb_get();
        if (!image_fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }

        ret = jpeg2avi_add_frame(&avi_recoder, image_fb->buf, image_fb->len);
        esp_camera_fb_return(image_fb);
        if (0 != ret) {
            break;
        }

        uint64_t t = esp_timer_get_time()/1000;
        if(t-printf_time > 1000) {
            printf_time = t;
            ESP_LOGI(TAG, "recording %d/%d s", (uint32_t)((t-fr_start)/1000), rec_arg->rec_time);
        }
        if (t > end_time || g_force_end) {
            break;
        }
    }
    uint32_t fps = avi_recoder.nframes * 1000 / (esp_timer_get_time()/1000 - fr_start);
    jpeg2avi_end(&avi_recoder, resolution[rec_arg->rec_size].width, resolution[rec_arg->rec_size].height, fps);
    g_state = REC_STATE_IDLE;
    g_force_end = 0;
    vTaskDelete(NULL);
}

void avi_recorder_start(const char *fname, framesize_t rec_size, uint32_t rec_time)
{
    if (REC_STATE_IDLE != g_state) {
        ESP_LOGE(TAG, "recorder already running");
        return;
    }
    g_force_end = 0;
    static recorder_param_t rec_arg = {0};
    rec_arg.fname = fname;
    rec_arg.rec_size = rec_size;
    rec_arg.rec_time = rec_time;

    xTaskCreate(recorder_task,
                "recorder",
                1024*4,
                &rec_arg,
                7,
                NULL
               );
    
    g_state = REC_STATE_BUSY;
}

void avi_recorder_stop(void)
{
    if (REC_STATE_BUSY != g_state) {
        ESP_LOGE(TAG, "recorder already running");
        return;
    }
    g_force_end = 1;
}