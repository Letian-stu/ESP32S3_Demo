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

static const char *TAG = "sta_ap";

typedef struct {
    wifi_sta_config_t sta;
    wifi_ap_config_t ap;
} custom_wifi_config_t;

static custom_wifi_config_t custom_wifi_config = {
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
        ESP_LOGI(TAG, "Got IP event!");
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "IPv4 address: " IPSTR, IP2STR(&event->ip_info.ip));
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
    esp_wifi_set_config(ESP_IF_WIFI_AP, (wifi_config_t*)&custom_wifi_config.ap);

    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_sta_ap_cb, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_sta_ap_cb, NULL);
    esp_wifi_set_config(ESP_IF_WIFI_STA, (wifi_config_t*)&custom_wifi_config.sta);

    esp_wifi_connect();
    return ESP_OK;
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_ap_sta_init();
}