#include <catch.hpp>

#define FEATURE_ESTD_ALGORITHM_OPT 0

#include <estd/array.h>
#include <estd/algorithm.h>
#include <estd/vector.h>

struct test_class_1
{
    estd::array<int, 10> values;
    int searching_for = 5;

    bool predicate(int candidate)
    {
        return candidate == searching_for;
    }
};

using namespace estd;

TEST_CASE("algorithm tests")
{
    test_class_1 tc1;

    SECTION("find_if in class")
    {
        // alas, can't do this.  closures are kinda mandatory it seems...
        //estd::find_if(tc1.values.begin(), tc.values.end(), tc1.predicate);
    }
    SECTION("copy_n")
    {
        char buf1[] = "hi2u";
        char buf2[4] = {0,0,0,0};

        estd::copy_n(buf1, 4, buf2);

        REQUIRE(estd::equal(buf1, &buf1[3], buf2));
    }
    SECTION("reverse")
    {
        char buf[] = "1234";

        estd::reverse(&buf[0], &buf[4]);

        REQUIRE(std::string(buf) == "4321");
    }
    SECTION("fill")
    {
        char buf[10];

        buf[9] = 0;

        estd::fill_n(buf, 10, 7);

        REQUIRE(buf[5] == 7);
        REQUIRE(buf[9] == 7);
    }
    SECTION("copy_backward")
    {
        array<unsigned, 10> a{ 0, 1, 2, 3, 4, 5 };

        copy_backward(a.begin() + 1, a.end() - 1, a.end());

        REQUIRE(a[0] == 0);
        REQUIRE(a[1] == 1);
        REQUIRE(a[2] == 1);
        REQUIRE(a[3] == 2);
    }
    SECTION("heap")
    {
        int values[] = { 1, 5, 9, 3, 2, 0 };
        estd::layer1::vector<int, 10> values2 = { 10, 50, 90, 30, 20, 0 };
        // FIX: begin2/end2 don't work with out make_heap/push_heap
        auto begin2 = values2.begin();
        //auto end2 = values2.end();
        int* begin = values;
        int* end = values + sizeof(values) / sizeof(values[0]);

        SECTION("make_heap")
        {
            // NOTE: lambda way works too
            //estd::make_heap(begin, end, [](int a, int b){ return a > b; });
            estd::make_heap(begin, end, estd::greater<int>{});

            REQUIRE(values[0] == 0);
            REQUIRE(values[1] == 2);
            REQUIRE(values[2] == 1);
            REQUIRE(values[3] == 3);
            REQUIRE(values[4] == 5);
            REQUIRE(values[5] == 9);
        }
        SECTION("make_heap (max)")
        {
            estd::make_heap(begin, end);

            REQUIRE(values[5] == 0);
            REQUIRE(values[4] == 2);
            REQUIRE(values[3] == 3);
            REQUIRE(values[2] == 1);
            REQUIRE(values[1] == 5);
            REQUIRE(values[0] == 9);

            estd::pop_heap(begin, end);

            REQUIRE(values[0] == 5);
        }
        SECTION("push_heap")
        {
            estd::make_heap(begin2, values2.end(), estd::greater<int>{});

            REQUIRE(values2[0] == 0);
            REQUIRE(values2[1] == 20);
            REQUIRE(values2[2] == 10);
            REQUIRE(values2[3] == 30);
            REQUIRE(values2[4] == 50);
            REQUIRE(values2[5] == 90);

            values2.push_back(5);

            estd::push_heap(begin2, values2.end(), estd::greater<int>{});

            values2.push_back(95);

            estd::push_heap(begin2, values2.end(), estd::greater<int>{});

            REQUIRE(values2[0] == 0);
            REQUIRE(values2[1] == 20);
            REQUIRE(values2[2] == 5);
            REQUIRE(values2[3] == 30);
            REQUIRE(values2[4] == 50);
            REQUIRE(values2[5] == 90);
            REQUIRE(values2[6] == 10);

            estd::internal::heap<decltype(begin2), estd::greater<int> > h(begin2, values2.end());

            h.pop();
            h.pop();

            REQUIRE(h.front() == 10);

            h.pop();
            h.pop();
            h.pop();

            REQUIRE(h.front() == 50);

            h.pop();

            REQUIRE(h.front() == 90);

            h.pop();

            REQUIRE(h.front() == 95);
        }
        SECTION("make_heap: std parity")
        {
            std::make_heap(begin, end, std::greater<int>{});

            // DEBT: It's quite conceivable the numbers won't perfectly match here
            // what we really need is either a decomposer of heaps or at a minimum
            // a bunch of pop operations

            REQUIRE(values[0] == 0);
            REQUIRE(values[1] == 2);
            REQUIRE(values[2] == 1);
            REQUIRE(values[3] == 3);
            REQUIRE(values[4] == 5);
            REQUIRE(values[5] == 9);
        }
        SECTION("internal heap")
        {
            estd::internal::heap<int*, estd::greater<int> > heap(begin, end);

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
}
