#include <catch.hpp>

#include <estd/queue.h>
#include <queue>
#include "test-data.h"

using namespace estd;
using namespace estd::test;


TEST_CASE("queue-test")
{
    SECTION("Basic layer1 queue")
    {
        queue<int, layer1::deque<int, 4 > > queue;

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
        queue<int, layer1::deque<int, 4 > > queue;

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
    SECTION("Emplacement tests")
    {
        queue<Dummy, layer1::deque<Dummy, 4 > > queue;

        queue.emplace(4, "hi there");

        REQUIRE(queue.size() == 1);
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
}
