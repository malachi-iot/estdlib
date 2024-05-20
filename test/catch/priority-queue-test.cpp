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

#if FEATURE_ESTD_ALLOCATED_ARRAY_TRADITIONAL
        REQUIRE(pq.top().val1 == 9);
        pq.pop();
        REQUIRE(pq.top().val1 == 7);
        pq.pop();
#else
        REQUIRE(pq.top().lock().val1 == 9);
        pq.pop();
        REQUIRE(pq.top().lock().val1 == 7);
        pq.pop();
#endif
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

#if FEATURE_ESTD_ALLOCATED_ARRAY_TRADITIONAL
        REQUIRE(pq.top().val1 == 9);
        pq.pop();;
        REQUIRE(pq.top().val1 == 8);
        pq.pop();;
        REQUIRE(pq.top().val1 == 7);
        pq.pop();
        REQUIRE(pq.top().val1 == 6);
        pq.pop();
        REQUIRE(pq.top().val1 == 5);
        pq.pop();
        REQUIRE(pq.top().val1 == 4);
        pq.pop();
#else
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
#endif
    }
    SECTION("experimental")
    {
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

        SECTION("erase")
        {
            auto f = estd::find_if(pq.container().begin(), pq.container().end(),
                [&](Dummy& v){ return v.value2 == d7.value2; });

            REQUIRE(f->val1 == 7);

            pq.erase(*f);
        }
        SECTION("erase_if")
        {
            pq.erase_if([&](Dummy& v) { return v.value2 == d7.value2; });
        }

#if FEATURE_ESTD_ALLOCATED_ARRAY_TRADITIONAL
        REQUIRE(pq.top().val1 == 9);
        pq.pop();
        REQUIRE(pq.top().val1 == 5);
        pq.pop();
        REQUIRE(pq.top().val1 == 3);
        pq.pop();
#else
        REQUIRE(pq.top().lock().val1 == 9);
        pq.pop();
        REQUIRE(pq.top().lock().val1 == 5);
        pq.pop();
        REQUIRE(pq.top().lock().val1 == 3);
        pq.pop();
#endif

        REQUIRE(pq.container().size() == 0);
    }
}
