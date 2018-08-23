#include <catch.hpp>

#include <estd/functional.h>


static const char* got_something = NULLPTR;

void do_something(const char* msg)
{
    got_something = msg;
    //printf("I did something: %s\n", msg);
}

using namespace estd;


template <class Tuple, size_t... Is>
constexpr auto take_front_impl(Tuple t,
                               index_sequence<Is...>) ->
decltype (make_tuple(get<Is>(t)...))
{
    return make_tuple(get<Is>(t)...);
}

template <size_t N, class Tuple>
constexpr auto take_front(Tuple t) ->
decltype (take_front_impl(t, make_index_sequence<N>{}))
{
    return take_front_impl(t, make_index_sequence<N>{});
}

TEST_CASE("functional")
{
    SECTION("tuple")
    {
        auto tuple = estd::make_tuple(0, 1.0, 2);

        int sz = sizeof(tuple);

        REQUIRE(sz == sizeof(long) * 2 + sizeof(double));

        auto val1 = estd::get<0>(tuple);

        REQUIRE(val1 == 0);

        auto val2 = estd::get<1>(tuple);

        REQUIRE(val2 == 1.0);

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
    }
    SECTION("A")
    {
        auto b = estd::bind(&do_something, "hello");

        int sz = sizeof(b);

        b();

        REQUIRE(got_something == "hello");
    }
}
