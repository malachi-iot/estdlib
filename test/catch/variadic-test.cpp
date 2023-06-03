#include <catch.hpp>

#include <estd/internal/variadic.h>
#include <estd/variant.h>

#include "test-data.h"

#include "macro/push.h"

using namespace estd;

// variadic utils are our own, do not directly match up to 'std'

struct identify_index_functor
{
    template <size_t I, class T>
    bool operator()(variadic::visitor_index<I, T>, int&& param)
    {
        return param == I;
    }
};

// NOTE: Works well enough we might put it out into tuple area
struct tuple_getter_functor
{
    template <size_t I, class T, class ...TArgs>
    T& operator()(variadic::visitor_index<I, T>, tuple<TArgs...>& t)
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
    template <size_t I>
    constexpr bool operator()(in_place_index_t<I>, T*) const { return false; }

    template <size_t I>
    bool operator()(variadic::visitor_instance<I, T> v, T* output) const
    {
        // output = const char**
        if(output)  *output = v.value;
        return true;
    }
};



TEST_CASE("variadic")
{
    SECTION("select_type")
    {
        SECTION("basic")
        {
            typedef estd::internal::select_type<int, estd::monostate, int, float> iot;

            constexpr int idx = iot::first::index;
            constexpr bool multiple = iot::size() > 1;

            REQUIRE(idx == 1);
            REQUIRE(multiple == false);
        }
        SECTION("not found")
        {
            typedef estd::internal::select_type<int, estd::monostate, float> iot;

            constexpr bool empty = iot::empty();
            constexpr bool multiple = iot::size() > 1;

            REQUIRE(empty);
            REQUIRE(multiple == false);
        }
        SECTION("multiple")
        {
            typedef estd::internal::select_type<int, int, int, int> iot;

            unsigned idx = iot::first::index;
            constexpr bool multiple = iot::size() > 1;

            REQUIRE(idx == 0);
            REQUIRE(multiple == true);

            typedef iot selected;

            unsigned v = selected::size();

            REQUIRE(v == 3);

            v = selected::get<1>::index;

            REQUIRE(v == 1);

            bool match = is_same<selected::get<1>::type, int>::value;

            REQUIRE(match);
        }
    }
    SECTION("visitor")
    {
        SECTION("static")
        {
            typedef variadic::visitor<monostate, int, float, const char*> vh_type;

            int result = vh_type::visit(identify_index_functor{}, 1);

            REQUIRE(result == 1);
        }
        SECTION("tuple instance")
        {
            typedef variadic::visitor<float, const char*, int> vh_type;

            tuple<float, const char*, int> t(1.2, &test::str_hello[0], 7);
            const char* output = nullptr;

            int result = vh_type::visit_instance(identify_type_functor<const char*>{},
                tuple_getter_functor{}, t,
                &output);

            REQUIRE(result == 1);
            REQUIRE(output == test::str_hello);
        }
        SECTION("variant instance")
        {
            // TODO: Do flavor of this calling direct 'visit_instance' on
            // variant itself and consider making a freestanding one
            // which takes variant as an input (so as to be slightly
            // more std-like)

            typedef variadic::visitor<monostate, int, float, const char*> vh_type;

            internal::variant<monostate, int, float, const char*> v;

            v = (const char*)test::str_hello;

            const char* output = nullptr;

            SECTION("direct")
            {
                int result = vh_type::visit_instance(
                    identify_type_functor<const char*>{},
                    internal::variant_storage_getter_functor{}, v,
                    &output);

                REQUIRE(result == 3);
                REQUIRE(output == test::str_hello);
            }
            SECTION("helper")
            {
                std::size_t result = -2;

                v.visit_instance(
                    identify_type_functor<const char*>{},
                    &result,
                    &output);

                REQUIRE(result == 3);
                REQUIRE(output == test::str_hello);
            }

            // NOTE: *might* go through emplace chain, we'll see
            v = test::str_simple;

            REQUIRE(get<const char*>(v) == test::str_simple);
        }
    }
    SECTION("visitor struct")
    {
        typedef variadic::selector<internal::converting_selector<int>, int, float, monostate> vhs_type;
        int selected = vhs_type::first::index;

        REQUIRE(selected == 0);

        typedef variadic::selector<internal::converting_selector<char[128]>, int, const char*, monostate> vhs_type2;
        selected = vhs_type2::first::index;

        REQUIRE(selected == 1);

        //typedef internal::visitor_helper_struct<internal::converting_selector<int>, test::NonTrivial, const char*, monostate> vhs_type3;
        typedef variadic::selector<internal::constructable_selector<int>, test::NonTrivial, const char*, monostate> vhs_type3;
        selected = vhs_type3::first::index;

        REQUIRE(selected == 0);

        typedef variadic::selector<internal::index_selector<1>, int, float, monostate> vhs_type4;
        selected = vhs_type4::first::index;
        bool v = is_same<vhs_type4::first::type, float>::value;

        REQUIRE(selected == 1);
        REQUIRE(v);
    }
    SECTION("variadic_first")
    {
        typedef variadic::first<float, int, monostate>::type first_type;
        bool v = is_same<float, first_type>::value;

        REQUIRE(v);
    }
    // TODO: Move this out to 'utility' test area
    SECTION("integer_sequence")
    {
        typedef integer_sequence<int, 0, 7, 77, 777> i_type;
        int value;

        SECTION("get_index")
        {
            value = internal::get_index<0, i_type>::value;

            REQUIRE(value == 0);

            value = internal::get_index<3, i_type>::value;

            REQUIRE(value == 777);

            value = i_type::get<2>::value;

            REQUIRE(value == 77);

            value = i_type::size();

            REQUIRE(value == 4);
        }
        SECTION("prepend")
        {
            typedef i_type::prepend<-7> i2_type;

            value = i2_type::get<0>::value;

            REQUIRE(value == -7);

            value = i2_type::size();

            REQUIRE(value == 5);
        }
        SECTION("append")
        {
            typedef i_type::append<7777> i2_type;

            value = i2_type::get<4>::value;

            REQUIRE(value == 7777);

            value = i2_type::size();

            REQUIRE(value == 5);
        }
        SECTION("reverse")
        {
            typedef internal::indices_reverser<777, 77, 7, 0> ir_type;
            //typedef ir_type::reversed i_type;

            //int value = internal::get_index<0, i_type>::value;

            //REQUIRE(value == 0);
        }
    }
}

#include "macro/pop.h"
