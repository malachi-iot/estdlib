#include <estd/string.h>
#include <estd/thread.h>
#include <estd/sstream.h>

using namespace estd;

//#pragma message IDF_VER

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

//#pragma message "Version: " STRINGIZE(ESTD_IDF_VER_MAJOR) " suffix: " STRINGIZE(ESTD_IDF_VER_SUFFIX)
//#pragma message "Version: " STRINGIZE(ESTD_IDF_VER)

#if ESTD_IDF_VER_MAJOR >= 2
#endif

extern "C" void test_task(void* pv)
{
    for(;;)
    {
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}