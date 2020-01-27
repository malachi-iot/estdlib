//#include <estd/string.h>
//#include <estd/thread.h>
#include <estd/internal/platform.h>

//using namespace estd;

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

//#pragma message "Version: " STRINGIZE(ESTD_IDF_VER_MAJOR) " suffix: " STRINGIZE(ESTD_IDF_VER_SUFFIX)
#pragma message "Version: " STRINGIZE(ESTD_IDF_VER)

#ifndef ESTD_IDF_VER
#error "Failed IDF version building sanity check"
#endif



extern "C" void test_task(void* pv)
{
    for(;;)
    {
        // Mainly so WDT doesn't get upset
        vTaskDelay(1);
    }
}