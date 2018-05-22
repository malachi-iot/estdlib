#include <catch.hpp>

#include <estd/queue.h>
#include <queue>

using namespace estd;

struct Dummy
{
    int val1;
    const char* value2;

    // because underlying struct is an array for layer1::queue, darnit
    Dummy() {}

    Dummy(int val1, const char* val2) :
        val1(val1), value2(val2)
        {}
};

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
        queue<int, layer1::deque<Dummy, 4 > > queue;

        queue.emplace(4, "hi there");

        REQUIRE(queue.size() == 1);
        REQUIRE(queue.back().val1 == 4);
        REQUIRE(queue.pop());
    }
    SECTION("std Priority queue")
    {
        // almost, but iterator needs a subtraction (-) operator
        std::priority_queue<int, estd::layer1::vector<int, 20> > pq;

        pq.push(5);
        pq.push(3);
        pq.push(8);

        REQUIRE(!pq.empty());

        //REQUIRE(pq.top() == 3);
    }
}
