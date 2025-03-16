#include <catch2/catch.hpp>

#include "estd/vector.h"
#include "estd/array.h"
#include <estd/span.h>

#include "test-data.h"
#include "mem.h"

#include "macro/push.h"

using namespace estd;

static int static_values[] = { 1, 2, 3, 4 };
static const uint8_t static_const_values[] = { 0x12, 0x34, 0x56, 0x78 };

namespace estd { namespace test {

struct AlignmentTester
{
    void* value2;
    uint16_t val1;

    AlignmentTester(uint16_t val1) : val1{val1} {}
};

struct AlignmentTester2
{
    char cval1;
    uint16_t val1;
};

#pragma pack(push, 1)
struct AlignmentTester3
{
    uint16_t val1;
    char cval1;
};
#pragma pack(pop)

}}

template <class T, unsigned sz>
using uninitialized_array = estd::internal::uninitialized_array<T, sz>;

TEST_CASE("array/vector tests")
{
    SECTION("Array")
    {
        array<int, 5> array1;
        int i = 0;

        for(; i < 5; i++)
            array1[i] = i;

        i = 0;

        for(auto i2 : array1)
        {
            REQUIRE(i2 == i);
            i++;
        }

        int sz = sizeof(array1);

        REQUIRE(sz == sizeof(int) * 5);
    }
    SECTION("Array layer2")
    {
        int buf[5];
        legacy::layer2::array<int, 5> array1(buf);

        int i = 0;

        for(; i < 5; i++)
            array1[i] = i;

        i = 0;

        for(auto i2 : array1)
        {
            REQUIRE(i2 == i);
            i++;
        }

        int sz = sizeof(array1);

        REQUIRE(sz == sizeof(int*));

        auto array2 = legacy::layer2::make_array(static_values);

        // compares entirety of static_values to subset of array1
        // (excluding first 0 and terminating early since array2 is smaller)
        bool result = estd::equal(array2.begin(), array2.end(), array1.begin() + 1);

        REQUIRE(result);
    }
    SECTION("Array layer3")
    {
        int buf[5];
        legacy::layer3::array<int> array1(buf);

        int i = 0;

        for(; i < 5; i++)
            array1[i] = i;

        i = 0;

        for(auto i2 : array1)
        {
            REQUIRE(i2 == i);
            i++;
        }

        int sz = sizeof(array1);

        // in theory we could deduce a smaller size_t, but declaring
        // 'layer3::array<int>' commits us to a full size_t
        REQUIRE(sz == sizeof(int*) + sizeof(size_t));

        auto array2 = legacy::layer3::make_array(static_values);

        sz = sizeof(array2);

        REQUIRE(estd::is_same<decltype(array2)::size_type, uint8_t>::value);
        // it's still big, because of padding.  above line verifies it is in fact
        // a uint8_t
        //REQUIRE(sz == sizeof(int*) + 1);

        bool result = estd::equal(array2.begin(), array2.end(), array1.begin() + 1);

        REQUIRE(result);
    }
    SECTION("Array Move Constructor")
    {
        array<test::NonCopyable, 5> array1;

        int i = 0;

        for(; i < 5; i++)
            array1[i].val = i;

        SECTION("moving array")
        {
            array<test::NonCopyable, 5> moved_array1(std::move(array1));

            REQUIRE(moved_array1[4].val == 4);
        }

    }
    SECTION("initialization")
    {
        SECTION("standard")
        {
            array<int, 4> a = to_array(static_values);

            bool result = estd::equal(a.begin(), a.end(), static_values);

            REQUIRE(result);
        }
        SECTION("from const")
        {
            array<uint8_t, 4> a = to_array(static_const_values);

            bool result = estd::equal(a.begin(), a.end(), static_const_values);

            REQUIRE(result);
        }
    }
    SECTION("Experimental layer0 array")
    {
        // something about gcc doesn't like how I'm doing static values here
#ifndef __GNUC__
        // I think this code is obsolete now
#if UNUSED
        experimental::layer0::array_exp2<int, static_values, 4> array1;
        int counter = 1;

        for(auto i : array1)
        {
            REQUIRE(i == counter);
            counter++;
        }

        REQUIRE(counter == 5);

        int sz = sizeof(array1);

        // even 0 size data structures in C++ report as 1 most of the time
        REQUIRE(sz == 1);

        //experimental::layer0::make_array(static_values);
#endif
#endif
    }
    SECTION("aligned_array")
    {
        constexpr int sz = 10;

        SECTION("deep dive")
        {
            uninitialized_array<test::AlignmentTester, sz> array;
            estd::span<test::AlignmentTester, sz> span(array.data());

            test::AlignmentTester& d5 = array[5];

            d5.val1 = 123;
            array[4].val1 = 456;

            REQUIRE(&d5 == &span[5]);
            REQUIRE(d5.val1 == span[5].val1);
            REQUIRE(d5.val1 != span[4].val1);
            REQUIRE(span[4].val1 == 456);
        }
        SECTION("various length checks")
        {
            REQUIRE(sizeof(uninitialized_array<int, sz>) == sizeof(int[sz]));
            REQUIRE(sizeof(uninitialized_array<test::Dummy, sz>) == sizeof(test::Dummy[sz]));
            REQUIRE(sizeof(uninitialized_array<test::AlignmentTester2, sz>) == sizeof(test::AlignmentTester2[sz]));
            REQUIRE(sizeof(uninitialized_array<test::AlignmentTester3, sz>) == sizeof(test::AlignmentTester3[sz]));
        }
        SECTION("deep dive: odd size")
        {
            uninitialized_array<test::AlignmentTester3, sz> array;
            test::AlignmentTester3 array2[sz];

            // Just to get debugger here
            array[0].val1 = 123;
        }
        SECTION("adapted to allocator")
        {
            estd::internal::single_fixedbuf_allocator<int, sz, uninitialized_array<int, sz> > alloc;
            typedef estd::allocator_traits<decltype(alloc)> traits;

            auto h = traits::allocate(alloc, 10);
            int& p = traits::lock(alloc, h);
            const int* const _p = &p;

            REQUIRE(_p != nullptr);

            // NOTE: noop
            traits::unlock(alloc, h);

            SECTION("vector")
            {
                typedef estd::internal::single_fixedbuf_allocator<test::AlignmentTester, sz,
                    uninitialized_array<test::AlignmentTester, sz> > allocator_type;
                estd::vector<test::AlignmentTester, allocator_type> v;

                v.push_back(7);

                REQUIRE(v.size() == 1);
                REQUIRE(v.max_size() == 10);

                test::AlignmentTester& front = v.front();

                REQUIRE(front.val1 == 7);

                v.pop_back();

                REQUIRE(v.size() == 0);
                REQUIRE(v.empty());
            }
        }
    }
}

#include "macro/pop.h"