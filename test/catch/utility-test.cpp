#include <catch.hpp>

#include <estd/utility.h>
#include <estd/internal/utility.h>
#include <estd/type_traits.h>

ESTD_FN_HAS_METHOD(void, test_fn1,)
ESTD_FN_HAS_METHOD(void, test_fn2,)
ESTD_FN_HAS_METHOD(void, test_fn3,)
ESTD_FN_HAS_METHOD(void, test_fn4,)

struct test_class_1
{
    void test_fn1() {}
};

struct test_class_2 : test_class_1
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

using namespace estd::internal;

// https://stackoverflow.com/questions/42175294/how-get-the-class-object-type-from-pointer-to-method
template<class T>
struct MethodInfo;

#ifdef FEATURE_CPP_VARIADIC
template<class C, class R, class... A>
struct MethodInfo<R(C::*)(A...)> //method pointer
{
    typedef C ClassType;
    typedef R ReturnType;
    typedef std::tuple<A...> ArgsTuple;
};

template<class C, class R, class... A>
struct MethodInfo<R(C::*)(A...) const> : MethodInfo<R(C::*)(A...)> {}; //const method pointer
#endif

template <typename U, U u> struct reallyHas2;

TEST_CASE("utility")
{
    typedef void (test_class_1::* fn_1_type)(void);
    typedef void (test_class_2::* fn_2_type)(void);

    SECTION("MethodInfo")
    {
        typedef MethodInfo<test_class_2> mi1;

        auto f = &test_class_2::test_fn1;

        typedef MethodInfo<decltype(f)> mi2;

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
        //REQUIRE(has_test_fn1_method<test_class_2>::value);

        test_class_1 tc1;
        test_class_2 tc2;

        fn_2_type f1 = &test_class_2::test_fn2;
        fn_2_type f2 = &test_class_2::test_fn1;

        REQUIRE(has_test_fn2_method<test_class_2>::value);
    }
    /*
     * These are all failing, indicating that ESTD_FN_HAS_METHOD isn't walking
     * up hierarchy chain
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
