#include <catch.hpp>

#include "estd/vector.h"
#include "estd/array.h"
#include "mem.h"

#include "estd/allocators/handle_desc.h"
#include "estd/allocators/fixed.h"

using namespace estd;
using namespace estd::internal;

TEST_CASE("allocator tests")
{
    SECTION("fixed allocator handle descriptor")
    {
        typedef single_fixedbuf_allocator<int, 10> allocator_t;

        SECTION("inline")
        {
            handle_descriptor<allocator_t> d;

            int szof = sizeof(d);

            int size = d.size();
            //d.reallocate(5);
            int* val_array = &d.lock();
        }
        // Our unit tests always have CONSTEXPR set, but sometimes I manually disable it to test for < C++11 compatibility
#ifdef FEATURE_CPP_CONSTEXPR
        SECTION("referenced")
        {
            allocator_t allocator;
            handle_descriptor<allocator_t&> d(allocator);

            int szof = sizeof(d);

            int size = d.size();
            //d.reallocate(5);
            int* val_array = &d.lock();
        }
#endif
    }
    SECTION("Unusual referenced allocator")
    {
        int buf[100];

        SECTION("layer2 ref")
        {
            typedef estd::layer2::allocator<int, 100 > allocator_type;
            allocator_type a(buf);
            estd::vector<int, allocator_type& > v(a);
            int sz = sizeof(v);

            REQUIRE(sz < 32);
        }
        SECTION("layer3 ref")
        {
            typedef estd::layer3::allocator<int> allocator_type;
            allocator_type a(buf);
            estd::vector<int, allocator_type& > v(a);
            int sz = sizeof(v);

            REQUIRE(sz < 32);
        }
    }
    SECTION("handle_descriptor")
    {
        int buf[100];

        SECTION("layer1")
        {
            typedef estd::layer1::allocator<int, 100 > allocator_type;
            estd::handle_descriptor<allocator_type> h;

            int sz = sizeof(h);
            int sz2 = h.size();

            REQUIRE(sz == 400);
            REQUIRE(sz2 == 100);
        }
        SECTION("layer1 ref")
        {
            typedef estd::layer1::allocator<int, 100 > allocator_type;
            allocator_type a;
            estd::handle_descriptor<allocator_type&> h(a);

            int sz = sizeof(h);
            int sz2 = h.size();

            REQUIRE(sz == sizeof(int*));
            REQUIRE(sz2 == 100);
        }
        SECTION("layer2")
        {
            typedef estd::layer2::allocator<int, 100 > allocator_type;
            estd::handle_descriptor<allocator_type> h(buf);

            int sz = sizeof(h);
            int sz2 = h.size();

            REQUIRE(sz == sizeof(int*));
            REQUIRE(sz2 == 100);
        }
        SECTION("layer2 ref")
        {
            typedef estd::layer2::allocator<int, 100 > allocator_type;
            allocator_type a(buf);
            estd::handle_descriptor<allocator_type&> h(a);

            int sz = sizeof(h);
            int sz2 = h.size();

            REQUIRE(sz == sizeof(int*));
            REQUIRE(sz2 == 100);
        }
    }
}
