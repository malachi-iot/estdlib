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

TEST_CASE("typeinfo tests")
{
    REQUIRE(type_info<int>::name() == "int");
    REQUIRE(type_info<bool>::name() == "bool");
    REQUIRE(type_info<bool>::hashcode() != type_info<int>::hashcode());

    type_index i_bool(type_info<bool>());
    type_index i_int(type_info<int>());

    const char* result = experimental::type_name_helper3(0);
    REQUIRE(result == "long");
    result = experimental::type_name_helper3(2);
    REQUIRE(result == "int8_t");
    result = experimental::type_name_helper3(4);
    result = experimental::type_name_helper3(8);
}

#pragma GCC diagnostic pop