#include <estd/chrono.h>
#include <estd/ostream.h>

#include <pico/stdio_usb.h>
#include <pico/time.h>

static estd::pico_ostream cout(stdio_usb);

using namespace estd::chrono_literals;

typedef estd::chrono::experimental::pico_clock steady_clock;

int main()
{
    stdio_init_all();

    unsigned counter = 0;

    auto now = steady_clock::now();

    for(;;)
    {
        cout << "Hello Chrono: " << ++counter << estd::endl;
        estd::this_core::sleep_until(now + 1s * counter);
    }
}