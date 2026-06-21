#include <catch2/catch_all.hpp>

#include <chrono>

#include <estd/functional.h>
#include <estd/string.h>
#include <estd/string_view.h>
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

TEST_CASE("unordered", "[unordered]")
{
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
    SECTION("basics")
    {
        using map_type = estd::layer1::unordered_map<int, float, 10>;
        using const_iter = typename map_type::const_iterator;
        map_type map;

        SECTION("const things")
        {
            const_iter it = map.find(1);

            REQUIRE(it == map.cend());

            map[1] = 3;

            it = map.find(1);

            REQUIRE(it->second == 3);
        }
    }
}

TEST_CASE("unordered: synthetic retry", "[unordered][retry]")
{
    SECTION("synthetic retry")
    {
        // synthetic (but representative, possibly reference) use case of transport retry logic
        test::retry_tracker<layer1::string<32>, test::retry_item> tracker;

        test::retry_item* item1 = tracker.track("hello5", test::retry_item{ 5 });

        REQUIRE(item1);

        REQUIRE(tracker.tracked_.size() == 1);
        REQUIRE(tracker.queue_.size() == 1);

        // item#1 replacement - rejected due to unordered_map.emplace behavior
        REQUIRE(!tracker.track("hello5", test::retry_item{ 10 }));

        REQUIRE(tracker.tracked_.at("hello5").timestamp_ == 5);
        REQUIRE(tracker.tracked_.size() == 1);
        REQUIRE(tracker.queue_.size() == 1);
        REQUIRE(item1->ack_received_ == false);

        // emplace mode
        test::retry_item* item2 = tracker.track("hello10", 10);   // item#2

        REQUIRE(tracker.tracked_.size() == 2);
        REQUIRE(tracker.queue_.size() == 2);
        REQUIRE(item2);
        REQUIRE(item2->timestamp_ == 10);

        const test::retry_item* top = tracker.top();
        REQUIRE(top);
        REQUIRE(top->timestamp_ == 5);

        unsigned processed;

        processed = tracker.poll_one(5);        // item#1 evaluates, no ACK received yet
        REQUIRE(processed == 1);
        REQUIRE(tracker.queue_.size() == 2);    // no expiry + ACK received means queue size is unchanged
        processed = tracker.poll_one(9);        // item#1 evaluates, no ACK yet, reschedules itself for +10 from here (DEBT)
        REQUIRE(processed == 0);
        const test::retry_item* tracked_pointer = tracker.incoming("hello5");             // ACK received, immediately untrack
        REQUIRE(tracked_pointer);
        REQUIRE(tracked_pointer == item1);
        REQUIRE(item1->timestamp_ == 15);
        REQUIRE(item1->ack_received_);
        REQUIRE(tracker.tracked_.size() == 2);  // "hello5" still exists, since we don't want him GC'd away yet
        REQUIRE(tracker.queue_.size() == 2);    // queue size only changes at poll
        // Don't do this because item is not fully tracked anymore, though lives on in not-yet-GC land
        //REQUIRE(tracker.tracked_.at("hello5").ack_received_);

        //processed = tracker.poll(15);         // Disabling temporarily while we diagnose https://github.com/malachi-iot/estdlib/issues/197
        bool ack_received{true};
        top = tracker.top();
        REQUIRE(top);
        REQUIRE(top->timestamp_ == 10);
        processed = tracker.poll_one(15, &ack_received);
        REQUIRE(processed == 1);
        REQUIRE(!ack_received);

        top = tracker.top();
        REQUIRE(top);
        REQUIRE(top->timestamp_ == 15);         // item#1 rescheduled timestamp
        REQUIRE(top == item1);
        //REQUIRE(item1->ack_received_);
        processed += tracker.poll_one(15, &ack_received);
        REQUIRE(processed == 2);
        REQUIRE(ack_received);

        REQUIRE(tracker.tracked_.size() == 1);  // item#1 (hello5) ACK received and removed from tracked
        unsigned size = tracker.queue_.size();
        REQUIRE(size == 1);
    }
}
