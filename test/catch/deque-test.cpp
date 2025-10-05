#include <catch2/catch_all.hpp>

#include <random>

#include <estd/deque.h>
#include <estd/string.h>
#include <estd/string_view.h>

#include "test-data.h"

using namespace estd;
using namespace estd::test;

template <class Policy, class Mutex = internal::circular_mutex_noop>
void circular_queue_test(internal::circular_queue<Policy>& q1, Mutex&& mutex = {})
{
    const Dummy d1(7, "Hi 7"), d3(9, "Hi 9");
    //const unsigned N = q1.max_size();

    REQUIRE(q1.empty());
    q1.push_back(d1, std::forward<Mutex>(mutex));
    q1.emplace_back(8, "Hi 8");
    REQUIRE(!q1.empty());
    REQUIRE(q1.size() == 2);
    REQUIRE(q1.front() == d1);
    q1.pop_front(std::forward<Mutex>(mutex));
    REQUIRE(!q1.empty());
    REQUIRE(q1.size() == 1);
    REQUIRE(q1.front().val1 == 8);
    q1.pop_front(std::forward<Mutex>(mutex));
    REQUIRE(q1.empty());
}

template <class Policy, class Mutex = internal::circular_mutex_noop>
void circular_queue_rollover_test(internal::circular_queue<Policy>& q, Mutex&& mutex = {})
{
    const int N = q.max_size();
    int i;

    for(i = 0; i < N; ++i)
        q.emplace_back_mutex(std::forward<Mutex>(mutex), i, "synthetic");

    bool r = q.emplace_back_mutex(std::forward<Mutex>(mutex), N, "rolled over");
    REQUIRE(r);

    REQUIRE(q[0].val1 == 1);
    REQUIRE(q[N - 1].val1 == N);

    i = 1;

    for(const Dummy& d : q)
    {
        INFO(i);
        REQUIRE(d.val1 == i);
        ++i;
    }

    REQUIRE(i == N + 1);

    for(i = 0; i < N; ++i)
    {
        INFO(i);
        REQUIRE(q.empty() == false);
        q.pop_back();
    }
    REQUIRE(q.empty());
}

template <class Policy, class Mutex = internal::circular_mutex_noop>
void circular_queue_reverse_test(internal::circular_queue<Policy>& q, Mutex&& mutex = {})
{
    const int N = q.max_size();
    int i;

    for(i = 0; i < N; ++i)
        q.emplace_back_mutex(std::forward<Mutex>(mutex), i, "synthetic");

    --i;

    const auto end = q.rend();

    for(auto it = q.rbegin(); it != end; --i, ++it)
    {
        INFO(i);
        REQUIRE(it->val1 == i);
    }
}


template <class Policy, class Mutex = internal::circular_mutex_noop>
void circular_queue_move_test(internal::circular_queue<Policy>& q, Mutex&& mutex = {})
{
    Dummy d1(7, "Hi 7");

    q.push_back(std::move(d1), std::forward<Mutex>(mutex));
    q.push_back(Dummy{8, "Hi 8"}, std::forward<Mutex>(mutex));

    REQUIRE(q.front().moved_);
    REQUIRE(q.front().val1 == 7);
    REQUIRE(d1.moved_from_);

    q.pop_front(std::forward<Mutex>(mutex));
    REQUIRE(q.front().val1 == 8);
    REQUIRE(q.front().moved_);

    new (&d1) Dummy(77, "Hi 77");

    q.push_front(std::move(d1), std::forward<Mutex>(mutex));

    REQUIRE(q.size() == 2);
    REQUIRE(d1.moved_from_);
    REQUIRE(q.front().val1 == 77);
}

template <class Policy, class Mutex>
void circular_queue_bulk_test(internal::circular_queue<Policy>& q1, Mutex&& mutex)
{
    // Always seed to the (default) same thing, thus ensuring identical inputs &
    // outputs
    std::default_random_engine e1, e2;

    // Prime things
    q1.push_back(Dummy(static_cast<int>(e1()), ""), std::forward<Mutex>(mutex));

    for(int i = 0; i < 100; ++i)
    {
        CAPTURE(i);
        unsigned v = q1.front().val1;
        q1.pop_front(std::forward<Mutex>(mutex));
        REQUIRE(v == e2());
        q1.push_back(Dummy(static_cast<int>(e1()), ""), std::forward<Mutex>(mutex));
    }
}


template <class Policy, class Mutex = internal::circular_mutex_noop>
void circular_queue_test_suite(internal::circular_queue<Policy>& q1, Mutex&& mutex = {})
{
    q1.clear();
    circular_queue_test(q1, std::forward<Mutex>(mutex));
    q1.clear();
    circular_queue_rollover_test(q1, std::forward<Mutex>(mutex));
    q1.clear();
    circular_queue_reverse_test(q1, std::forward<Mutex>(mutex));
    q1.clear();
    circular_queue_move_test(q1, std::forward<Mutex>(mutex));
    q1.clear();
    circular_queue_bulk_test(q1, std::forward<Mutex>(mutex));
}

TEST_CASE("deque-test")
{
    SECTION("deque")
    {
        SECTION("pointers")
        {
            layer1::deque<const int*, 10> queue;
            using value_type = decltype (queue)::value_type;

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
    }
    SECTION("ring")
    {
        Dummy d1(7, "Hi 7"), d3(9, "Hi 9");

        using options = ring_options;

        SECTION("default")
        {
            using queue_type = layer1::ring<Dummy, 4>;
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
        SECTION("default: array element")
        {
            using queue_type = layer1::ring<char[32], 4>;
            queue_type q;
            int counter = 0;
            auto op = [&](char (*v)[32])
            {
                // NOTE: Old clumsy (but effective) syntax to empty-init string.
                // layer2 strings are designed to pick up existing strings like
                // a string_view does, thus the confusion.  Really a kind of
                // in_place_non_init_t or something might be better
                layer2::string<32> s(*v, 0);

                s += '#';
                s += to_string(++counter);
            };

            q.push_back_op(op);
            q.push_back_op(op);

            string_view s1 = q.front();

            REQUIRE(s1 == "#1");

            q.pop_front();

            s1 = q.front();

            REQUIRE(s1 == "#2");
        }
        SECTION("flagged")
        {
            using queue_type = layer1::ring<Dummy, 4, options::flagged>;

            static_assert(queue_type::type == options::flagged, "flagged mode expected");

            queue_type q1, q2;

            for(unsigned i = 0; i < q2.max_size(); ++i)
                q2.emplace_back(i, "synthetic");

            REQUIRE(q2.size() == 4);

            REQUIRE(q2.back().val1 == 3);

            // Force rollover
            q2.emplace_back(1000, "dummy");

            REQUIRE(q2.normalize_pos(0) == 1);
            REQUIRE(q2.normalize_pos(2) == 3);
            REQUIRE(q2.normalize_pos(3) == 0);

            REQUIRE(q2[2].val1 == 3);
            REQUIRE(q2[3].val1 == 1000);

            q2.clear();

            circular_queue_test_suite(q1);
        }
        SECTION("counter: layer1")
        {
            layer1::ring<Dummy, 4, options::counter> q1;

            REQUIRE(q1.max_size() == 4);

            circular_queue_test_suite(q1);
        }
        SECTION("counter: layer2")
        {
            std::array<Dummy, 4> storage;
            layer2::ring<Dummy, 4, options::counter> q1(in_place_t{}, storage.data());

            REQUIRE(q1.max_size() == 4);

            circular_queue_test_suite(q1);
        }
        SECTION("sentinel: layer1")
        {
            using queue_type = layer1::ring<Dummy, 4, options::sentinel>;
            queue_type q1;

            REQUIRE(q1.max_size() == 3);

            circular_queue_test_suite(q1);

            SECTION("mutex things")
            {
                internal::circular_mutex_synthetic mutex;
                circular_queue_test_suite(q1, mutex);
                circular_queue_test_suite(q1, internal::circular_mutex_std{});

                REQUIRE(mutex.front_ == 214);
                REQUIRE(mutex.count_ == 0);     // Sentinel needs no count/flag
            }
        }
        SECTION("sentinel: layer2")
        {
            std::array<Dummy, 4> storage;
            using queue_type = layer2::ring<Dummy, 4, options::sentinel>;
            queue_type q1(in_place_t{}, storage.data());

            REQUIRE(q1.max_size() == 3);

            circular_queue_test_suite(q1);

            queue_type copied(q1);
        }
        SECTION("sentinel: layer3")
        {
            std::array<Dummy, 4> storage;
            using queue_type = layer3::ring<Dummy, options::sentinel>;
            queue_type q1(in_place_t{}, storage);

            REQUIRE(q1.max_size() == 3);

            circular_queue_test_suite(q1);
        }
        SECTION("packed: layer1")
        {
            // Not ready yet
            //using queue_type = layer1::ring<Dummy, 4, options::packed>;

            //queue_type q1;
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
            using queue_type = layer1::ring<Dummy, 4, options::sentinel | options::atomic>;

            queue_type q1;

            circular_queue_test_suite(q1);
        }
        SECTION("no_rollover")
        {
            SECTION("atomic")
            {
                using queue_type = layer1::ring<
                    int, 4,
                    options::no_rollover | options::atomic>;

                queue_type q1;

            }
            SECTION("sentinel")
            {
                using queue_type = layer1::ring<
                    int, 4,
                    options::sentinel | options::no_rollover | options::strict>;

                queue_type q1;

                q1.push_back(0);
                q1.push_back(1);
                REQUIRE(q1.push_back(2));
                REQUIRE(q1.push_back(3) == nullptr);
            }
        }
    }
}
