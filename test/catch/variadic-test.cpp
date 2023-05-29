#include <catch.hpp>

#include <estd/internal/variadic.h>
#include <estd/variant.h>

#include "test-data.h"

#include "macro/push.h"

using namespace estd;

// variadic utils are inteneral and do not directly match up to 'std'

struct identify_index_functor
{
    template <unsigned I, class T>
    bool operator()(internal::visitor_index<I, T>, int&& param)
    {
        return param == I;
    }
};

// NOTE: Works well enough we might put it out into tuple area
struct tuple_getter_functor
{
    template <unsigned I, class T, class ...TArgs>
    T& operator()(internal::visitor_index<I, T>, tuple<TArgs...>& t)
    {
        return get<I>(t);
    }
};


// NOTE: This is just for testing.  To do this particular operation
// "in real life" it's better to use direct struct/variadic navigation
// since that will definitely compile-time resolve
template <class T>
struct identify_type_functor
{
    template <unsigned I>
    constexpr bool operator()(in_place_index_t<I>, T* = nullptr) const { return false; }

    template <unsigned I>
    bool operator()(internal::visitor_instance<I, T> v, T* output = nullptr) const
    {
        // output = const char**
        if(output)  *output = v.value;
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

                int result = vh_type::visit(identify_index_functor{}, 1);

                REQUIRE(result == 1);
            }
            SECTION("tuple instance")
            {
                typedef internal::variadic_visitor_helper2<float, const char*, int> vh_type;

                tuple<float, const char*, int> t(1.2, &test::str_hello[0], 7);

                int result = vh_type::visit_instance(identify_type_functor<const char*>{}, tuple_getter_functor{}, t);

                REQUIRE(result == 1);
            }
            SECTION("variant instance")
            {
                // TODO: Do flavor of this calling direct 'visit_instance' on
                // variant itself and consider making a freestanding one
                // which takes variant as an input (so as to be slightly
                // more std-like)

                typedef internal::variadic_visitor_helper2<monostate, int, float, const char*> vh_type;

                internal::variant<monostate, int, float, const char*> v;
                const char* output = nullptr;

                // FIX: 'output' never truly passed into identify_type_functor
                int result = vh_type::visit_instance(
                    identify_type_functor<const char*>{},
                    internal::variant_storage_getter_functor{}, v,
                    &output);

                REQUIRE(result == 3);
                REQUIRE(output != nullptr);
                //REQUIRE(v.raw() == (byte*)output);
            }
        }
    }
}

#include "macro/pop.h"
