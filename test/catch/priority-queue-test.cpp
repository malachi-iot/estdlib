//
// Created by malachi on 5/21/18.
//

#include <catch.hpp>

#include "estd/internal/platform.h"
#include "test-data.h"

using namespace estd::test;

inline static bool compare(const Dummy& lhs, const Dummy& rhs)
{
    return lhs.val1 < rhs.val1;
}

inline bool operator <(const Dummy& lhs, const Dummy& rhs)
{
    return lhs.val1 < rhs.val1;
}

#include <estd/queue.h>
#include <queue>


TEST_CASE("priority-queue-test")
{
    SECTION("std Priority queue")
    {
#ifdef UNUSED
        // Doesn't work because of no stateful allocator awareness
        typedef estd::layer1::vector<int, 20> backing_t;
        backing_t backing;
        // should work as constructor parameters but cant figure it out yet (std::less<int>, backing);
        std::priority_queue<int, backing_t > pq;

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
#endif
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

        pq.emplace(7);

        REQUIRE(pq.top() == 7);
    }
    SECTION("priority queue, reverse sort")
    {
        estd::layer1::priority_queue<int, 10, estd::greater<int> > pq;

        pq.push(5);
        pq.push(3);
        pq.push(8);

        REQUIRE(pq.top() == 3);

        pq.pop();

        REQUIRE(pq.top() == 5);
    }
    SECTION("priority queue, custom compare")
    {
        estd::layer1::priority_queue<Dummy, 10, decltype(&compare)> pq(compare);

        pq.push(Dummy(5, "val5"));
        pq.push(Dummy(9, "val9"));
        pq.push(Dummy(3, "val3"));
        Dummy d7(7, "val7");
        pq.push(d7);

        REQUIRE(pq.top().lock().val1 == 9);
        pq.pop();
        REQUIRE(pq.top().lock().val1 == 7);
        pq.pop();
    }
    SECTION("priority queue, emplacement")
    {
        estd::layer1::priority_queue<Dummy, 10> pq;

        Dummy d7(7, "val7");

        pq.push(Dummy(5, "val5"));
        pq.push(Dummy(9, "val9"));
        pq.push(Dummy(3, "val3"));
        pq.push(d7);
        pq.emplace(4, "val4");
        pq.emplace(8, "val8");

        d7.val1 = 6;

        pq.push(d7);

        // it seems one could but shouldn't use emplace this way, since it's an alias
        // to push but going through the move mechanisms.  If you really want to move
        // I expect a std::move should be involved
        //pq.emplace(d7); // remember this effectively is an efficient 'push'

        REQUIRE(pq.top().lock().val1 == 9);
        pq.pop();;
        REQUIRE(pq.top().lock().val1 == 8);
        pq.pop();;
        REQUIRE(pq.top().lock().val1 == 7);
        pq.pop();
        REQUIRE(pq.top().lock().val1 == 6);
        pq.pop();
        REQUIRE(pq.top().lock().val1 == 5);
        pq.pop();
        REQUIRE(pq.top().lock().val1 == 4);
        pq.pop();
    }
    SECTION("experimental")
    {
        int values[] = { 1, 5, 9, 3, 2, 0 };
        int* begin = values;
        int* end = values + sizeof(values) / sizeof(values[0]);

        SECTION("make_heap")
        {
            estd::experimental::make_heap(begin, end, [](int a, int b){ return a < b; });

            REQUIRE(values[0] == 0);
            REQUIRE(values[1] == 2);
            REQUIRE(values[2] == 1);
            REQUIRE(values[3] == 3);
            REQUIRE(values[4] == 5);
            REQUIRE(values[5] == 9);
        }
        SECTION("internal heap")
        {
            estd::experimental::internal_heap<int*, estd::less<int> > heap(begin, end);

            heap.make();

            REQUIRE(heap.size() == 6);

            SECTION("front/pop")
            {
                REQUIRE(heap.front() == 0);

                heap.pop();

                REQUIRE(heap.front() == 1);

                heap.pop();

                REQUIRE(heap.front() == 2);

                heap.pop();

                REQUIRE(heap.front() == 3);

                heap.pop();

                REQUIRE(heap.front() == 5);

                heap.pop();

                REQUIRE(heap.front() == 9);
            }
            SECTION("pop + push")
            {
                heap.pop();
                heap.push(4);

                REQUIRE(heap.front() == 1);

                heap.pop();

                REQUIRE(heap.front() == 2);

                heap.pop();

                REQUIRE(heap.front() == 3);

                heap.pop();

                REQUIRE(heap.front() == 4);

                REQUIRE(heap.size() == 3);
            }
            SECTION("posh")
            {
                heap.pop();

                REQUIRE(heap.front() == 1);

                *(heap.first) = 4;

                REQUIRE(heap.front() == 4);

                heap.posh();

                REQUIRE(heap.front() == 2);

                heap.pop();

                REQUIRE(heap.front() == 3);

                heap.pop();

                REQUIRE(heap.front() == 4);

                heap.pop();
            }
        }
        SECTION("internal heap mk. 2")
        {
            int values2[] =
                { 1, 5, 9, 3, 2, 0, 98,
                  100, 509, 407, 34, 7 };
            int* begin2 = values2;
            int* end2 = values2 + sizeof(values2) / sizeof(values2[0]);

            estd::experimental::internal_heap<int*, estd::less<int> > heap(begin2, end2);

            heap.make2();

            REQUIRE(heap.size() == 12);

            SECTION("pop2")
            {
                REQUIRE(heap.front() == 0);
                REQUIRE(heap.pop2() == 1);
                REQUIRE(heap.pop2() == 2);
                REQUIRE(heap.pop2() == 3);
                REQUIRE(heap.pop2() == 5);
                REQUIRE(heap.pop2() == 7);
                REQUIRE(heap.pop2() == 9);
                REQUIRE(heap.pop2() == 34);
                REQUIRE(heap.pop2() == 98);
            }
            SECTION("posh")
            {
                heap.front() = 11;
                //heap.posh();

                heap.sift_down();

                REQUIRE(heap.front() == 1);
                REQUIRE(heap.pop2() == 2);
                REQUIRE(heap.pop2() == 3);
                REQUIRE(heap.pop2() == 5);
                REQUIRE(heap.pop2() == 7);
                REQUIRE(heap.pop2() == 9);
                REQUIRE(heap.pop2() == 11);
                REQUIRE(heap.pop2() == 34);
            }
        }
    }
}
