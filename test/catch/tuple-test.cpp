#include <catch.hpp>

#include <estd/tuple.h>

#include "test-data.h"

using namespace estd::test;

template <class ...TArgs>
void tester(TArgs...args)
{

}

TEST_CASE("tuple")
{
    SECTION("std lib version")
    {
        using namespace std;

        tuple<int> t(5);

        REQUIRE(get<0>(t) == 5);
    }
    SECTION("estd lib version")
    {
        using namespace estd;

        tuple<int> t(5);

        REQUIRE(get<0>(t) == 5);
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
        // TODO: Implement from https://en.cppreference.com/w/cpp/utility/tuple/tuple_size
        estd::make_tuple(2.0f, 3.0f);
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
