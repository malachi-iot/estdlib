#include <catch2/catch_all.hpp>

#include <estd/functional.h>
#include <estd/string.h>
#include <estd/internal/container/unordered_set.h>
#include <estd/internal/container/unordered_map.h>
#include <estd/optional.h>

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
        // NOTE: Doing a 'more' here for convenience
        return lhs->second.timestamp_ > rhs->second.timestamp_;
    }
};

}

TEST_CASE("unordered")
{
    SECTION("unordered_map")
    {
        using type = estd::internal::unordered_map<16, int, layer1::string<32>>;
        //using value_type = typename type::value_type;
        using iter = typename type::local_iterator;
        using const_iter = typename type::const_local_iterator ;
        using pair = estd::pair<typename type::iterator, bool>;
        using ptr = typename type::pointer;

        type map;

        // FIX: Linear probing may span across two (or more) different
        // key spaces before it finds something, resulting in incorrect and undiscoverable
        // placements.  We need to check against bucket a bit more during insert/emplace.

        // NOTE: Key '0' won't work due to collision between hash(0) and Null, but that can be overcome
        // by tuning nullable_traits

        pair r1 = map.insert({1, "hi2u"});
        REQUIRE(r1.second);
        r1 = map.insert({1, "hi again"});
        REQUIRE(r1.second == false);
        REQUIRE(r1.first->second == "hi2u");
        int bucket1 = map.bucket(1);
        unsigned counter = 0;

        // NOTE: Undefined behavior, but valid in this tightly controlled unit test
        REQUIRE(bucket1 == type::bucket_depth);     // key 1 x bucket_depth

        for(const_iter i = map.cbegin(bucket1); i != map.end(bucket1); ++i, ++counter)
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
            iter it_bucket_2 = map.begin(map.bucket(2));
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
            REQUIRE(map.find(3) == map.cend_old());
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
            // DEBT: Would try emplace but that one doesn't permit dups
            r1 = map.insert({2, "hello1.1"}, true);
            REQUIRE(r1.second);
            ptr p1 = map.find(2);
            REQUIRE(p1->second == "hello1");
            map.erase(p1);
            p1 = map.gc_active_ll(r1.first);
            REQUIRE(p1->second == "hello1.1");
            map.erase(p1);
            //p1 = map.gc_active_ll(p1);
            REQUIRE(map.count(2) == 0);
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
    SECTION("unordered_set")
    {
        using type = estd::internal::unordered_set<array<int, 10 >>;
        using rtype = pair<type::iterator, bool>;

        type value;

        REQUIRE(value.size() == 0);
        REQUIRE(value.empty());

        rtype r = value.insert(5);

        REQUIRE(r.second);

        REQUIRE(value.size() == 1);
        REQUIRE(value.empty() == false);

        r = value.insert(6);

        REQUIRE(r.second);

        REQUIRE(value.size() == 2);
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
