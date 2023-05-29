#include <catch.hpp>

#include <estd/internal/variant.h>

#include "test-data.h"

#include "macro/push.h"

using namespace estd;

// None of these directly match up to 'std'

struct test_functor
{
    template <unsigned I, class T>
    bool operator()(internal::visitor_index<I, T>, int&& param)
    {
        return param == I;
    }
};

struct tuple_getter_functor
{
    template <unsigned I, class T, class ...TArgs>
    T& operator()(internal::visitor_index<I, T>, tuple<TArgs...>& t)
    {
        return get<I>(t);
    }
};



struct test2_functor
{
    template <unsigned I>
    constexpr bool operator()(in_place_index_t<I>) const { return false; }

    template <unsigned I>
    bool operator()(internal::visitor_instance<I, const char*> v)
    {
        return true;
    }
};



TEST_CASE("variadic")
{
    SECTION("misc")
    {
        SECTION("index_of_type")
        {
            SECTION("basic")
            {
                typedef estd::internal::index_of_type<int, estd::monostate, int, float> iot;
                constexpr int idx = iot::index;
                constexpr bool multiple = iot::multiple;

                REQUIRE(idx == 1);
                REQUIRE(multiple == false);
            }
            SECTION("not found")
            {
                typedef estd::internal::index_of_type<int, estd::monostate, float> iot;

                constexpr int idx = iot::index;
                constexpr bool multiple = iot::multiple;

                REQUIRE(idx == -1);
                REQUIRE(multiple == false);
            }
            SECTION("multiple")
            {
                typedef estd::internal::index_of_type<int, int, int> iot;

                constexpr int idx = iot::index;
                constexpr bool multiple = iot::multiple;

                REQUIRE(idx == 0);
                REQUIRE(multiple == true);
            }
        }
        SECTION("visitor")
        {
            SECTION("static")
            {
                typedef internal::variadic_visitor_helper2<monostate, int, float, const char*> vh_type;

                int result = vh_type::visit(test_functor{}, 1);

                REQUIRE(result == 1);
            }
            SECTION("tuple instance")
            {
                typedef internal::variadic_visitor_helper2<float, const char*, int> vh_type;

                tuple<float, const char*, int> t(1.2, "hello", 7);

                int result = vh_type::visit_instance(test2_functor{}, tuple_getter_functor{}, t);

                REQUIRE(result == 1);
            }
            SECTION("variant instance")
            {
                typedef internal::variadic_visitor_helper2<monostate, int, float, const char*> vh_type;

                internal::variant<monostate, int, float, const char*> v;

                int result = vh_type::visit_instance(test2_functor{}, internal::variant_storage_getter_functor{}, v);

                REQUIRE(result == 3);
            }
        }
    }
}

#include "macro/pop.h"
