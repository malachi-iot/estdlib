#include <estd/string.h>
#include <estd/thread.h>

#include <estd/ostream.h>
#include <estd/sstream.h>

using namespace estd;

extern "C" void test_task(void* pv)
{
    int counter = 0;
    for(;;)
    {
        this_thread::sleep_for(chrono::milliseconds(1000));
        printf("Got here: %d -", counter++);

        experimental::ostringstream<32> out;

        out << "hi2u";
        out << endl;

        const char* str = out.rdbuf()->str().data();

        printf(str);
    }
}
