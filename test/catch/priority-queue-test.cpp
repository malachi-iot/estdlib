//
// Created by malachi on 5/21/18.
//

#include <catch.hpp>

#include <estd/queue.h>
#include <queue>
#include <estd/internal/priority_queue.h>

TEST_CASE("priority-queue-test")
{
    SECTION("std Priority queue")
    {
        // Doesn't work because of no stateful allocator awareness
        std::priority_queue<int, estd::layer1::vector<int, 20> > pq;

        // compiles but doesn't quite work yet.  Our handle_with_offset goodies probably
        // need some debugging.  Unfortunately, debugger acting strangely and no ambient
        // variable inspection is working, making debugging prohibitively difficult here
        pq.push(5);
        pq.push(3);
        pq.push(8);

        REQUIRE(!pq.empty());

        REQUIRE(pq.size() == 3);

        const int& val = pq.top();
        const int* _val = &val;

        REQUIRE(_val == NULLPTR);
        //REQUIRE(val == 3); // crashes, due (probably) to a NULL exception
        //pq.pop();
        //REQUIRE(pq.top() == 5);
    }
    SECTION("estd priority queue layer 1")
    {
        estd::layer1::priority_queue<int, 10> pq;

        pq.push(5);
        pq.push(3);
        pq.push(8);

        REQUIRE(pq.top() == 8);

        pq.pop();

        REQUIRE(pq.top() == 5);
    }
}