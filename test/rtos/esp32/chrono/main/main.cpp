#include <estd/string.h>
#include <estd/thread.h>

using namespace estd;

//#pragma message IDF_VER

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

//#pragma message "Version: " STRINGIZE(ESTD_IDF_VER_MAJOR) " suffix: " STRINGIZE(ESTD_IDF_VER_SUFFIX)
#pragma message "Version: " STRINGIZE(ESTD_IDF_VER)


extern "C" void test_task(void* pv)
{
    int counter = 0;
    // TODO: Do a freertos_clock (aka stead_clock) test
    for(;;)
    {
        this_thread::sleep_for(chrono::milliseconds(1000));
        printf("Got here: %d\n", counter++);
    }
}