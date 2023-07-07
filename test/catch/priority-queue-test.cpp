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
    SECTION("std priority queue")
    {
        // NOTE: Unlake make_heap, this behaves identically with estd flavor, because
        // currently our priority_queue uses std::make_heap/std::push_heap
        std::priority_queue<int> pq;

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
        estd::layer1::vector<int, 10> values2 = { 10, 50, 90, 30, 20, 0 };
        // FIX: begin2/end2 don't work with out make_heap/push_heap
        auto begin2 = values2.begin();
        auto end2 = values2.end();
        int* begin = values;
        int* end = values + sizeof(values) / sizeof(values[0]);

        SECTION("make_heap")
        {
            // FIX: our implementation is backwards from std
            estd::experimental::make_heap(begin, end, [](int a, int b){ return a < b; });

            REQUIRE(values[0] == 0);
            REQUIRE(values[1] == 2);
            REQUIRE(values[2] == 1);
            REQUIRE(values[3] == 3);
            REQUIRE(values[4] == 5);
            REQUIRE(values[5] == 9);
        }
        SECTION("push_heap")
        {
            estd::experimental::make_heap(begin2, values2.end(), estd::less<int>{});

            REQUIRE(values2[0] == 0);
            REQUIRE(values2[1] == 20);
            REQUIRE(values2[2] == 10);
            REQUIRE(values2[3] == 30);
            REQUIRE(values2[4] == 50);
            REQUIRE(values2[5] == 90);

            values2.push_back(5);

            estd::experimental::push_heap(begin2, values2.end(), estd::less<int>{});

            // FIX: Well that's not right
            //REQUIRE(values2[6] == 5);
        }
        SECTION("make_heap: std parity")
        {
            std::make_heap(begin, end, [](int a, int b){ return a > b; });

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
        }
    }
    SECTION("erase")
    {
        estd::layer1::priority_queue<Dummy, 10> pq;

        Dummy d7(7, "val7");

        pq.push(Dummy(5, "val5"));
        pq.push(Dummy(9, "val9"));
        pq.push(Dummy(3, "val3"));
        pq.push(d7);

        REQUIRE(pq.container().size() == 4);

        /*
        auto f = estd::find_if(pq.container().begin(), pq.container().end(),
            [&](Dummy& v){ return v.value2 == d7.value2; });

        REQUIRE((*f).val1 == 7);

        pq.erase(*f); */
        pq.erase2([&](Dummy& v) { return v.value2 == d7.value2; });

        REQUIRE(pq.top().lock().val1 == 9);
        pq.pop();
        REQUIRE(pq.top().lock().val1 == 5);
        pq.pop();
        REQUIRE(pq.top().lock().val1 == 3);
        pq.pop();

        REQUIRE(pq.container().size() == 0);
    }
}
