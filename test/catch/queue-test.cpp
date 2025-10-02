#include <catch2/catch_all.hpp>

#include <queue>

#include <estd/queue.h>
#include <estd/string.h>
#include <estd/string_view.h>

#include "test-data.h"

using namespace estd;
using namespace estd::test;

TEST_CASE("queue-test")
{
    SECTION("Basic layer1 queue")
    {
        layer1::queue<int, 4> queue;

        queue.push(3);

        REQUIRE(queue.front() == 3);
        REQUIRE(queue.size() == 1);

        queue.push(4);

        REQUIRE(queue.front() == 3);
        REQUIRE(queue.size() == 2);

        queue.pop();

        REQUIRE(queue.front() == 4);
        REQUIRE(queue.size() == 1);
    }
    SECTION("Rollover layer1 queue")
    {
        layer1::queue<int, 4> queue;

        queue.push(1);
        queue.push(2);
        queue.push(3);
        queue.push(4);

        REQUIRE(queue.front() == 1);
        REQUIRE(queue.size() == 4);

        queue.pop();

        queue.push(5);

        REQUIRE(queue.size() == 4);

        REQUIRE(queue.front() == 2);
        queue.pop();
        queue.push(6);

        for(int i = 3; i <= 6; i++)
        {
            REQUIRE(queue.front() == i);
            queue.pop();
        }
    }
    SECTION("layer1 circular queue + aligned storage")
    {
        //queue<int, layer1::deque<int, 4, experimental::aligned_storage_array_policy > > queue;
        queue<int, layer1::deque<int, 4> > queue;

        queue.push(5);
        queue.push(10);
        REQUIRE(queue.front() == 5);
        REQUIRE(queue.pop());
        REQUIRE(queue.front() == 10);
        REQUIRE(queue.pop());
        REQUIRE(queue.empty());
    }
    SECTION("Emplacement tests")
    {
        layer1::queue<Dummy, 4> queue;

        queue.emplace(4, "hi there");

        int size = queue.size();

        REQUIRE(size == 1);
        REQUIRE(queue.back().val1 == 4);
        REQUIRE(queue.pop());
    }
    SECTION("Move tests")
    {
        queue<Dummy, layer1::deque<Dummy, 4 > > q;

        Dummy value1;

        value1.val1 = 7;
        value1.value2 = "hi there 1";

        q.push(std::move(value1));

        value1.val1 = 8;

        q.push(std::move(value1));

        REQUIRE(q.front().val1 == 7);
        q.pop();
        REQUIRE(q.front().val1 == 8);

        value1.val1 = 9;

        q.push(std::move(value1));

        value1.val1 = 10;

        q.push(std::move(value1));

        q.pop();
        REQUIRE(q.front().val1 == 9);
        q.pop();
        REQUIRE(q.size() == 1);

        SECTION("move whole queue")
        {
            queue<Dummy, layer1::deque<Dummy, 4 > > q2(std::move(q));
        }
    }
    SECTION("layer1 iterators")
    {
        layer1::deque<int, 10> queue;
        typedef decltype (queue)::forward_iterator iterator;

        queue.push_back(1);
        queue.push_back(2);

        REQUIRE(queue.size() == 2);

        SECTION("manual")
        {
            // DEBT: Bring this back for GH#144 mode
#if FEATURE_ESTD_GH144 == 0
            iterator i(queue, &queue.front());
#else
            iterator i(queue, &queue.front(), 0);
#endif

            REQUIRE(*i++ == 1);
            REQUIRE(*i++ == 2);
        }
        SECTION("ranged 1")
        {
            int counter = 0;

            for(auto& i : queue)
            {
                counter++;
                REQUIRE(i == counter);
            }

            REQUIRE(counter == 2);
        }
        SECTION("ranged 2")
        {
            int counter = 0;

            // SHOULD be same logic as ranged 1... I am just extra cautious
            for(iterator i = queue.begin();i != queue.end(); ++i)
            {
                ++counter;
            }

            REQUIRE(counter == 2);

            iterator i = queue.begin();
            REQUIRE(*i++ == 1);
            REQUIRE(*i++ == 2);
        }
    }
    SECTION("edge case layer1 deque operations")
    {
        layer1::deque<int, 10> queue;

        queue.push_front(1);

        REQUIRE(queue.back() == 1);
        REQUIRE(queue.front() == 1);
        REQUIRE(queue.size() == 1);

        queue.push_front(2);

        REQUIRE(queue.back() == 1);
        REQUIRE(queue.front() == 2);
        REQUIRE(queue.size() == 2);

        queue.pop_back();

        REQUIRE(queue.back() == 2);
        REQUIRE(queue.front() == 2);
        REQUIRE(queue.size() == 1);

        queue.pop_front();

        REQUIRE(queue.size() == 0);
    }
    SECTION("layer3")
    {
        int storage[4] {};
        span<int, 4> s(storage);
        using queue_type = queue<Dummy, layer3::ring<int, ring_options::sentinel>>;

        // NOTE: Default constructor does work here but you probably
        // don't want it.  Only way to use queue instantiated that way
        // is to reassign it
        queue_type q;

        q = queue_type{in_place_t{}, s};

        q.push(1);
        q.push(2);

        REQUIRE(q.front() == 1);

        q.pop();

        REQUIRE(q.size() == 1);
    }
}
