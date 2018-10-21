#include <estd/string.h>
#include <estd/thread.h>

#include <estd/array.h>
#include <estd/vector.h>
#include <estd/iterator.h>

#include <stdio.h> // for printf

using namespace estd;

/**
 * Trying to eliminate:
   /home/malachi/Projects/iot/estdlib/test/rtos/esp32/collection/
    components/estdlib/estd/internal/../array.h:315:26: 
        warning: type qualifiers ignored on function return type [-Wignored-qualifiers]
    const_iterator end() const { return base_type::data() + N; }
 */

extern "C" void test_task(void* pv)
{
    int counter = 0;
    for(;;)
    {
        this_thread::sleep_for(chrono::milliseconds(1000));
        printf("Got here: %d\n", counter++);
    }
}