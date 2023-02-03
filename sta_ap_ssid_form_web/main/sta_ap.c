#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "web_config.h"

static const char *TAG = "sta_ap";

#define ESP_FS_PATH        "/fs"

typedef struct {
    char ssid[32];
    uint8_t ssidlen;
    char pass[32];
    uint8_t passlen;
} read_wifi_buf_t;

read_wifi_buf_t read_wifi_buf;

typedef struct {
    wifi_sta_config_t sta;
    wifi_ap_config_t ap;
} sta_ap_wifi_config_t;

static sta_ap_wifi_config_t sta_ap_wifi_config = {
    .sta = {
        .ssid = "home",
        .password = "1656733975",
    },
    .ap = {
        .ssid = "Wifi_Config",
        .password = "wificonfig",
        .max_connection = 1,
        .authmode = WIFI_AUTH_WPA_WPA2_PSK,
    },
};

static void wifi_sta_ap_cb(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",MAC2STR(event->mac), event->aid);
    } 
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",MAC2STR(event->mac), event->aid);
    }
    else if(event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP IPv4 address: " IPSTR, IP2STR(&event->ip_info.ip));
    }
    else if(event_id == WIFI_EVENT_STA_DISCONNECTED)   
    {
        static int count = 5;
        esp_err_t err;
        ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
        if (count > 0) 
        {
            err = esp_wifi_connect();
            if (err == ESP_ERR_WIFI_NOT_STARTED) {
                return;
            }
            count--;
        } 
        else 
        {
            ESP_LOGE(TAG, "try max connect count, but failed");
        }
    }
}

esp_err_t wifi_ap_sta_init(void)
{
    static tcpip_adapter_ip_info_t local_ip;
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));

    esp_netif_config_t netif_config =  {                                                
        .base = ESP_NETIF_BASE_DEFAULT_WIFI_AP,      
        .driver = NULL,                              
        .stack = ESP_NETIF_NETSTACK_DEFAULT_WIFI_AP, 
    };
    esp_netif_t *netif = esp_netif_new(&netif_config);
    esp_netif_attach_wifi_ap(netif);

    netif_config.base = ESP_NETIF_BASE_DEFAULT_WIFI_STA;
    netif_config.stack = ESP_NETIF_NETSTACK_DEFAULT_WIFI_STA;
    netif = esp_netif_new(&netif_config);
    esp_netif_attach_wifi_station(netif);

    esp_wifi_set_default_wifi_sta_handlers(); 

    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &wifi_sta_ap_cb, NULL);
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &wifi_sta_ap_cb, NULL);
    esp_wifi_set_config(ESP_IF_WIFI_AP, (wifi_config_t*)&sta_ap_wifi_config.ap);

    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_sta_ap_cb, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_sta_ap_cb, NULL);
    esp_wifi_set_config(ESP_IF_WIFI_STA, (wifi_config_t*)&sta_ap_wifi_config.sta);

    esp_wifi_connect();
    uint8_t ret = tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &local_ip);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "wificonfig IP:" IPSTR "", IP2STR(&local_ip.ip));
    }
    return ESP_OK;
}

esp_err_t read_wifi_from_nvs(void)
{
    esp_err_t err = nvs_open("nvs", NVS_READWRITE, &nvs_wifi_config);
    if (err != ESP_OK)
    {
        return ESP_FAIL;
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        //read
        ESP_LOGI(TAG, "=========nvs read data==========");
        err = nvs_get_u8(nvs_wifi_config, WIFISSIDLEN, &read_wifi_buf.ssidlen);
        if(err != ESP_OK)
        {
            ESP_LOGE(TAG, "read len Failed!");
            return ESP_FAIL;
        }
        err = nvs_get_str(nvs_wifi_config, WIFISSID, read_wifi_buf.ssid, (size_t*)(&read_wifi_buf.ssidlen));
        if(err != ESP_OK)
        {
            ESP_LOGE(TAG, "read name Failed!");
            return ESP_FAIL;
        }
        err = nvs_get_u8(nvs_wifi_config, WIFIPASSLEN, &read_wifi_buf.passlen);
        if(err != ESP_OK)
        {   
            ESP_LOGE(TAG, "read len Failed!");
            return ESP_FAIL;
        }
        err = nvs_get_str(nvs_wifi_config, WIFIPASS, read_wifi_buf.pass, (size_t*)(&read_wifi_buf.passlen));
        if(err != ESP_OK)
        {
            ESP_LOGE(TAG, "read ssid Failed!");
            return ESP_FAIL;
        }
        ESP_LOGI(TAG, "wifiname:len=%d,data=%s",read_wifi_buf.ssidlen,read_wifi_buf.ssid);
        ESP_LOGI(TAG, "wifissid:len=%d,data=%s",read_wifi_buf.passlen,read_wifi_buf.pass);
        ESP_LOGI(TAG, "=======================================");
        // Close
        nvs_close(nvs_wifi_config);
    }
    return ESP_OK;
}

/* Function to initialize SPIFFS */
esp_err_t mount_storage(const char* base_path)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = base_path,
        .partition_label = NULL,
        .max_files = 1,   // This sets the maximum number of files that can be open at the same time
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    return ESP_OK;
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    if(read_wifi_from_nvs() == ESP_OK) 
    {
        //ESP_LOGI(TAG, "wifi data from nvs");
        strncpy ( (char *)sta_ap_wifi_config.sta.ssid, read_wifi_buf.ssid, read_wifi_buf.ssidlen );
        strncpy ( (char *)sta_ap_wifi_config.sta.password, read_wifi_buf.pass, read_wifi_buf.passlen );
        //ESP_LOGI(TAG, "ssid=%s pass=%s",sta_ap_wifi_config.sta.ssid,sta_ap_wifi_config.sta.password);
    }
    wifi_ap_sta_init();

    mount_storage(ESP_FS_PATH);
    start_wifi_config_server(ESP_FS_PATH);
}

