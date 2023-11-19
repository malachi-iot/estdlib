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

    setUp_chrono();
}

//void tearDown_static() {}

extern "C" void setUp()
{
    setUp_static();
}