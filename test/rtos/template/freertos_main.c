/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <estd/port/identify_platform.h>

// higher versions of esp-idf are (more) api compatible between ESP32 and ESP8266

// If need be, we can access IDF-VER ala https://github.com/espressif/ESP8266_RTOS_SDK/blob/master/make/project.mk
// OK above was a bit of a pipe dream, since C preprocessor isn't powerful enough to decompose it well
// so instead we rebuild ourselves with special version_finder.mk
#if ESTD_IDF_VER >= ESTD_IDF_VER_2_0_0_644
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#else
#include "esp_misc.h"
#include "esp_sta.h"
#include "esp_system.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void test_task(void*);

void got_ip_event();


#ifdef CONFIG_WIFI_SSID
/******************************************************************************
 * FunctionName : wifi_event_handler_cb
 * Description  : wifi event callback
 * Parameters   : system event
 * Returns      : none
 *******************************************************************************/
#if ESTD_IDF_VER <= ESTD_IDF_VER_2_0_0_444
void wifi_event_handler_cb(System_Event_t * event)
#elif ESTD_IDF_VER <= ESTD_IDF_VER_2_0_0_644
void wifi_event_handler_cb(system_event_t * event)
#else
esp_err_t wifi_event_handler_cb(void* context, system_event_t * event)
#endif
{
    static const char *TAG = "wifi event";

    if (event != NULL) 
    {
    switch (event->event_id) {
#if ESTD_IDF_VER <= ESTD_IDF_VER_2_0_0_444
        case EVENT_STAMODE_GOT_IP:
#else
        case SYSTEM_EVENT_STA_GOT_IP:
#if ESTD_IDF_VER >= ESTD_IDF_VER_2_0_0_740
        case SYSTEM_EVENT_ETH_GOT_IP:
        case SYSTEM_EVENT_GOT_IP6:
#endif
            // 2.0.0-740 never gets here, but arp and ping shows IP address
            // does actually get assigned
            // 3.2.0-708 *does* get here, so I am gonna assume everything over 3.0
            // arrives here
            ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
#endif
            printf("free heap size %d line %d \n", 
#if ESTD_IDF_VER <= ESTD_IDF_VER_2_0_0_444
                system_get_free_heap_size(), 
#else
                // NOTE: Not even sure if this is the right substitute
                // but it looks right
                esp_get_free_heap_size(),
#endif
                __LINE__);

#if ESTD_IDF_VER >= ESTD_IDF_VER_3_0_0
            got_ip_event();
#endif
            break;

#if ESTD_IDF_VER >= ESTD_IDF_VER_2_0_0_644
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "STA_START");
            esp_wifi_connect();
            break;

        case SYSTEM_EVENT_STA_CONNECTED:
            // at this point, doesn't have WLAN-provided address yet
            ESP_LOGI(TAG, "STA_CONNECTED: ip=%s",
                ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "STA_DISCONNECTED");
            esp_wifi_connect();
            break;

#endif

        default:
#if ESTD_IDF_VER >= ESTD_IDF_VER_2_0_0_644
            ESP_LOGI(TAG, "Servicing event: %d", event->event_id);
#endif
            break;

    }
    }
#if ESTD_IDF_VER >= ESTD_IDF_VER_2_0_0_740
    return ESP_OK;
#else
    return;
#endif
}


/*
// unforunately this doesn't work out presumably because it's in the
// same file as freertos_main so can't be non-linked easily... ?
void __attribute__((weak)) got_ip_event()
{

} */

/******************************************************************************
 * FunctionName : wifi_config
 * Description  : wifi_config task
 * Parameters   : task param
 * Returns      : none
 *******************************************************************************/
void wifi_config(void *pvParameters)
{
    static const char *TAG = "wifi config";

    ESP_LOGI(TAG, "wifi_config startup");

#if ESTD_IDF_VER >= ESTD_IDF_VER_2_0_0_644
    tcpip_adapter_init();
#endif

#if ESTD_IDF_VER >= ESTD_IDF_VER_2_0_0_740
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);

#else
    struct ip_info ip_config;
    struct station_config sta_config;
    memset(&sta_config, 0, sizeof(struct station_config));
    wifi_set_opmode(STATION_MODE);
    memcpy(sta_config.ssid, CONFIG_WIFI_SSID, strlen(CONFIG_WIFI_SSID));
    memcpy(sta_config.password, CONFIG_WIFI_PASSWORD, strlen(CONFIG_WIFI_PASSWORD));
    wifi_station_set_config(&sta_config);

    wifi_station_disconnect();
    wifi_station_connect();
#endif

    vTaskDelete( NULL );
}
#endif

#if ESTD_IDF_VER <= ESTD_IDF_VER_2_0_0_444
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
 *******************************************************************************/
uint32_t user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32_t rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}
#endif

#ifndef CONFIG_TESTTASK_STACKSIZE
#define CONFIG_TESTTASK_STACKSIZE 2048
#endif

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
#if ESTD_IDF_VER >= ESTD_IDF_VER_2_0_0_644
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

#else
void user_init(void)
{
    printf("SDK version:%s\n", system_get_sdk_version());
#endif

#ifdef CONFIG_WIFI_SSID
#if ESTD_IDF_VER >= ESTD_IDF_VER_2_0_0_740
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler_cb, NULL) );
#else
    wifi_set_event_handler_cb(wifi_event_handler_cb);
#endif
#endif

#ifdef CONFIG_WIFI_SSID
    xTaskCreate(wifi_config, "wfcf", 
#if ESTD_IDF_VER >= ESTD_IDF_VER_2_0_0_740
        2048,
#else
        512,
#endif
        NULL, 4, NULL);
#endif
    xTaskCreate(test_task, "test_task", CONFIG_TESTTASK_STACKSIZE, NULL, 4, NULL);
}
