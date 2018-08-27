#include <catch.hpp>

#include <estd/functional.h>


static const char* got_something = NULLPTR;

int do_something(const char* msg)
{
    got_something = msg;
    //printf("I did something: %s\n", msg);

    return -1;
}

using namespace estd;


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
    }
    SECTION("bind")
    {
        auto b = estd::bind(do_something, "hello");

        int sz = sizeof(b);

        REQUIRE(b() == -1);

        REQUIRE(got_something == "hello");
    }
}
