#include "esp_log.h"
#include "nvs_flash.h"

#include <estd/thread.h>
#include <estd/mutex.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}