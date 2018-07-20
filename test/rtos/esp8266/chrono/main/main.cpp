#include <estd/string.h>
#include <estd/thread.h>

using namespace estd;

extern "C" void test_task(void* pv)
{
    for(;;)
    {
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}