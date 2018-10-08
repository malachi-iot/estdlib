#include <estd/string.h>
#include <estd/thread.h>

#include <estd/ostream.h>
#include <estd/sstream.h>

using namespace estd;

extern "C" void test_task(void* pv)
{
    // NOTE: interesting that we can't do an int8_t but can
    // do a char.  Indicates that typedef'ing isn't 100% aliasing,
    // compiler does seem to differenciate - even though I've
    // seen indicators to the contrary elsewhere
    int16_t counter = 0;

    for(;;)
    {
        this_thread::sleep_for(chrono::milliseconds(1000));
        printf("Got here: %d -", ++counter);

        experimental::ostringstream<32> out;

        out << "hi2u: ";
        // TODO: ESP32 doesn't support this quite yet, but we need it
        out << counter;
        out << endl;

        const char* str = out.rdbuf()->str().data();

        printf(str);
    }
}
