#include "unit-test.h"

#include <estd/port/identify_platform.h>

#ifdef ESTD_OS_FREERTOS

#include <estd/port/freertos/wrapper/task.h>

#ifdef ESP_IDF_TESTING
TEST_CASE("freertos tests", "[freertos]")
{

}
#endif

#endif
