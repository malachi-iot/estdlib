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



/******************************************************************************
 * FunctionName : wifi_event_handler_cb
 * Description  : wifi event callback
 * Parameters   : system event
 * Returns      : none
 *******************************************************************************/
#if ESTD_IDF_VER <= ESTD_IDF_VER_2_0_0_444
void wifi_event_handler_cb(System_Event_t * event)
#else
void wifi_event_handler_cb(system_event_t * event)
#endif
{
    if (event == NULL) {
        return;
    }
    switch (event->event_id) {
#if ESTD_IDF_VER <= ESTD_IDF_VER_2_0_0_444
        case EVENT_STAMODE_GOT_IP:
#else
        case SYSTEM_EVENT_STA_GOT_IP:
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
            break;
        default:
            break;

    }
    return;
}


#ifdef CONFIG_WIFI_SSID
/******************************************************************************
 * FunctionName : wifi_config
 * Description  : wifi_config task
 * Parameters   : task param
 * Returns      : none
 *******************************************************************************/
void wifi_config(void *pvParameters)
{
#if ESTD_IDF_VER >= ESTD_IDF_VER_2_0_0_644
    tcpip_adapter_init();
#endif

    struct ip_info ip_config;
    struct station_config sta_config;
    memset(&sta_config, 0, sizeof(struct station_config));
    wifi_set_opmode(STATION_MODE);
    memcpy(sta_config.ssid, CONFIG_WIFI_SSID, strlen(CONFIG_WIFI_SSID));
    memcpy(sta_config.password, CONFIG_WIFI_PASSWORD, strlen(CONFIG_WIFI_PASSWORD));
    wifi_station_set_config(&sta_config);

    wifi_station_disconnect();
    wifi_station_connect();

    while (1) {
        vTaskDelay(10);
    }
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
    wifi_set_event_handler_cb(wifi_event_handler_cb);
#endif

#ifdef CONFIG_WIFI_SSID
    xTaskCreate(wifi_config, "wfcf", 512, NULL, 4, NULL);
#endif
    xTaskCreate(test_task, "test_task", 2048, NULL, 4, NULL);
}
