#include "esp_log.h"
#include "nvs_flash.h"

#include <estd/thread.h>

#include "tests.h"

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

    freertos::timer_test();

    for(;;)
    {
        estd::this_thread::sleep_for(estd::chrono::seconds(5));
    }
}