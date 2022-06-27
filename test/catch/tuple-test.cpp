#include <catch.hpp>

#include <estd/tuple.h>

#include "test-data.h"

using namespace estd;
using namespace estd::test;

template <class ...TArgs>
void tester(TArgs...args)
{

}


template <class Tuple, size_t... Is>
constexpr auto take_front_impl(const Tuple& t,
    index_sequence<Is...>) ->
decltype (make_tuple(get<Is>(t)...))
{
    return make_tuple(get<Is>(t)...);
}

template <size_t N, class Tuple>
constexpr auto take_front(const Tuple& t) ->
decltype (take_front_impl(t, make_index_sequence<N>{}))
{
    return take_front_impl(t, make_index_sequence<N>{});
}



TEST_CASE("tuple")
{
    constexpr auto z = estd::make_tuple(1, 2);

    SECTION("std lib version")
    {
        std::tuple<int> t(5);

        REQUIRE(get<0>(t) == 5);
    }
    SECTION("estd lib version")
    {
        using namespace estd;

        tuple<int> t(5);

        REQUIRE(get<0>(t) == 5);
    }
    SECTION("minimal tuple")
    {
        auto tuple = estd::make_tuple(false, true);

        int sz = sizeof(tuple);

        REQUIRE(sz == 2);

        auto val1 = estd::get<0>(tuple);

        REQUIRE(val1 == false);
    }
    SECTION("tuple")
    {
        auto tuple = estd::make_tuple(0, 1.0, 2);

        // Size checks are very platform/toolchain dependent
#ifdef ESTD_OS_UNIX
        int sz = sizeof(tuple);

        REQUIRE(sz == sizeof(long) * 2 + sizeof(double));
#endif

        auto val1 = estd::get<0>(tuple);

        REQUIRE(val1 == 0);

        auto val2 = estd::get<1>(tuple);

        REQUIRE(val2 == 1.0);

        auto val3 = estd::get<2>(tuple);

        REQUIRE(val3 == 2);

        typedef estd::tuple_size<decltype (tuple)> test_type;
        int val = test_type::value;
        REQUIRE(val == 3);
        // Following doesn't work, catch must try to get the address of value
        //REQUIRE(estd::tuple_size<decltype (tuple)>::value == 3);

        SECTION("take front")
        {
            auto tuple2 = take_front<2>(tuple);
            val = estd::tuple_size<decltype (tuple2)>::value;

            REQUIRE(val == 2);

            REQUIRE(get<1>(tuple2) == 1.0);

            // Compile time error, as it should be:
            //estd::get<2>(tuple2);
        }

        typedef estd::tuple_element<0, decltype (tuple)>::type te;

        bool is_same = estd::is_same<te, int>::value;

        REQUIRE(is_same);

        is_same = estd::is_same<estd::tuple_element_t<1, decltype (tuple)>, double>::value;

        REQUIRE(is_same);

        is_same = estd::is_same<estd::tuple_element_t<2, decltype (tuple)>, int>::value;

        REQUIRE(is_same);

        WHEN("reference tests")
        {
            int& v1 = get<0>(tuple);

            REQUIRE(v1 == val1);

            double& v2 = get<1>(tuple);

            REQUIRE(v2 == val2);
        }
        WHEN("more reference tests")
        {
            int int_value = 7;
            estd::tuple<int&> tuple3(int_value);

            int& int_ref = get<0>(tuple3);

            REQUIRE(int_ref == 7);

            int_ref = 5;

            REQUIRE(int_value == 5);
        }
    }
    SECTION("value initialization")
    {
        using namespace estd;

        // As per https://en.cppreference.com/w/cpp/utility/tuple/tuple, tuples
        // value-initialize, which means default constructors shall run

        tuple<DefaultConstructor, int> t;

        const DefaultConstructor& dc = get<0>(t);

        REQUIRE(dc.val == DefaultConstructor::default_value());
    }
    SECTION("tuple size")
    {
        using namespace estd;

        SECTION("simple type")
        {
            tuple<bool, bool, bool> t(true, false, true);

            int sz = sizeof(t);

            REQUIRE(sz == 3);
        }
        SECTION("complex empty type")
        {
            tuple<EmptyClass, EmptyClass, EmptyClass> t;

            int sz = sizeof(t);

            REQUIRE(sz == 3);
        }
        SECTION("intermixed types")
        {
            tuple<EmptyClass, Dummy, EmptyClass> t;

            int sz = sizeof(t);

            // on debian x64, this comes out to 32
            //REQUIRE(sz == 3);

            // TODO: Look into tuple<T&, TArgs...> see if we accidently light that up
        }
    }
    SECTION("tuple_size")
    {
        SECTION("tuple")
        {
            // TODO: Implement from https://en.cppreference.com/w/cpp/utility/tuple/tuple_size
            auto t = estd::make_tuple(2.0f, 3.0f);
            constexpr int sz = estd::tuple_size<decltype(t)>::value;
            REQUIRE(sz == 2);
        }
        SECTION("pair")
        {
            auto p = estd::make_pair(2.0f, 3.0f);
            //constexpr int sz = estd::tuple_size<decltype(p)>::value;
            //REQUIRE(sz == 2);
        }
    }
    SECTION("apply")
    {
        // From https://en.cppreference.com/w/cpp/utility/apply

        SECTION("basic")
        {
            auto add_lambda = [](int first, int second) { return first + second; };

            // FIX: Doesn't work yet
            //estd::apply(add_lambda, estd::make_pair(2.0f, 3.0f));
        }
        SECTION("parameter pack")
        {
            auto t = estd::make_tuple(2.0f, 3.0f);

            // template argument deduction/substitution failed:
            // couldn’t deduce template parameter ‘F2’
            //estd::apply(tester, estd::make_tuple(2.0f, 3.0f));
        }
    }
}
