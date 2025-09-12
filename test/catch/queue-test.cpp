#include <catch2/catch_all.hpp>

#include <estd/queue.h>
#include <queue>
#include "test-data.h"

using namespace estd;
using namespace estd::test;

template <class Policy>
void circular_queue_test(internal::circular_queue<Policy>& q1)
{
    const Dummy d1(7, "Hi 7"), d3(9, "Hi 9");
    //const unsigned N = q1.max_size();

    REQUIRE(q1.empty());
    q1.push_back(d1);
    q1.emplace_back(8, "Hi 8");
    REQUIRE(!q1.empty());
    REQUIRE(q1.size() == 2);
    REQUIRE(q1.front() == d1);
    q1.pop_front();
    REQUIRE(!q1.empty());
    REQUIRE(q1.size() == 1);
    REQUIRE(q1.front().val1 == 8);
    q1.pop_front();
    REQUIRE(q1.empty());
}

template <class Policy>
void circular_queue_rollover_test(internal::circular_queue<Policy>& q)
{
    const int N = q.max_size();
    int i;

    for(i = 0; i < N; ++i)
        q.emplace_back(i, "synthetic");

    bool r = q.emplace_back(N, "rolled over");
    REQUIRE(r);

    i = 1;

    for(const Dummy& d : q)
    {
        INFO(i);
        REQUIRE(d.val1 == i);
        ++i;
    }

    REQUIRE(i == N + 1);
}

template <class Policy>
void circular_queue_reverse_test(internal::circular_queue<Policy>& q)
{
    const int N = q.max_size();
    int i;

    for(i = 0; i < N; ++i)
        q.emplace_back(i, "synthetic");

    --i;

    const auto end = q.rend();

    for(auto it = q.rbegin(); it != end; --i, ++it)
    {
        INFO(i);
        REQUIRE(it->val1 == i);
    }
}


template <class Policy>
void circular_queue_test_suite(internal::circular_queue<Policy>& q1)
{
    circular_queue_test(q1);
    q1.clear();
    circular_queue_rollover_test(q1);
    q1.clear();
    circular_queue_reverse_test(q1);
}

template <class T, unsigned N, internal::queue_options o = internal::queue_options::default_opt>
using layer1_circular = internal::circular_queue<internal::array_circular_policy<T, N, o>>;

template <class T, unsigned N, internal::queue_options o = internal::queue_options::default_opt>
using layer2_circular = internal::circular_queue<internal::span_circular_policy<T, N, o>>;

template <class T, unsigned N, internal::queue_options o = internal::queue_options::default_opt>
using layer3_circular = internal::circular_queue<internal::span_circular_policy<T, estd::detail::dynamic_extent::value, o>>;

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
    SECTION("dequeue pointers")
    {
        layer1::deque<const int*, 10> queue;
        typedef decltype (queue)::value_type value_type;

        REQUIRE(estd::is_same<const int*, value_type>::value);

        int val1 = 5;
        const int* real_ptr = &val1;

        queue.push_front(&val1);

        REQUIRE(!queue.empty());

        const int* out = queue.back();

        REQUIRE(out != NULLPTR);
        REQUIRE(out == real_ptr);
        REQUIRE(*out == 5);

        out = queue.front();

        REQUIRE(out != NULLPTR);
        REQUIRE(out == real_ptr);
        REQUIRE(*out == 5);

        queue.pop_front();

        REQUIRE(queue.empty());
    }
    SECTION("Emplacement tests")
    {
        queue<Dummy, layer1::deque<Dummy, 4 > > queue;

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
            iterator i(queue, &queue.front());

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
    SECTION("circular")
    {
        Dummy d1(7, "Hi 7"), d3(9, "Hi 9");

        using options = internal::queue_options;

        SECTION("default")
        {
            using queue_type = layer1_circular<Dummy, 4>;
            queue_type q1;

            REQUIRE(q1.size() == 0);

            q1.push_back(d1);
            q1.emplace_back(8, "Hi 8");

            REQUIRE(q1.size() == 2);

            SECTION("test1")
            {
                queue_type::iterator it1 = q1.begin();

                REQUIRE(q1.front() == d1);
                REQUIRE(*it1++ == d1);
                // DEBT: Not quite sure if stock iterator will do this
                REQUIRE(it1->val1 == 8);

                q1.pop_front();

                REQUIRE(q1.front().val1 == 8);
            }
            SECTION("test2")
            {
                q1.push_back(d3);
                REQUIRE(q1.size() == 3);
                q1.emplace_back(10, "Hi 10");
                // rollover+overwrite
                q1.emplace_back(11, "Hi 11");
                //unsigned sz = q1.size();
                //REQUIRE(q1.size() == 4);
            }
        }
        SECTION("flagged")
        {
            using queue_type = layer1_circular<Dummy, 4, options::flagged>;

            static_assert(queue_type::type == options::flagged);

            queue_type q1, q2;

            circular_queue_test(q1);

            for(unsigned i = 0; i < q2.max_size(); ++i)
                q2.emplace_back(i, "synthetic");

            REQUIRE(q2.size() == 4);

            REQUIRE(q2.back().val1 == 3);
            q2.pop_back();
            REQUIRE(q2.empty() == false);
            q2.pop_back();
            REQUIRE(q2.empty() == false);
            q2.pop_back();
            REQUIRE(q2.empty() == false);
            q2.pop_back();
            REQUIRE(q2.empty());

            q1.clear();
            q2.clear();

            circular_queue_test_suite(q1);
        }
        SECTION("counter: layer1")
        {
            layer1_circular<Dummy, 4, options::counter> q1;

            REQUIRE(q1.max_size() == 4);

            circular_queue_test_suite(q1);
        }
        SECTION("counter: layer2")
        {
            std::array<Dummy, 4> storage;
            layer2_circular<Dummy, 4, options::counter> q1(storage.data());

            REQUIRE(q1.max_size() == 4);

            circular_queue_test_suite(q1);
        }
        SECTION("sentinel: layer1")
        {
            using queue_type = layer1_circular<Dummy, 4, options::sentinel>;
            queue_type q1;

            REQUIRE(q1.max_size() == 3);

            circular_queue_test_suite(q1);
        }
        SECTION("sentinel: layer2")
        {
            std::array<Dummy, 4> storage;
            using queue_type = layer2_circular<Dummy, 4, options::sentinel>;
            queue_type q1(storage.data());

            REQUIRE(q1.max_size() == 3);

            circular_queue_test_suite(q1);
        }
        SECTION("sentinel: layer3")
        {
            std::array<Dummy, 4> storage;
            using queue_type = layer3_circular<Dummy, 4, options::sentinel>;
            queue_type q1(storage.data(), 4);

            REQUIRE(q1.max_size() == 3);

            circular_queue_test_suite(q1);
        }
        SECTION("atomic | bare")
        {
            //using queue_type = layer1_circular<Dummy, 4, options::atomic | options::bare>;

            // FIX: bare has no 'empty' - though only FIX because we haven't ironed out scope
            // of 'bare' behavior fully
            //queue_type q1;

            //q1.push_back(d1);
            //q1.emplace_back(8, "Hi 8");
        }
        SECTION("atomic | sentinel")
        {
            using queue_type = layer1_circular<Dummy, 4, options::sentinel | options::atomic>;

            queue_type q1;

            circular_queue_test_suite(q1);
        }
    }
}
