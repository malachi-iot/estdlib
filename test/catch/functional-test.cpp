#include <catch.hpp>

#include <estd/functional.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

static const char* got_something = NULLPTR;

int do_something(const char* msg)
{
    got_something = msg;
    //printf("I did something: %s\n", msg);

    return -1;
}

using namespace estd;

// temporarily switching this on or off here as I build out experimental pre C++03 tuple
#ifdef FEATURE_CPP_VARIADIC


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

TEST_CASE("functional")
{
    constexpr auto z = estd::make_tuple(1, 2);

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

        int sz = sizeof(tuple);

        REQUIRE(sz == sizeof(long) * 2 + sizeof(double));

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
    SECTION("function")
    {
        //estd::function<int()> f = []() { return 5; };

        SECTION("experimental")
        {
            SECTION("simplest lambda")
            {
                estd::experimental::function<int()> f = []() { return 5; };

                int val = f();

                REQUIRE(val == 5);
            }
            SECTION("ref capture lambda")
            {
                int val2 = 5;
                int val3 = 0;

                estd::experimental::function<int(int)> f = [&val2](int x) { return val2++; };

                REQUIRE(f(0) == 5);
                f = [&](int x) { ++val3; return val2++; };
                REQUIRE(f(1) == 6);
                REQUIRE(val2 == 7);
                REQUIRE(val3 == 1);
            }
            SECTION("make_inline")
            {
                auto i = estd::experimental::function<int(int)>::make_inline([](int x) { return x + 1; });

                REQUIRE(i.exec(1) == 2);

                estd::experimental::function_base<int(int)> f(&i);

                REQUIRE(f.operator()(1) == 2);

                int outside_scope_value = 5;
                auto l = [&](int x) { return x + outside_scope_value; };

                estd::experimental::inline_function<decltype(l), int(int)> _if(std::move(l));
                /*
                auto _if2 =
                    estd::experimental::make_inline_function<decltype(l), int(int)>(std::move(l)); */

                REQUIRE(_if(5) == 10);

                auto _if3 = estd::experimental::function<int(int)>::make_inline2(
                    [&](int x) { return x * outside_scope_value; });

                REQUIRE(_if3(5) == 25);

                estd::experimental::function_base<int(int)> fb1(_if3);

                REQUIRE(fb1(5) == 25);
            }
        }
    }
    SECTION("bind")
    {
        auto b = estd::bind(do_something, "hello");

        int sz = sizeof(b);

        REQUIRE(b() == -1);

        REQUIRE(got_something == "hello");
    }
}

#endif

#pragma GCC diagnostic pop
