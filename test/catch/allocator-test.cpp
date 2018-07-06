#include <catch.hpp>

#include "estd/vector.h"
#include "estd/array.h"
#include "mem.h"

#include "estd/allocators/handle_desc.h"
#include "estd/allocators/fixed.h"

using namespace estd;
using namespace estd::internal;

template <class TAllocator>
class test_specialization;

template <class T, int N>
struct test_specialization<layer1::allocator<T, N> > :
        handle_descriptor_base<layer1::allocator<T, N>,
        true, true, true, true>
{
    typedef handle_descriptor_base<layer1::allocator<T, N>,
    true, true, true, true> base;

    test_specialization() :
        base(base::allocator_traits::invalid())
    {}
};


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
    SECTION("impl::dynamic_array_base testing")
    {
        typedef estd::layer1::allocator<int, 100 > allocator_type;
        estd::internal::impl::dynamic_array_base<allocator_type, false> dai;

        int capacity = dai.capacity();

        REQUIRE(capacity == 100);

        SECTION("test_specialization")
        {
            test_specialization<allocator_type> t;

            int sz = t.size();

            REQUIRE(sz == 100);
        }
    }
    SECTION("impl::dynamic_array testing")
    {
        SECTION("layer1")
        {
            typedef estd::layer1::allocator<int, 100 > allocator_type;
            estd::internal::impl::dynamic_array<allocator_type, void> dai;

            int capacity = dai.capacity();

            REQUIRE(capacity == 100);
        }
    }
}
