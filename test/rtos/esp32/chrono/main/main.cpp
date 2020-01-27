#include <estd/string.h>
#include <estd/thread.h>
#include <estd/chrono.h>

using namespace estd;

extern "C" void test_task(void* pv)
{
    // TODO: Do a freertos_clock (aka steady_clock) test
    for(;;)
    {
        this_thread::sleep_for(chrono::seconds(1));

        auto now = estd::chrono::esp_clock::now();
        auto now_ms = estd::chrono::milliseconds(now.time_since_epoch());

        printf("Time passed: %d\n", now_ms.count());
    }
}