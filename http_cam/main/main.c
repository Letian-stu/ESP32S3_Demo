#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "lwip/sockets.h"
#include "esp_camera.h"
#include "esp_http_server.h"

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

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static void initialise_wifi(void);
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void cam_task(void *pvParameters);

#define EXAMPLE_STATIC_IP_ADDR        "192.168.0.99"
#define EXAMPLE_STATIC_NETMASK_ADDR   "255.255.255.0"
#define EXAMPLE_STATIC_GW_ADDR        "192.168.0.1"

static void example_set_static_ip(esp_netif_t *netif)
{
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
        printf( "Failed to stop dhcp client");
        return;
    }
    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));
    ip.ip.addr = ipaddr_addr(EXAMPLE_STATIC_IP_ADDR);
    ip.netmask.addr = ipaddr_addr(EXAMPLE_STATIC_NETMASK_ADDR);
    ip.gw.addr = ipaddr_addr(EXAMPLE_STATIC_GW_ADDR);
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
        printf("Failed to set ip info");
        return;
    }
    printf( "Success to set static ip: %s, netmask: %s, gw: %s\n", EXAMPLE_STATIC_IP_ADDR, EXAMPLE_STATIC_NETMASK_ADDR, EXAMPLE_STATIC_GW_ADDR);
}

void app_main()
{
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    camera_config_t camera_config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sscb_sda = CAM_PIN_SIOD,
        .pin_sscb_scl = CAM_PIN_SIOC,

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

        .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = FRAMESIZE_VGA, 
        .jpeg_quality = 32, //0-63 lower number means higher quality
        .fb_count = 2      //if more than one, i2s runs in continuous mode. Use only with JPEG
    };

    err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        printf("Camera init failed with error 0x%x\r\n", err);
        return;
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    vTaskDelay(300);
    initialise_wifi();

    
}

static void initialise_wifi(void)
{
    esp_netif_t *esp_netif;          //保存net信息
    ESP_ERROR_CHECK(esp_netif_init());                                                                                      //初始化底层TCP/IP堆栈
    ESP_ERROR_CHECK(esp_event_loop_create_default());                                                                       //创建默认事件循环
    esp_netif = esp_netif_create_default_wifi_sta();                                                                        //创建???? TCP/IP 堆栈的默认网络接口实例绑定station
    ESP_ERROR_CHECK(esp_event_handler_instance_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, &event_handler, esp_netif, NULL)); //处理Wifi所有事????

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config)); //初始化WiFi驱动程序
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); //设置操作模式
    ESP_ERROR_CHECK(esp_wifi_start());                 //触发wifi事件中的WIFI_EVENT_STA_START信息

    //设置wifi
    wifi_config_t wifi_config =
        {
            .sta = {
                .ssid = "home",
                .password = "1656733975",
                .scan_method = WIFI_FAST_SCAN,
                .bssid_set = 0,
                .channel = 0,
                .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
                .threshold.rssi = -70,
                .threshold.authmode = WIFI_AUTH_OPEN,
            },
        };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    //printf("ESP:%s,%d\r\n", event_base, event_id);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        example_set_static_ip(arg);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) //断开后重
    {
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) //获取到IP地址
    {
        xTaskCreate(&cam_task, "cam_task", 4096*2, NULL, 5, NULL); 
    }
}

/*
 * MJPEG stream handler on http://<host>/
 */
esp_err_t jpg_stream_httpd_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t *_jpg_buf;
    char *part_buf[64];
    static int64_t last_frame = 0;
    if (!last_frame)
    {
        last_frame = esp_timer_get_time();
    }
    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
    {
        return res;
    }
    while (1)
    {
        fb = esp_camera_fb_get();
        if (!fb)
        {
            printf(("Camera capture failed"));
            res = ESP_FAIL;
        }
        else
        {
            if (fb->format != PIXFORMAT_JPEG)
            {
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                if (!jpeg_converted)
                {
                    printf(("JPEG compression failed"));
                    esp_camera_fb_return(fb);
                    res = ESP_FAIL;
                }
            }
            else
            {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }
        if (res == ESP_OK)
        {
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (fb->format != PIXFORMAT_JPEG)
        {
            free(_jpg_buf);
        }
        esp_camera_fb_return(fb);
        if (res != ESP_OK)
        {
            break;
        }

        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        printf("MJPG: %u KB %u ms (%.1ffps)\n",
               (uint32_t)(_jpg_buf_len / 1024),
               (uint32_t)(frame_time),
               1000.0 / (uint32_t)(frame_time));

    }
    last_frame = 0;
    return res;
}

/* URI handler structure for GET / */
httpd_uri_t uri_get_mjpeg = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = jpg_stream_httpd_handler,
    .user_ctx = NULL
};

/* 启动 Web 服务器的函数 */
httpd_handle_t start_webserver(void)
{
    /* 生成默认的配置参*/
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 8081;
    /* 置空 esp_http_server 的实例句 */
    httpd_handle_t server = NULL;
    /* 启动 httpd server */
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get_mjpeg);
    }
    /* 如果服务器启动失败，返回的句柄是 NULL */
    return server;
}

static void cam_task(void *pvParameters)
{
    //启动 Web 服务
    start_webserver();
    while (1)
    {
        //printf("cam_task!\r\n");
        vTaskDelay(5000);
    }
}

