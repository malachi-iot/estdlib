#include <estd/string.h>
#include <estd/thread.h>

using namespace estd;

#pragma message IDF_VER

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#pragma message "Version: " STRINGIZE(ESTD_IDF_VER_MAJOR)

#if ESTD_IDF_VER_MAJOR >= 2
#endif

extern "C" void test_task(void* pv)
{
    for(;;)
    {
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}