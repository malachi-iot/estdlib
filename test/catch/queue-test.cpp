#include <catch.hpp>

#include <estd/queue.h>
#include <queue>

using namespace estd;

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
    SECTION("std Priority queue")
    {
        // almost, but iterator needs a subtraction (-) operator
        //std::priority_queue<int, estd::layer1::vector<int, 20> > pq;
    }
}
