// Simplistic wrapper to bring things online for non-esp targets
// DEBT: Would be better to use a wholistic logging system which included ESP target
#pragma once

#include <stdio.h>

#define ESP_LOGW(tag, fmt...)     { printf("W %s: ", tag); printf(fmt); puts(""); }
#define ESP_LOGI(tag, fmt...)     { printf("I %s: ", tag); printf(fmt); puts(""); }
#define ESP_LOGD(tag, fmt...)     { printf("D %s: ", tag); printf(fmt); puts(""); }
#define ESP_LOGV(tag, fmt...)     { printf("V %s: ", tag); printf(fmt); puts(""); }

#define ESP_LOG_BUFFER_HEX(tag, buffer, len)