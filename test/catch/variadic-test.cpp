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
    constexpr bool operator()(variadic::type<I, T>, int&& param) const
    {
        return param == I;
    }
};

struct identify_value_functor
{
    template <size_t I, class T, T v>
    constexpr bool operator()(variadic::value<I, T, v>, T&& param) const
    {
        return param == v;
    }
};

// NOTE: Works well enough we might put it out into tuple area
struct tuple_getter_functor
{
    template <size_t I, class T, class ...TArgs>
    T& operator()(variadic::type<I, T>, tuple<TArgs...>& t)
    {
        return get<I>(t);
    }
};

// Handling 'instance' mechanism right within functor
struct tuple_getter_functor2
{
    // DEBT: Ensure this in fact is a tuple type, or perhaps there's a clever
    // way to guarantee that a 'get' is present and operates as expected
    template <size_t I, class T, class TTuple, class F, class ...TArgs>
    bool operator()(variadic::type<I, T>, TTuple& t, F&& f, TArgs&&...args)
    {
        // DEBT: visitor_instance erodes down to a visitor_index.  Works fine,
        // but CLang-Tidy complains 8 bytes are wasted (which they are).  However,
        // I expected optimizer and general compiler rules to be A-OK with that
        return f(
            variadic::v1::visitor_instance<I, T>{get<I>(t)},
            std::forward<TArgs>(args)...);
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
    bool operator()(variadic::instance<I, T> v, T* output) const
    {
        // output = const char**
        if(output)  *output = v.value;
        return true;
    }
};


struct synthetic_projector
{
    template <class T, size_t>
    using evaluator = variadic::projected_result<double, is_same<T, float>::value>;
};

template <class T, class Tag, class Args, class enabled = void>
struct has_method_group : bool_constant<false> {};

template <class T, class Tag, class ...Args>
struct method_invoke_assist;

//decltype(std::declval<TObserver>().on_notify(e)

struct hello_tag {};

template <class T>
struct method_invoke_assist<T, hello_tag>
{
    void operator()(T& t) const
    {
        t.hello();
    }
};

template <class... Args> struct fake : bool_constant<true> {};

template <class T>
struct has_method_group<T, hello_tag, fake<>, decltype(T().hello())> : bool_constant<true> {};

template <class T, class Arg1>
struct has_method_group<T, hello_tag, fake<Arg1>, decltype(T().hello(std::declval<Arg1>()))> : bool_constant<true> {};

template <class T>
struct has_method_group<T, hello_tag, fake<>, decltype(T().hello2())> : bool_constant<true> {};

struct Invoker1 {};

struct Invoker2
{
    void hello() {}
    void hello(int param) {}
};

struct Invoker3
{
    static void hello() {}
};

class Invoker4
{
    void hello() {}
};


template <class Tag, class ...Args>
struct has_method_selector
{
    template <class T, size_t>
    using evaluator = has_method_group<T, Tag, fake<Args...>>;
};



TEST_CASE("variadic")
{
    SECTION("types")
    {
        typedef variadic::types<monostate, float, char*> types;

        REQUIRE(is_same<monostate, types::get<0>>::value);
        REQUIRE(is_same<char*, types::get<2>>::value);

        SECTION("prepend")
        {
            using prepended = types::prepend<void>;

            REQUIRE(prepended::size() == 4);
            REQUIRE(is_same<float, prepended::get<2>>::value);
        }
    }
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
        SECTION("constructible")
        {
            using _selector = internal::constructible_selector<const char*>;
            using selector = variadic::selector<_selector, int, const char*>;

            constexpr int idx = selector::first::index;

            REQUIRE(idx == 1);
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
            typedef variadic::type_visitor<monostate, int, float, const char*> vh_type;

            int result = vh_type::visit(identify_index_functor{}, 1);

            REQUIRE(result == 1);
        }
        SECTION("tuple instance")
        {
            typedef variadic::type_visitor<float, const char*, int> vh_type;

            tuple<float, const char*, int> t(1.2, &test::str_hello[0], 7);
            const char* output = nullptr;

            int result = vh_type::visit_instance(identify_type_functor<const char*>{},
                tuple_getter_functor{}, t,
                &output);

            REQUIRE(result == 1);
            REQUIRE(output == test::str_hello);

            result = vh_type::visit(
                tuple_getter_functor2{}, t,
                identify_type_functor<const char*>{}, &output);

            REQUIRE(result == 1);
            REQUIRE(output == test::str_hello);
        }
        SECTION("variant instance")
        {
            // TODO: Do flavor of this calling direct 'visit_instance' on
            // variant itself and consider making a freestanding one
            // which takes variant as an input (so as to be slightly
            // more std-like)

            typedef variadic::type_visitor<monostate, int, float, const char*> vh_type;

            variant<monostate, int, float, const char*> v;

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
                std::size_t result = v.visit(
                    identify_type_functor<const char*>{},
                    &output);

                REQUIRE(result == 3);
                REQUIRE(output == test::str_hello);
            }

            v = 0;

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
        typedef variadic::selector<internal::constructible_selector<int>, test::NonTrivial, const char*, monostate> vhs_type3;
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
    SECTION("values")
    {
        typedef variadic::values<int, 0, 7, 77, 777> values;

        SECTION("visit")
        {
            int index = values::visit(identify_value_functor{}, 77);

            REQUIRE(index == 2);

            // Nifty trick - since integral_constant has a value_type conversion
            // operator, we can directly specify 'int' for v
            index = values::visit([](int v, int compare_to)
              { return v == compare_to; }, 777);

            REQUIRE(index == 3);
        }
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
        SECTION("not integer: nullptr")
        {
            // NOTE: It seems floats and doubles are not supported by compilers in this case
            typedef integer_sequence<void*, nullptr> type;
        }
    }
    SECTION("projector")
    {
        size_t v;

        SECTION("basic")
        {
            typedef variadic::projector<internal::is_same_projector<int>, int, int, int> type;

            REQUIRE(type::size() == 3);
            REQUIRE(type::get<0>::value);

            REQUIRE(internal::conjunction<type>::value);
        }
        SECTION("test2")
        {
            typedef variadic::type_visitor<int, float, monostate, int> type;

            REQUIRE(type::select<internal::is_same_selector<int>>::indices::size() == 2);
            REQUIRE(type::select<internal::is_same_projector<float>>::all == true);

            typedef type::select<synthetic_projector>::selected projected;

            v = projected::single::index;

            REQUIRE(v == 1);
            REQUIRE(is_same<projected::single::type, double>::value);
        }
    }
    SECTION("experimental")
    {
#if __cpp_template_auto
        static constexpr const char* c1 = "hello";
        using v = variadic::experimental::v<0, *c1>;
        using values = variadic::experimental::v2<0, 4, 7>;

        REQUIRE(values::size() == 3);
        REQUIRE(values::get<1>() == 4);
#endif

        SECTION("invoker")
        {
            using types = variadic::types<Invoker1, Invoker2, Invoker3, Invoker4>;
            using selected = types::selector<has_method_selector<hello_tag> >;
            using selected2 = types::selector<has_method_selector<hello_tag, int> >;

            REQUIRE(selected::size() == 2);
            REQUIRE(selected2::size() == 1);
        }
    }
}

#include "macro/pop.h"
