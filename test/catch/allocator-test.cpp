#include <catch.hpp>

#include <estd/vector.h>
#include <estd/array.h>
#include "mem.h"

#include "estd/allocators/handle_desc.h"
#include "estd/allocators/fixed.h"

#include "macro/push.h"

using namespace estd;

template <class TAllocator>
struct test_specialization;

template <class T, size_t N>
struct test_specialization<layer1::allocator<T, N> > :
        internal::handle_descriptor_base<layer1::allocator<T, N>,
        true, true, true, true>
{
    typedef internal::handle_descriptor_base<layer1::allocator<T, N>,
    true, true, true, true> base;

    test_specialization() :
        base(base::allocator_traits::invalid())
    {}
};

TEST_CASE("allocator tests")
{
    SECTION("fixed allocator handle descriptor")
    {
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        typedef layer1::allocator<int, 10> allocator_t;
#else
        typedef single_fixedbuf_allocator<int, 10> allocator_t;
#endif

        REQUIRE(sizeof(allocator_t) == sizeof(int) * 10);

        // at first glance, it may seem odd to have an inline/value
        // instead of a reference for handle_descriptor.  However, this
        // is typical of how std containers track their allocators, and
        // fitting as an underpinning of our allocated_array
        SECTION("inline")
        {
            handle_descriptor<allocator_t> d;

            unsigned szof = sizeof(d);

            int size = d.size();

            REQUIRE(szof == size * sizeof(int));

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
        estd::internal::impl::dynamic_array_base<allocator_type, false, false> dai;

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
    SECTION("tag detection")
    {
        typedef estd::layer1::allocator<int, 100 > allocator_type;
        typedef allocator_type::is_pinned_tag_exp test_typedef;

        allocator_type a;

        int& val = a.lock(0);

        // FIX: something is wrong here, this should work
        REQUIRE(internal::has_is_pinned_tag_exp_typedef<allocator_type>::value);
    }
    SECTION("traits")
    {
        using traits = estd::allocator_traits<std::allocator<int> >;

        REQUIRE(traits::is_contiguous_exp);

        int v[] = { 0, 1, 2, 3 };

        // DEBT: A tiny bit concerning since we lightly frown on turning a reference
        // to a pointer to then do pointer math on.  Not terrible, but would like to avoid that
        traits::iterator::accessor a(v[0]);
        traits::iterator it(a);

        ++it;

        REQUIRE(*it == 1);
    }
    SECTION("constexpr")
    {
        constexpr const char* backing = "Hello";
        layer2::allocator<const char, 128> a(backing);
    }
    SECTION("experimental")
    {
    }
}

#include "macro/pop.h"
