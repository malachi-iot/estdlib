#include "unit-test.h"

#include <estd/array.h>
#include <estd/unordered_map.h>
#include <estd/unordered_set.h>

using namespace estd;

static void test_unordered_map()
{
    using key_type = estd::array<uint8_t, 6>;
    using mapped_type = estd::array<uint8_t, 10>;

    using map_type = layer1::unordered_map<
        key_type, mapped_type, 10, internal::container_hash<>>;
    using iterator = typename map_type::iterator;
    using rtype = pair<iterator, bool>;

    // DEBT: Consumes too much memory to be friendly to AVR.  Consider a unity-wide
    // objstack, since unity tests are sequential
#ifndef __AVR__
    static map_type map;

    constexpr key_type key1{ 0, 1, 2, 3, 4, 5 };
    constexpr key_type key2{ 0, 1, 2, 3, 4, 6 };

    rtype r1 = map.try_emplace(key1);
    TEST_ASSERT_TRUE(r1.second);
    r1.first->second[0] = 0x77;
    TEST_ASSERT_EQUAL_UINT8(0x77, map[key1][0]);
    //r1 = map.emplace(key2, {});
#endif
}


static void test_unordered_set()
{

}

#ifdef ESP_IDF_TESTING
TEST_CASE("unordered map/set tests", "[unordered]")
#else
void test_unordered()
#endif
{
    RUN_TEST(test_unordered_map);
    RUN_TEST(test_unordered_set);
}
