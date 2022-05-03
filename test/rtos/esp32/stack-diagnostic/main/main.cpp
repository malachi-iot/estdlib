#include "esp_log.h"
#include "nvs_flash.h"

#include <estd/istream.h>
#include <estd/string.h>

// FIX: Explicit include is too hard to get wrong
#include <estd/internal/istream_runtimearray.hpp>

// FIX: 144 bytes used in this method
void test1()
{
    static char buf[] = "hi2u hru";
    estd::layer1::string<16> s;

    estd::span<char> span(buf);
    estd::experimental::ispanstream is(span);

    // FIX: 544 bytes used in this operator overload
    is >> s;
}

extern "C" void app_main(void)
{
    static const char *TAG = "app_main";

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Startup");

    test1();
}