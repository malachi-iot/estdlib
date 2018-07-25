//#include <estd/string.h>
//#include <estd/thread.h>
#include <estd/internal/platform.h>

//using namespace estd;


#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

//#pragma message "Version: " STRINGIZE(ESTD_IDF_VER_MAJOR) " suffix: " STRINGIZE(ESTD_IDF_VER_SUFFIX)
#pragma message "Version: " STRINGIZE(ESTD_IDF_VER)

extern "C" void test_task(void* pv)
{
    for(;;)
    {
    }
}