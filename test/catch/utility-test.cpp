#include <catch.hpp>

#include <estd/utility.h>
#include <estd/internal/utility.h>

ESTD_FN_HAS_METHOD(void, test_fn1,)
ESTD_FN_HAS_METHOD(void, test_fn2,)
ESTD_FN_HAS_METHOD(void, test_fn3,)
ESTD_FN_HAS_METHOD(void, test_fn4,)

struct test_class_1
{
    void test_fn1() {}
};

struct test_class_2
{
    void test_fn2() {}
};

template <class TRetVal = void>
struct test_class_3 : test_class_2
{
    TRetVal test_fn3() {}
};

struct test_class_4 : test_class_3<>
{
    void test_fn4() {}
};


TEST_CASE("utility")
{
    SECTION("class 1")
    {
        REQUIRE(has_test_fn1_method<test_class_1>::value);
    }
    /*
     * These are all failing, indicating that ESTD_FN_HAS_METHOD isn't walking
     * up hierarchy chain
    SECTION("class 2")
    {
        REQUIRE(has_test_fn1_method<test_class_2>::value);
        REQUIRE(has_test_fn2_method<test_class_2>::value);
    }
    SECTION("class 3")
    {
        REQUIRE(has_test_fn1_method<test_class_3<> >::value);
        REQUIRE(has_test_fn2_method<test_class_3<> >::value);
        REQUIRE(has_test_fn3_method<test_class_3<> >::value);
    }
    SECTION("class 4")
    {
        REQUIRE(has_test_fn1_method<test_class_4>::value);
        REQUIRE(has_test_fn2_method<test_class_4>::value);
        REQUIRE(has_test_fn3_method<test_class_4>::value);
        REQUIRE(has_test_fn4_method<test_class_4>::value);
    } */
}