#include <estd/string.h>
#include <estd/thread.h>
#include <estd/chrono.h>

using namespace estd;

//#pragma message IDF_VER

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

//#pragma message "Version: " STRINGIZE(ESTD_IDF_VER_MAJOR) " suffix: " STRINGIZE(ESTD_IDF_VER_SUFFIX)
#pragma message "Version: " STRINGIZE(ESTD_IDF_VER)


extern "C" void test_task(void* pv)
{
    // TODO: Do a freertos_clock (aka stead_clock) test
    for(;;)
    {
        this_thread::sleep_for(chrono::seconds(1));

        auto now = estd::chrono::esp_clock::now();
        auto now_ms = estd::chrono::milliseconds(now.time_since_epoch());

        printf("Time passed: %d\n", now_ms.count());
    }
}