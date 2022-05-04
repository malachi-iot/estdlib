#include "esp_log.h"
#include "nvs_flash.h"

//#define FEATURE_ESTD_CHARTRAITS 1

#include <estd/istream.h>
#include <estd/string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// FIX: Explicit include is too hard - easy to get wrong
#include <estd/internal/istream_runtimearray.hpp>

// FIX: 128 bytes used in this method
void test1()
{
    //typedef estd::char_traits<char> traits_type;

    static const char* TAG = "test1";

    static char buf[] = "hi2u hru";
    estd::layer1::string<16> s;

    estd::span<char> span(buf);
    estd::experimental::ispanstream is(span);

    is >> s;

    ESP_LOGI(TAG, "sizes buf=%u, s=%u, span=%u, is=%u",
        sizeof(buf), sizeof(s), sizeof(span), sizeof(is));
    ESP_LOGI(TAG, "s=%s", s.data());
}


void test2()
{
    static const char* TAG = "test2";

    estd::layer1::vector<uint8_t, 64> v;

    v.push_back(5);

    ESP_LOGI(TAG, "sizeof(v)=%u, v[0]=%u", sizeof(v), (unsigned)v[0]);
}

extern "C" void app_main(void)
{
    static const char* TAG = "app_main";

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Startup");

    for(;;)
    {
        test1();
        test2();

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}