#include <catch2/catch_all.hpp>

#include <estd/unordered_map.h>

using namespace estd;

namespace test {

}

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
    using control_type = map_type::control_type;
    using modes = estd::internal::unordered_map_control_enum::modes;

    map_type map;
    pair<map_type::iterator, bool> it;

    constexpr int bucket = map.bucket(1);

    static_assert(bucket == 1, "1:1 mapping with key to bucket# expected for this test");
    static_assert(map.bucket(2) == 2, "1:1 mapping with key to bucket# expected for this test");

    SECTION("low level")
    {
        using modes = map_type::modes;
        map_type::container_type& c = map.container();
        using meta_type = map_type::meta;
        //using control = typename map_type::control_type;

        auto active = [](double v)
        {
            meta_type test;
            test.mapped() = v;
            return test;
        };

        auto tombstone = []()
        {
            return control_type{ 0, meta_type::create_tombstone() };
        };

        auto eol = [](unsigned n)
        {
            return control_type{ 0, meta_type::create_eol(n) };
        };

        SECTION("boomerang")
        {
            map_type::eol_helper eh;

            // Be mindful we have to start at '1' to match specified key (who is 1:1 with bucket#)
            c[1] = { 1, active(1) };

            REQUIRE(map.size() == 1);
            REQUIRE(map.bucket_size(1) == 1);

            SECTION("scenario 1")
            {
                c[2] = { 1, active(2) };
                c[3] = eol(1);

                REQUIRE(map.size() == 2);
                REQUIRE(map.bucket_size(1) == 2);

                eh.null = &c[4];

                // Turn trailing EOL into a null
                map.null_boomerang(eh, 1);

                REQUIRE(c[3].second.mode() == modes::NULLED);
            }
            SECTION("scenario 2")
            {
                c[2] = { 1, active(2) };
                c[3] = eol(1);
                c[4] = { 4, active(3) };
                c[5] = tombstone();
                c[6] = { 6, active(4) };

                REQUIRE(map.bucket_size(1) == 2);
                REQUIRE(map.bucket_size(4) == 1);
                REQUIRE(map.bucket_size(6) == 1);

                eh.null = &c[7];

                // Turn trailing tombstones into null
                map.null_boomerang(eh, 1);

                REQUIRE(c[3].second.mode() == modes::NULLED);
                REQUIRE(c[5].second.mode() == modes::NULLED);
            }
            SECTION("scenario 3")
            {
                c[2] = { 1, active(2) };
                c[3] = eol(1);
                c[4] = { 4, active(3) };
                c[5] = eol(4);
                c[6] = { 6, active(4) };

                eh.null = &c[7];

                // Turn trailing tombstones into null
                map.null_boomerang(eh, 1);

                REQUIRE(c[3].second.mode() == modes::NULLED);
                REQUIRE(c[5].second.mode() == modes::NULLED);
            }
            SECTION("scenario 4")
            {
                c[2] = { 2, active(2) };
                c[3] = eol(2);
                c[4] = { 1, active(3) };
                c[5] = eol(1);
                c[6] = { 6, active(4) };

                eh.null = &c[7];

                // c[3] EOL must stay EOL for linear probing to stay happy
                map.null_boomerang(eh, 1);

                REQUIRE(c[3].second.mode() == modes::EOL);
                REQUIRE(c[5].second.mode() == modes::NULLED);
            }
            SECTION("scenario 6")
            {
                c[2] = tombstone();
                c[3] = { 1, active(2) };
                c[4] = tombstone();
                c[5] = { 2, active(3) };
                c[6] = eol(1);
                c[7] = { 2, active(4) };

                eh.null = &c[8];

                // EOL can move forward in this one
                map.null_boomerang(eh, 1);

                REQUIRE(c[2].second.mode() == modes::TOMBSTONE);
                REQUIRE(c[4].second.mode() == modes::EOL);
                REQUIRE(c[4].second.bucket() == 1);
                REQUIRE(c[6].second.mode() == modes::EOL);
            }
            SECTION("scenario 7")
            {
                c[2] = tombstone();
                c[3] = { 2, active(2) };
                c[4] = eol(2);
                c[5] = { 1, active(3) };
                c[6] = tombstone();
                c[7] = { 2, active(4) };

                eh.null = &c[8];

                map.null_boomerang(eh, 1);
            }
        }
    }
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