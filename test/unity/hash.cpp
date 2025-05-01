#include "unit-test.h"
#include "test-data.h"

#include <estd/internal/functional/hash.h>

using namespace estd;

inline namespace R {
#if ESP_PLATFORM
ESP_BINARY(hello, "hello_txt")
#endif
}

static void test_fnv1a_32()
{
    using fnv1a = internal::fnv_hash<uint32_t, internal::FNV_1A>;
    uint32_t h;

    h = fnv1a::hash(hello.begin(), hello.end());
    TEST_ASSERT_EQUAL(0x325D0001, h);
}

#ifdef ESP_IDF_TESTING
TEST_CASE("expected", "[expected]")
#else
void test_expected()
#endif
{
    RUN_TEST(test_fnv1a_32);
}

