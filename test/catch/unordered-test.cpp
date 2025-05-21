#include <catch2/catch_all.hpp>

#include <chrono>

#include <estd/functional.h>
#include <estd/string.h>
#include <estd/unordered_set.h>
#include <estd/unordered_map.h>

#include "test/retry.h"

using namespace estd;

namespace estd {

// For these synthetic tests only, priority_queue ignores key during compare
template <class Key>
struct less<pair<Key, test::retry_item>*>
{
    using value_type = pair<Key, test::retry_item>*;
    using const_reference = const value_type&;

    bool operator()(const_reference lhs, const_reference rhs) const
    {
        // NOTE: Doing a 'greater' here for convenience
        return lhs->second.timestamp_ > rhs->second.timestamp_;
    }
};

}

struct buf_item
{
    char buf[0];
};

TEST_CASE("unordered")
{
    using map_type = estd::layer1::unordered_map<int, layer1::string<32>, 16>;
    using iter = typename map_type::iterator;
    using pair = estd::pair<iter, bool>;

    SECTION("unordered_map")
    {
        using type = map_type;
        //using value_type = typename type::value_type;
        using iter = typename type::iterator;
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
        SECTION("duplicate, emplace and erase_and_gc")
        {
            // Duplicates not permitted on this flavor of emplace
            REQUIRE(map.emplace(2, "hello1.1").second == false);
            REQUIRE(map.emplace(4, "hello3").second);

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
        }
        SECTION("try_emplace")
        {
            REQUIRE(map.try_emplace(6, "hello5").second);
        }
        SECTION("erase and gc (distinct steps)")
        {
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
        int r1 = map.erase(2);
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
    }
    SECTION("unordered_set")
    {
        using type = estd::layer1::unordered_set<int, 10>;
        using rtype = estd::pair<type::iterator, bool>;

        type value;

        REQUIRE(value.size() == 0);     // NOLINT
        REQUIRE(value.empty());
        REQUIRE(value.contains(5) == false);

        rtype r = value.insert(5);

        REQUIRE(r.second);
        REQUIRE(value.contains(5));
        REQUIRE(value.contains(4) == false);

        REQUIRE(value.size() == 1);
        REQUIRE(value.empty() == false);

        r = value.insert(6);

        REQUIRE(r.second);

        REQUIRE(value.size() == 2);

        value.erase(5);

        REQUIRE(value.contains(5) == false);
        REQUIRE(value.contains(6));

        REQUIRE(value.size() == 1);

        SECTION("compare")
        {
            type value2;

            value2.insert(6);

            bool r = value == value2;

            REQUIRE(r);
        }
    }
    SECTION("synthetic retry")
    {
        // synthetic (but representative, possibly reference) use case of transport retry logic
        test::retry_tracker<layer1::string<32>, test::retry_item> tracker;

        tracker.track("hello5", test::retry_item{ 5 });

        REQUIRE(tracker.tracked_.size() == 1);
        REQUIRE(tracker.queue_.size() == 1);

        tracker.track("hello5", test::retry_item{ 10 });

        REQUIRE(tracker.tracked_.size() == 1);
        REQUIRE(tracker.queue_.size() == 1);

        tracker.track("hello10", test::retry_item{ 10 });

        REQUIRE(tracker.tracked_.size() == 2);
        REQUIRE(tracker.queue_.size() == 2);

        unsigned processed;

        processed = tracker.poll_one(5);
        REQUIRE(processed == 1);
        processed = tracker.poll_one(9);
        REQUIRE(processed == 0);
        tracker.incoming("hello5");
        processed = tracker.poll(15);
        REQUIRE(processed == 2);

        REQUIRE(tracker.tracked_.size() == 1);
        REQUIRE(tracker.queue_.size() == 1);
    }
}
