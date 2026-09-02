#include <catch2/catch_all.hpp>

#include <estd/unordered_map.h>

using namespace estd;

// 02AUG26 MB DEBT: Really at this point ESTD_UNORDERED_MAP_BUCKET_SIZE probably should be 1 by default
// Prior to #211, we needed a bigger bucket size.  Now, it's only interesting if we anticipate low randomness
// on key hash - and even then, might not be too big a bother - occasional colliding entries in linear
// probing is expected
struct traits : internal::unordered_map_traits<uint8_t, double>
{
    static constexpr unsigned bucket_depth = 1;
    static constexpr bool permit_duplicates = true;    // UNUSED
};

TEST_CASE("unordered_map gc", "[unordered][map][unordered_map][gc]")
{
    using map_type = estd::layer1::detail::unordered_map<16, traits>;
    using modes = estd::internal::unordered_map_control_enum::modes;

    map_type map;
    pair<map_type::iterator, bool> it;

    constexpr int bucket = map.bucket(1);

    static_assert(bucket == 1, "1:1 mapping with key to bucket# expected for this test");
    static_assert(map.bucket(2) == 2, "1:1 mapping with key to bucket# expected for this test");

    // NOTE: Doing inserts not emplace to gain access to permit_duplicates
    SECTION("insert, remove: all same bucket")
    {
        // DEBT: '0' index still counts as null, don't forget
        it = map.insert({ 1, 1.1 }, true);
        map.insert({1, 1.2}, true);
        map.insert({1, 1.3}, true);
        REQUIRE(map.bucket_size(bucket) == 3);
        map.erase(it.first);
        map_type::control_pointer control = map.cast_control(it.first.operator->());
        const map_type::meta& meta = control->second;
        REQUIRE(meta.mode() == modes::TOMBSTONE);

        // bucket_depth MUST be 1 for this to work
        REQUIRE((void*)(map.container().begin() + 1) == (void*)it.first.operator ->());
    }
    SECTION("insert, remove: adjacent bucket and mark EOL")
    {
        map.insert({ 1, 1.1 }, true);
        it = map.insert({1, 1.2}, true);
        map.insert({2, 2.1}, true);
        map.erase(it.first);
        REQUIRE(map.bucket_size(bucket) == 1);
        map_type::control_pointer control = map.cast_control(it.first.operator->());
        const map_type::meta& meta = control->second;
        // FIX: In fact, this ought to be NULLED since he can deduce that no other bucket 1's appear.
        // If we operate in an abbreviated mode, EOL is OK - but we haven't coded out an abbreviated
        // mode yet
        REQUIRE(meta.mode() == modes::EOL);
        REQUIRE(meta.bucket() == bucket);
    }
}