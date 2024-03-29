#include <catch.hpp>

#define FEATURE_ESTD_ALGORITHM_OPT 0

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

using namespace estd;

TEST_CASE("algorithm tests")
{
    test_class_1 tc1;

    SECTION("find_if in class")
    {
        // alas, can't do this.  closures are kinda mandatory it seems...
        //estd::find_if(tc1.values.begin(), tc.values.end(), tc1.predicate);
    }
    SECTION("copy_n")
    {
        char buf1[] = "hi2u";
        char buf2[4] = {0,0,0,0};

        estd::copy_n(buf1, 4, buf2);

        REQUIRE(estd::equal(buf1, &buf1[3], buf2));
    }
    SECTION("reverse")
    {
        char buf[] = "1234";

        estd::reverse(&buf[0], &buf[4]);

        REQUIRE(std::string(buf) == "4321");
    }
    SECTION("fill")
    {
        char buf[10];

        buf[9] = 0;

        estd::fill_n(buf, 10, 7);

        REQUIRE(buf[5] == 7);
        REQUIRE(buf[9] == 7);
    }
    SECTION("copy_backward")
    {
        array<unsigned, 10> a{ 0, 1, 2, 3, 4, 5 };

        copy_backward(a.begin() + 1, a.end() - 1, a.end());

        REQUIRE(a[0] == 0);
        REQUIRE(a[1] == 1);
        REQUIRE(a[2] == 1);
        REQUIRE(a[3] == 2);
    }
}
