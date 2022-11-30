#include <estd/ostream.h>

#include <pico/stdio_usb.h>
#include <pico/time.h>

static estd::basic_pico_ostream<char, &stdio_usb> cout;

int main()
{
    stdio_init_all();

    static unsigned counter = 0;

    for(;;)
    {
        cout << "Hello World: " << ++counter << estd::endl;
        sleep_ms(1000);
    }
}