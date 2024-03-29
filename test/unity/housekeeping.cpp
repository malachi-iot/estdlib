#ifdef ESP_PLATFORM
#include <esp_log.h>

static const char* TAG = "unity::housekeeping";
#endif

void __attribute__((weak)) setUp_chrono();

//#error GOT HERE

static void setUp_static()
{
    // DEBT: Lazy initialization, would be much better to init this in the
    // actual main of the program - just that varies a lot depending on whether
    // we're in esp-idf or not
    static bool initialized = false;

    if(initialized) return;

    initialized = true;

    // DEBT: For some unknown reason, AVR weak linkage flips out.  For
    // the time being, disabling the whole thing
#ifndef __AVR__
    setUp_chrono();
#endif
}

//void tearDown_static() {}

extern "C" void setUp()
{
    setUp_static();
}


#ifdef ESP_PLATFORM
extern "C" void tearDown()
{
    static bool deinitialized = false;

    if(deinitialized)    return;

    ESP_LOGV(TAG, "tearDown");
    deinitialized = true;
}
#endif