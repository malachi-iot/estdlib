/**
 * @file
 */

#include "unit-test.h"

#include <estd/chrono.h>

#undef min
#undef max

static void test_chrono_convert()
{
    estd::chrono::minutes m(2);

    estd::chrono::seconds s = estd::chrono::duration_cast<estd::chrono::seconds>(m);

    TEST_ASSERT_EQUAL(120, s.count());
}

static void test_chrono_subtract()
{
    estd::chrono::seconds s(10);
    estd::chrono::minutes m(2);

    estd::common_type<estd::chrono::seconds,estd::chrono::minutes>::type d;

    d = m - s;

    TEST_ASSERT_EQUAL_INT(110, d.count());

    estd::chrono::milliseconds ms(100);

    ms = d - ms;

    TEST_ASSERT_EQUAL_INT(110000 - 100,ms.count());
}

template <class TClock>
void test_clock()
{
    typedef TClock clock_type;
    typedef typename clock_type::time_point time_point;

    clock_type c;

    time_point min = time_point::min();
    time_point n = c.now();

    TEST_ASSERT(n > min);
}


// NOTE: Right now we aggressively alias our own _clock into steady_clock
// so this test at the moment overlaps with others, except for robust environments
// like esp-idf which have a proper std::chrono::steady_clock implementation
#ifdef FEATURE_STD_CHRONO
static void test_steady_clock()
{
    test_clock<std::chrono::steady_clock>();
    test_clock<estd::chrono::steady_clock>();
}
#endif

#ifdef ESTD_FREERTOS
static void test_freertos_clock()
{
    test_clock<estd::chrono::freertos_clock>();
}
#endif

#ifdef ESTD_SDK_IDF
static void test_esp_idf_clock()
{
    test_clock<estd::chrono::esp_clock>();
}
#endif


#ifdef ESP_IDF_TESTING
TEST_CASE("chrono tests", "[chrono]")
#else
void test_chrono()
#endif
{
    RUN_TEST(test_chrono_convert);
    RUN_TEST(test_chrono_subtract);
#ifdef FEATURE_STD_CHRONO
    RUN_TEST(test_steady_clock);
#endif
#ifdef ESTD_FREERTOS
    RUN_TEST(test_freertos_clock);
#endif
#ifdef ESTD_SDK_IDF
    RUN_TEST(test_esp_idf_clock);
#endif
}

