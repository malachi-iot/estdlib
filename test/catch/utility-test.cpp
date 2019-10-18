#include <catch.hpp>

#include <estd/utility.h>
#include <estd/internal/utility.h>
#include <estd/type_traits.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

ESTD_FN_HAS_METHOD(void, test_fn1,)
ESTD_FN_HAS_METHOD(void, test_fn2,)
ESTD_FN_HAS_METHOD(void, test_fn3,)
ESTD_FN_HAS_METHOD(void, test_fn4,)

struct test_class_1
{
    void test_fn1() {}

    typedef void tag1;
    typedef void test_tag;
};

ESTD_FN_HAS_TYPEDEF_EXP(tag1);
ESTD_FN_HAS_TAG_EXP(test);

struct test_class_2 : test_class_1
{
    void test_fn2() {}
};

template <class TRetVal = void>
struct test_class_3 : test_class_2
{
    TRetVal test_fn3() const;
};

struct test_class_4 : test_class_3<>
{
    void test_fn4() {}
};


class test_class_5 : protected test_class_4
{
    void test_fn5() {}

public:
    ESTD_FN_HAS_PROTECTED_METHOD_EXP(void, test_fn1,)
    ESTD_FN_HAS_PROTECTED_METHOD_EXP(void, test_fn5,)
    ESTD_FN_HAS_PROTECTED_METHOD_EXP(void, test_fn6,)
};

using namespace estd::internal;


template <typename U, U u> struct reallyHas2;


template <class T>
void swap_numeric_test(const T orig_swap1 = 77, const T orig_swap2 = 111)
{
    T swap1 = orig_swap1, swap2 = orig_swap2;

    estd::swap(swap1, swap2);

    REQUIRE(swap1 == orig_swap2);
    REQUIRE(swap2 == orig_swap1);
}

TEST_CASE("utility")
{
    typedef void (test_class_1::* fn_1_type)(void);
    typedef void (test_class_2::* fn_2_type)(void);

    SECTION("MethodInfo")
    {
        typedef MethodInfo<test_class_2> mi1;

        auto f = &test_class_2::test_fn1;

        typedef MethodInfo<decltype(f)> mi2;
        typedef MethodInfo<decltype(&test_class_2::test_fn1)> mi3;

        typedef mi2::ClassType class_type;

        REQUIRE(estd::is_same<class_type, test_class_1>::value);
    }
    SECTION("has_member_base")
    {
        typedef has_member_base::reallyHas<fn_1_type, &test_class_2::test_fn1> has_1;
        // error: non-type template argument of type 'void (test_class_1::*)()' cannot
        // be converted to a value of type 'void (test_class_2::*)()'
        //
        // because test_class_2::test_fn1 actually yields pointer of type test_class_1::test_fn1
        //typedef has_member_base::reallyHas<fn_2_type, &test_class_2::test_fn1> has_1;
        //typedef reallyHas2<fn_2_type, &test_class_2::test_fn1> has_1;
        typedef has_member_base::reallyHas<fn_2_type, &test_class_2::test_fn2> has_2;
    }
    SECTION("class 1")
    {
        REQUIRE(has_test_fn1_method<test_class_1>::value);
    }
    SECTION("class 2")
    {
        REQUIRE(has_test_fn1_method<test_class_2>::value);

        test_class_1 tc1;
        test_class_2 tc2;

        fn_2_type f1 = &test_class_2::test_fn2;
        fn_2_type f2 = &test_class_2::test_fn1;

        REQUIRE(has_test_fn2_method<test_class_2>::value);
    }
    SECTION("class 3")
    {
        REQUIRE(has_test_fn1_method<test_class_3<> >::value);
        REQUIRE(has_test_fn2_method<test_class_3<> >::value);
        REQUIRE(has_test_fn3_method<test_class_3<> >::value);

        // because method signature matching includes return value
        REQUIRE(!has_test_fn3_method<test_class_3<int> >::value);
    }
    SECTION("class 4")
    {
        REQUIRE(has_test_fn1_method<test_class_4>::value);
        REQUIRE(has_test_fn2_method<test_class_4>::value);
        REQUIRE(has_test_fn3_method<test_class_4>::value);
        REQUIRE(has_test_fn4_method<test_class_4>::value);
    }
    SECTION("class 5")
    {
        REQUIRE(test_class_5::has_test_fn1_method<test_class_5>::value);
        REQUIRE(test_class_5::has_test_fn5_method<test_class_5>::value);
        REQUIRE(!test_class_5::has_test_fn6_method<test_class_5>::value);
        /*
         * None of these work because ESTD_FN_HAS_METHOD doesn't detect
         * non-public members
        REQUIRE(has_test_fn1_method<test_class_5>::value);
        REQUIRE(has_test_fn2_method<test_class_5>::value);
        REQUIRE(has_test_fn3_method<test_class_5>::value);
        REQUIRE(has_test_fn4_method<test_class_5>::value);
         */
    }
    SECTION("has_typedef testing")
    {
        REQUIRE(has_tag1_typedef<test_class_1>::value);
        REQUIRE(has_tag1_typedef<test_class_2>::value);
        REQUIRE(has_tag1_typedef<test_class_4>::value);

        REQUIRE(has_test_tag<test_class_1>::value);
        REQUIRE(has_test_tag<test_class_2>::value);
        REQUIRE(has_test_tag<test_class_4>::value);
    }
    SECTION("swap")
    {
        SECTION("char")
        {
            char swap1 = 'A', swap2 = 'B';

            estd::swap(swap1, swap2);

            REQUIRE(swap1 == 'B');
            REQUIRE(swap2 == 'A');
        }
        SECTION("short")
        {
            swap_numeric_test<short>();
        }
        SECTION("int")
        {
            swap_numeric_test<short>();
        }
        SECTION("long")
        {
            swap_numeric_test<int>();
        }
        SECTION("long long")
        {
            swap_numeric_test<long>();
        }
        SECTION("uint32_t")
        {
            swap_numeric_test<uint32_t>();
        }
    }
}

#pragma GCC diagnostic pop
