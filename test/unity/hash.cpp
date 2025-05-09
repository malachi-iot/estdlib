#include "unit-test.h"
#include "test-data.h"

#include <estd/internal/functional/hash.h>

using namespace estd;

inline namespace R {
#if ESP_PLATFORM
ESP_BINARY(hello, "hello_txt")
ESP_BINARY(blob1, "blob1")
#endif
}

static void test_fnv1a_32()
{
    using fnv1a = internal::fnv_hash<uint32_t, internal::FNV_1A>;
    uint32_t h;

#if ESP_PLATFORM
    h = fnv1a::hash(hello.begin(), hello.end());
    TEST_ASSERT_EQUAL(0x325D0001, h);
    h = fnv1a::hash(blob1.begin(), blob1.end());
    TEST_ASSERT_EQUAL(0x774dd18d, h);
#else
    const char* hello = "Hi!";
    h = fnv1a::hash(hello, hello + 3);
    TEST_ASSERT_EQUAL_HEX32(0x325D0001, h);
#endif
}

#ifdef ESP_IDF_TESTING
TEST_CASE("hash", "[hash]")
#else
void test_hash()
#endif
{
    RUN_TEST(test_fnv1a_32);
}

