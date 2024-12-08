// Simplistic wrapper to bring things online for non-esp targets
// DEBT: Would be better to use a wholistic logging system which included ESP target
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

//#define LOG_LOCAL_LEVEL

//#define ESP_LOG_LEVEL

enum esp_log_level_t
{
    ESP_LOG_NONE,
    ESP_LOG_ERROR,
    ESP_LOG_WARN,
    ESP_LOG_INFO,
    ESP_LOG_DEBUG,
    ESP_LOG_VERBOSE
};

#define ESP_LOGE(tag, fmt...)     { printf("E %s: ", tag); printf(fmt); puts(""); }
#define ESP_LOGW(tag, fmt...)     { printf("W %s: ", tag); printf(fmt); puts(""); }
#define ESP_LOGI(tag, fmt...)     { printf("I %s: ", tag); printf(fmt); puts(""); }
#define ESP_LOGD(tag, fmt...)     { printf("D %s: ", tag); printf(fmt); puts(""); }
#define ESP_LOGV(tag, fmt...)     { printf("V %s: ", tag); printf(fmt); puts(""); }

#define ESP_LOG_BUFFER_HEX(tag, buffer, len)

void esp_log_write(esp_log_level_t level, const char *tag, const char *format, ...);

#ifdef __cplusplus
}
#endif
