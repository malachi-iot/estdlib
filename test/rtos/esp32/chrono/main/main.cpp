#include <estd/string.h>
#include <estd/thread.h>
#include <estd/chrono.h>

using namespace estd;

extern "C" void test_task(void* pv)
{
    for(;;)
    {
        this_thread::sleep_for(chrono::seconds(1));

        auto now = chrono::esp_clock::now();
        auto now_freertos = chrono::freertos_clock::now();
        auto now_ms = chrono::milliseconds(now.time_since_epoch());
        auto now_freertos_ms = chrono::milliseconds(now_freertos.time_since_epoch());

        printf("Time passed: (native)=%d / (rtos)=%d\n", 
            now_ms.count(),
            now_freertos_ms.count());
    }
}