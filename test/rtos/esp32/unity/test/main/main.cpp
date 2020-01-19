#include "unity.h"

extern "C" void app_main(void)
{
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();

    /* This function will not return, and will be busy waiting for UART input.
     * Make sure that task watchdog is disabled if you use this function.
     */
    unity_run_menu();    
}