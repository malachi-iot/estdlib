#include <catch2/catch.hpp>

#include <estd/utility.h>
#include <estd/internal/utility.h>
#include <estd/type_traits.h>

#include "macro/push.h"

// DEBT: IIRC, 'HAS_METHOD' is now deprecated
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


class test_impl_base
{
protected:
    void protected_fn1() {}
    int protected_fn2() { return 0; }
};

template <class TBase = test_impl_base>
class test_impl : TBase
{
    typedef TBase base_type;
    typedef test_impl<TBase> this_type;

protected:
    ESTD_FN_HAS_PROTECTED_METHOD_EXP(void, protected_fn1,)
    ESTD_FN_HAS_METHOD(int, protected_fn2,)

public:
    void require_fn1()
    {
        REQUIRE(has_protected_fn1_method<test_impl<>>::value);
    }

    void require_fn2()
    {
        REQUIRE(has_protected_fn2_method<this_type>::value);
    }
};

template <class T>
struct test_template_base
{
    typedef T value_type;

protected:
    T value() const { return T(); }
    void value(T v) {}
};


struct test_overload
{
    typedef int value_type;

    // Enabling this overload kills has_value_method/has_value_method_2 presumably
    // because it greedily notices that the overload DOES NOT match the requested signature
    //value_type value() const { return value_type(); }
    void value(value_type v) {}

    ESTD_FN_HAS_METHOD(void, value, value_type);

    template <class T> struct has_value_method_2 : estd::internal::has_member_base
    {
        ESTD_FN_HAS_METHOD_EXP(void, value, value_type);

        static CONSTEXPR int sz = sizeof(test_has_value_<T>(nullptr));
        static CONSTEXPR bool value = sizeof(test_has_value_<T>(nullptr)) == sizeof(yes);
    };
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

        int sz_no = sizeof(estd::internal::has_member_base::no);
        int sz_yes = sizeof(estd::internal::has_member_base::yes);

        // NOTE: Odd side effects caused sz_yes to become 2 sometimes
        REQUIRE(sz_no == 2);
        REQUIRE(sz_yes == 1);
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
    SECTION("protected 1")
    {
        test_impl<> test;

        test.require_fn1();
        test.require_fn2();
    }
    SECTION("overload")
    {
        typedef test_overload t;

        //REQUIRE(t::has_value_method<t>::value);

        int sz = t::has_value_method_2<t>::sz;
        //REQUIRE(sz == 2);
        REQUIRE(t::has_value_method_2<t>::value);
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
    SECTION("pair")
    {
        using pair_type = estd::pair<float, float>;
        const pair_type p(2.0, 3.0);

        REQUIRE(estd::get<0>(p) == 2.0);

        SECTION("operator ==")
        {
            REQUIRE(pair_type(2, 3) == p);
        }
        SECTION("operator !=")
        {
            REQUIRE(pair_type(2.1, 3) != p);
        }
        SECTION("operator <")
        {
            REQUIRE(pair_type(1.9, 3) < p);
            REQUIRE(pair_type(2, 2.9) < p);
            REQUIRE((pair_type(2.1, 2.9) < p) == false);
        }
    }
    SECTION("integer_sequence")
    {
        SECTION("normal")
        {
            using seq = estd::make_index_sequence<4>;

            REQUIRE(seq::size() == 4);
            REQUIRE(seq::get<0>::value == 0);
            REQUIRE(seq::get<3>::value == 3);
        }
        SECTION("reversed")
        {
            using seq = estd::make_reverse_index_sequence<4>;

            REQUIRE(seq::size() == 4);
            REQUIRE(seq::get<0>::value == 3);
            REQUIRE(seq::get<1>::value == 2);
            REQUIRE(seq::get<2>::value == 1);
            REQUIRE(seq::get<3>::value == 0);
        }
    }
}

// DEBT: Put this test elsewhere
#define FEATURE_ESTD_STD_ALIAS 1

#include <estd/internal/alias/cstdint.h>
#include <estd/internal/alias/string.h>
#include <estd/internal/alias/type_traits.h>


#include "macro/pop.h"
