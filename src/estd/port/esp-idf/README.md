# esp-idf support

esp-idf is largely FreeRTOS supported for estd purposes

## fake

Since so much testing happens on esp-idf, it's not unusual to use ESP_LOG facilities.
As a convenience, there is an `esp_log.h` wrapper.  It's not recommended for production
use, only to bring up test code.