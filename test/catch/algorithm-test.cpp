#include <catch.hpp>

#include <estd/array.h>
#include <estd/algorithm.h>

struct test_class_1
{
    estd::array<int, 10> values;
    int searching_for = 5;

    bool predicate(int candidate)
    {
        return candidate == searching_for;
    }
};


TEST_CASE("algorithm tests")
{
    test_class_1 tc1;

    SECTION("find_if in class")
    {
        // alas, can't do this.  closures are kinda mandatory it seems...
        //estd::find_if(tc1.values.begin(), tc.values.end(), tc1.predicate);
    }
}