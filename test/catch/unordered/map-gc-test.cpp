#include <catch2/catch_all.hpp>

#include <estd/unordered_map.h>

using namespace estd;

TEST_CASE("unordered_map gc", "[unordered][map][unordered_map][gc]")
{
    using map_type = estd::layer1::unordered_map<uint8_t, double, 16>;

    map_type map;

    // NOTE: Doing inserts not emplace to gain access to permit_duplicates
    SECTION("insert, remove")
    {
        constexpr int bucket = map.bucket(1);

        // DEBT: '0' index still counts as null, don't forget
        pair<map_type::iterator, bool> it = map.insert({ 1, 1.1 }, true);
        map.insert({1, 1.2}, true);
        map.insert({1, 1.3}, true);
        REQUIRE(map.bucket_size(bucket) == 3);
        map.erase(it.first);
    }
}