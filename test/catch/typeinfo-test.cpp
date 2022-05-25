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

TEST_CASE("typeinfo tests")
{
    REQUIRE(type_info<int>::name() == "int");
    REQUIRE(type_info<bool>::name() == "bool");
    REQUIRE(type_info<bool>::hashcode() != type_info<int>::hashcode());

    type_index i_bool(type_info<bool>());
    type_index i_int(type_info<int>());
}

#pragma GCC diagnostic pop