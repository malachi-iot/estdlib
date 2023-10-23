#include <stdio.h>

#include <estd/string.h>

#include <esp_log.h>

extern "C" void app_main(void)
{
    static const char* TAG = "app_main";

    estd::layer1::string<32> s("hi2u");

    ESP_LOGI(TAG, "s=%s", s.data());
}
