#include <catch.hpp>

#include <estd/type_traits.h>
#include <estd/limits.h>
#include <estd/string.h>

#include <estd/typeindex.h>
#include <estd/typeinfo.h>

#include "test-data.h"

using namespace estd;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

static const char* get_name(int index)
{
    return nullptr;
}
/*
namespace estd::experimental {

template <> struct reverse_type_info<0, 0>
{
    static const char* name() { return "zero"; }
};

template <> struct reverse_type_info<0, 1>
{
    static const char* name() { return "one"; }
};

template <> struct reverse_type_info<0, 2>
{
    static const char* name() { return "index2"; }
};

}
*/

template <typename T>
static void dispatched(layer0::type_info<T> t)
{

}

template <typename T>
static void dispatched2(T t)
{

}

TEST_CASE("typeinfo tests")
{
    SECTION("type_info")
    {
        REQUIRE(layer0::type_info<int>::name() == "int");
        REQUIRE(layer0::type_info<bool>::name() == "bool");
        REQUIRE(layer0::type_info<bool>::hash_code() != layer0::type_info<int>::hash_code());
    }
    SECTION("type_index")
    {
        layer0::type_info<bool> dummy;
        layer0::type_index i_bool (dummy);
        layer0::type_index i_int (layer0::type_info<int>{});

        REQUIRE(i_int.name() == "int");
        REQUIRE(i_bool.name() == "bool");
    }
    SECTION("internal")
    {
        const char* result = layer0::experimental::type_name_helper3(0);
        REQUIRE(result == "int");
        result = layer0::experimental::type_name_helper3(2);
        REQUIRE(result == "bool");
        result = layer0::experimental::type_name_helper3(4);
        result = layer0::experimental::type_name_helper3(8);

        bool v = layer0::experimental::_reverse_type_in_range<
            0, layer0::experimental::system_type_info_index::end, 10>::value;

        REQUIRE(v == false);

        v = layer0::experimental::_reverse_type_in_range<
            0,
            layer0::experimental::system_type_info_index::i_uint8,
            layer0::experimental::system_type_info_index::i_uint8 + 4>::value;

        REQUIRE(v == true);

        v = layer0::experimental::_reverse_type_in_range<
                0,
            layer0::experimental::system_type_info_index::i_uint8,
            layer0::experimental::system_type_info_index::i_uint8>::value;

        REQUIRE(v == false);
    }
    SECTION("dispatcher")
    {
        // Not working
        //experimental::type_dispatcher<0, 0>(0, &dispatched2);
    }
}

#pragma GCC diagnostic pop