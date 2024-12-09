#include <esp_log.h>

#include "unity.h"

static const char* TAG = "unity::main";

extern "C" void app_main(void)
{
    ESP_LOGV(TAG, "app_main: entry");

    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();

    /* This function will not return, and will be busy waiting for UART input.
     * Make sure that task watchdog is disabled if you use this function.
     */
    unity_run_menu();    
}