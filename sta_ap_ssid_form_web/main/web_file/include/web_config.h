/*
 * @Author: letian
 * @Date: 2022-11-30 09:21
 * @LastEditors: letian
 * @LastEditTime: 2023-02-03 11:44
 * @FilePath: \sta_ap_ssid_form_web\main\web_file\include\web_config.h
 * @Description: 
 * Copyright (c) 2022 by letian 1656733965@qq.com, All Rights Reserved. 
 */
#ifndef _WEB_CONFIG
#define _WEB_CONFIG

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_log.h"
#include "cJSON.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"


#define WIFISSIDLEN "WIFISSIDLEN"
#define WIFISSID    "WIFISSID"
#define WIFIPASSLEN "WIFIPASSLEN"
#define WIFIPASS    "WIFIPASS"

typedef struct {
    char *ssid;
    char *passward;
} recv_wifi_buf_t;

extern nvs_handle_t nvs_wifi_config;
extern recv_wifi_buf_t recv_wifi_buf;
extern httpd_handle_t server;

esp_err_t start_wifi_config_server(const char *base_path);

#endif

