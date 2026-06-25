#include <catch2/catch_all.hpp>

#include <chrono>

#include <estd/functional.h>
#include <estd/string.h>
#include <estd/string_view.h>
#include <estd/unordered_map.h>

#include "test/retry.h"

#pragma GCC diagnostic ignored "-Wunused-variable"

using namespace estd;

struct map_traits1 : internal::unordered_map_traits<int, layer1::string<16>>
{
    static constexpr unsigned bucket_depth = 1;
};

TEST_CASE("unordered_map", "[unordered][map][unordered_map]")
{
    using map_type = estd::layer1::unordered_map<int, layer1::string<32>, 16>;
    using iter = typename map_type::iterator;
    using const_iter = typename map_type::const_iterator;
    using pair = estd::pair<iter, bool>;

    SECTION("primary case")
    {
        using type = map_type;
        //using value_type = typename type::value_type;
        //using const_iter = typename type::const_iterator;
        using iterl = typename type::local_iterator;
        using const_iterl = typename type::const_local_iterator ;
        using pair = estd::pair<iter, bool>;

        type map;

        // FIX: Linear probing may span across two (or more) different
        // key spaces before it finds something, resulting in incorrect and undiscoverable
        // placements.  We need to check against bucket a bit more during insert/emplace.

        // NOTE: Key '0' won't work due to collision between hash(0) and Null, but that can be overcome
        // by tuning nullable_traits

        pair r2 = map.insert({1, "hi2u"});
        REQUIRE(r2.second);
        r2 = map.insert({1, "hi again"});
        REQUIRE(r2.second == false);
        REQUIRE(r2.first->second == "hi2u");
        int bucket1 = map.bucket(1);
        unsigned counter = 0;

        // NOTE: Undefined behavior, but valid in this tightly controlled unit test
        REQUIRE(bucket1 == type::bucket_depth);     // key 1 x bucket_depth

        for(const_iterl i = map.cbegin(bucket1); i != map.end(bucket1); ++i, ++counter)
        {
            REQUIRE(i->second == "hi2u");
        }

        REQUIRE(counter == 1);

        REQUIRE(map.bucket_size(bucket1) == 1);
        REQUIRE(map.contains(0) == false);
        REQUIRE(map.contains(1));
        REQUIRE(map.contains(2) == false);

        map.insert({2, "hello1"});
        map.emplace(piecewise_construct_t{},
            make_tuple(3),
            make_tuple("hello2"));

        SECTION("bucket ops")
        {
            // We already do a lot of bucket ops.  This is to push it harder

        }
        SECTION("counts / contains")
        {
            REQUIRE(map.contains(2));
            REQUIRE(map.contains(3));

            REQUIRE(map.count(0) == 0);
            REQUIRE(map.count(1) == 1);
            REQUIRE(map.count(2) == 1);
            REQUIRE(map.count(3) == 1);

            REQUIRE(map.size() == 3);
        }
        SECTION("existing/duplicate, emplace and erase_and_gc")
        {
            // Duplicates not permitted on this flavor of emplace
            REQUIRE(map.emplace(2, "hello1.1").second == false);
            REQUIRE(map.emplace(4, "hello3").second);

            REQUIRE(map[2] == "hello1");

            // Overriding and permitting duplicate for this guy
            // NOTE: Would fail if bucket_depth wasn't > 1, since 1 and 3 buckets are adjacent
            REQUIRE(map.insert({2, "hello1.1"}, true).second);

            REQUIRE(map.count(2) == 2);
            REQUIRE(map[2] == "hello1");
            iterl it_bucket_2 = map.begin(map.bucket(2));
            REQUIRE(it_bucket_2->second == "hello1");
            REQUIRE((it_bucket_2 + 1)->second == "hello1.1");
            map.erase_and_gc(it_bucket_2);
            REQUIRE(map.count(2) == 1);
            REQUIRE(map[2] == "hello1.1");
            map.erase_and_gc(it_bucket_2);
            REQUIRE(map.count(2) == 0);
        }
        SECTION("find")
        {
            REQUIRE(map.find(3)->second == "hello2");
            map.erase_and_gc(map.find(3));
            REQUIRE(map.find(3) == map.cend());
        }
        SECTION("operator[] assignment")
        {
            map[5] = "hello4";
            map.contains(5);
            REQUIRE(map[5] == "hello4");
        }
        SECTION("try_emplace")
        {
            REQUIRE(map.try_emplace(6, "hello5").second);
        }
        SECTION("erase and gc (distinct steps)")
        {
            SECTION("erase_ll, inspect (with find_ll), then gc")
            {
                // FIX: idx 0 does not work here, but should.  Do we need to do an auto-gc?
                // also I seem to recall idx 0 can be a special case sometimes - is that going on?
                constexpr int idx = 10;
                r2 = map.insert({idx, "hello1.1"}, true);
                REQUIRE(r2.second);

                auto found = map.find_ll(idx);

                REQUIRE(found.second != map.npos());
                REQUIRE(found.first->second.mapped() == "hello1.1");

                // FIX: Although it's undefined behavior to read an object post-destruction,
                // I don't think that's what's causing https://github.com/malachi-iot/estdlib/issues/197.
                // Still, we ought to roll in destruction at gc phase not erase phase if we can
                map.erase_ll(found);

                REQUIRE(found.second != map.npos());
                REQUIRE(found.first->second.mapped() == "hello1.1");

                map.gc_sparse_ll(found.first);
            }
            SECTION("duplicate")
            {
                // DEBT: Would try emplace but that one doesn't permit dups
                r2 = map.insert({2, "hello1.1"}, true);
                REQUIRE(r2.second);
                iter p1 = map.find(2);
                REQUIRE(p1->second == "hello1");
                map.erase(p1);
                // DEBT: insert itself needs to pass back iter
                p1 = map.gc_active(r2.first);
                REQUIRE(p1->second == "hello1.1");
                map.erase(p1);
                //p1 = map.gc_active_ll(p1);
                REQUIRE(map.count(2) == 0);
            }
        }
        SECTION("insert_or_assign")
        {
            map.insert_or_assign(10, "hi#10");
            REQUIRE(map[10] == "hi#10");
            map.insert_or_assign(10, "hi#10.1");
            REQUIRE(map[10] == "hi#10.1");
        }
        SECTION("clear")
        {
            // https://en.cppreference.com/w/cpp/container/unordered_map/clear
            auto it1 = map.begin();

            REQUIRE(it1->first == 1);
            ++it1;
            REQUIRE(it1->first == 2);
            REQUIRE(it1 != map.end());
            ++it1;
            REQUIRE(it1->first == 3);
            ++it1;
            REQUIRE(it1 == map.end());

            map.clear();

            it1 = map.begin();
            REQUIRE(it1 == map.end());
        }
        SECTION("equality")
        {
            map_type map1, map2;

            map1[1] = "hi1.1";
            map2.try_emplace(1, "hi2.1");
            map1.insert_or_assign(2, "hi1.2");
            map2.insert({ 2, "hi2.2" });

            REQUIRE(map1 == map2);
        }
    }
    SECTION("unordered_map: aggressive gc")
    {
        constexpr unsigned depth = map_type::bucket_depth;
        using cp = map_type::const_control_pointer;

        map_type map;
        pair r = map.insert({1, "hello1"});
        REQUIRE(r.second);
        r = map.insert({2, "hello2"});
        REQUIRE(r.second);
        r = map.insert({3, "hello3"});
        REQUIRE(r.second);
        r = map.insert({4, "hello4"});
        REQUIRE(r.second);
        r = map.insert({5, "hello5"});
        REQUIRE(r.second);
        r = map.insert({6, "hello6"});
        REQUIRE(r.second);
        REQUIRE(map.size() == 6);

        unsigned count = map.bucket_size(0);
        REQUIRE(count == 1);
        count = map.bucket_size(map_type::bucket_depth);
        REQUIRE(count == 2);
        count = map.bucket_size(map_type::bucket_depth * 2);
        REQUIRE(count == 2);
        count = map.bucket_size(map_type::bucket_depth * 3);
        REQUIRE(count == 1);

        // Erase key#2 which is bucket index 2 (x4 = 8)
        unsigned r1 = map.erase(2);
        REQUIRE(r1 == 1);
        REQUIRE(map.size() == 5);

        count = map.bucket_size(0);
        REQUIRE(count == 1);
        count = map.bucket_size(map_type::bucket_depth * 1);
        REQUIRE(count == 2);
        count = map.bucket_size(map_type::bucket_depth * 2);
        REQUIRE(count == 1);
        count = map.bucket_size(map_type::bucket_depth * 3);
        REQUIRE(count == 1);

        r = map.insert({2, "hello2.1"});
        REQUIRE(r.second);
        REQUIRE(map.size() == 6);
        count = map.bucket_size(map_type::bucket_depth * 2);
        REQUIRE(count == 2);

        REQUIRE(map[2] == "hello2.1");
        r1 = map.erase(2);
        REQUIRE(r1 == 1);
        REQUIRE(map.size() == 5);
        r1 = map.erase(6);
        REQUIRE(r1 == 1);
        REQUIRE(map.size() == 4);

        SECTION("not-null check")
        {
            // we expect prune to operate after last erase and really null out
            // this bucket
            map_type::const_local_iterator it = map.cbegin(depth * 1);
            cp it1 = it.it_;

            REQUIRE(!map.is_empty(*it1));
            ++it1;
            REQUIRE(!map.is_empty(*it1));
        }
        SECTION("null check")
        {
            // we expect prune to operate after last erase and really null out
            // this bucket
            map_type::const_local_iterator it = map.cbegin(depth * 2);
            cp it1 = it.it_;
            REQUIRE(map.is_null_not_sparse(*it1));
            ++it1;
            REQUIRE(map.is_null_not_sparse(*it1));
        }
        SECTION("misc")
        {
            // DEBT: put elsewhere
            static_assert(internal::has_destructor<int>::value == false, "");
        }
    }
    SECTION("unordered_map: layer2")
    {
        // layer2::const_string as convenient as it is, doesn't like assigning new pointer =
        // and I think I prefer that restriction.  So, using string view instead
        // DEBT: estd::string_view explodes, figure out why
        using type = layer2::unordered_map<int, estd::string_view, 16>;
        using control_type = typename type::control_type;
        control_type backing[16];

        type map1(backing);

        map1[1] = "hi2u";
        REQUIRE(map1[1] == "hi2u");
        REQUIRE(map1.size() == 1);
    }
    SECTION("unordered_map: layer3")
    {
        using type = layer3::unordered_map<int, estd::string_view>;
        using control_type = typename type::control_type;
        control_type backing[16];

        type map1(backing);

        map1[1] = "hi2u";
        REQUIRE(map1[1] == "hi2u");
        REQUIRE(map1.size() == 1);
    }
    SECTION("unordered_map: edge cases")
    {
        SECTION("more complicated item")
        {
            using namespace std::chrono;
            using clock = steady_clock;
            using time_point = typename clock::time_point;
            using item_type = test::retry_item_base<time_point>;
            // https://github.com/malachi-iot/estdlib/issues/110 - reproduced
            estd::layer1::unordered_map<int, item_type, 10> map;

            map.try_emplace(1, time_point(seconds(1)));
        }
        SECTION("estd::optional key")
        {
            using key_type = estd::layer1::optional<uint16_t, 0xFFFF>;

            using type = estd::layer1::unordered_map<key_type, layer1::string<16>, 8>;

            type map;

            map[0] = "Hello";

            REQUIRE(map.size() == 1);
        }
        SECTION("null key (invalid)")
        {
            map_type map;

            pair r = map.try_emplace(0, "That's a big no can do");

            REQUIRE(r.second == false);

            const_iter ci1 = map.find(0);

            REQUIRE(ci1 == map.cend());
        }
    }
    SECTION("full container")
    {
        //using type = layer1::unordered_map<int, layer1::string<16>, 4>;
        using traits = map_traits1;
        using type = layer2::detail::unordered_map<4, traits>;
        using pair = estd::pair<typename type::iterator, bool>;
        map_traits1::control_type backing[4] {};
        pair its[4] {};
        using iterator = typename type::iterator;
        using local_iterator = typename type::const_local_iterator;
        using end_local_iterator = typename type::end_local_iterator;

        auto addr_of = [](estd::pair<iterator, bool> v)
        {
            void* addr = v.first.operator->();
            return addr;
        };

        static_assert(type::bucket_depth == 1);

        type map(backing);

        SECTION("full up: distinct buckets")
        {
            its[0] = map.try_emplace(1, "Hello1");
            its[1] = map.try_emplace(2, "Hello2");
            its[2] = map.try_emplace(3, "Hello3");
            its[3] = map.try_emplace(4, "Hello4");

            REQUIRE(its[0].second);
            REQUIRE(its[1].second);
            REQUIRE(its[2].second);
            REQUIRE(its[3].second);

            pair it5 = map.try_emplace(5, "Hello5");

            REQUIRE(!it5.second);

            REQUIRE(map.bucket(1) == 1);
            REQUIRE(map.bucket(2) == 2);
            REQUIRE(map.bucket(3) == 3);
            REQUIRE(map.bucket(4) == 0);

            int key = 1;
            unsigned bucket1 = map.bucket(key);

            // FIX: 'count' reported 1, even when all 4 items really were in one bucket
            unsigned bucket_count = map.count(1);

            REQUIRE(bucket_count == 1);

            local_iterator lit = map.cbegin(bucket1);

            REQUIRE(lit->second == "Hello1");

            ++lit;

            end_local_iterator cend = map.cend(bucket1);

            bool ended = lit == cend;

            REQUIRE(ended);
        }
        SECTION("full up: shared bucket")
        {
            its[0] = map.try_emplace(1, "Hello1");
            its[1] = map.try_emplace(2, "Hello2");
            its[2] = map.try_emplace(3, "Hello3");
            its[3] = map.try_emplace(5, "Hello5");

            REQUIRE(its[0].second);
            REQUIRE(its[1].second);
            REQUIRE(its[2].second);
            REQUIRE(its[3].second);

            REQUIRE(addr_of(its[0]) == backing + 1);    // Key 1 at position 1
            REQUIRE(addr_of(its[3]) == backing);        // Key 5 at position 0 due to wraparound

            REQUIRE(map.bucket(1) == 1);
            REQUIRE(map.bucket(5) == 1);

            local_iterator lit = map.cbegin(1);

            REQUIRE(lit->second == "Hello1");

            ++lit;

            REQUIRE(lit->second == "Hello5");

            ++lit;

            end_local_iterator cend = map.cend(1);

            bool ended = lit == cend;

            REQUIRE(ended);

            // Catch2 gets mad about this
            //REQUIRE(lit == cend);
        }
        SECTION("full up: wraparound")
        {

        }
        // TODO: Do test which fills up a bucket, causes it to wrap around, then remove items
        // (not yet gc) and do further inserts to see where they land.  Then, GC and do it again.
        // TODO: As part of this, verify insert_precheck is skipping non-fitting sparse items
        // (you can insert back into a sparse if the bucket matches).
        // TODO: Outline insert_precheck use cases for when we encounter sparse items:
        //       1. Are there conditions where we accept inserting into a non-fitting sparse?
        //       1.a. Like what if we move past all the active items and find outself in another bucket?
        // TODO: Verify insert_precheck behaves rationally as above TODOs inquire about during a wrap around
    }
}