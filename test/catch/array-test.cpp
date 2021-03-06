#include <catch.hpp>

#include "estd/vector.h"
#include "estd/array.h"

#include "test-data.h"
#include "mem.h"

using namespace estd;

static int static_values[] = { 1, 2, 3, 4 };


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
        layer2::array<int, 5> array1(buf);

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

        auto array2 = layer2::make_array(static_values);

        // compares entirety of static_values to subset of array1
        // (excluding first 0 and terminating early since array2 is smaller)
        bool result = estd::equal(array2.begin(), array2.end(), array1.begin() + 1);

        REQUIRE(result);
    }
    SECTION("Array layer3")
    {
        int buf[5];
        layer3::array<int> array1(buf);

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

        auto array2 = layer3::make_array(static_values);

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
    SECTION("Experimental layer0 array")
    {
        // something about gcc doesn't like how I'm doing static values here
#ifndef __GNUC__
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
    }
}
