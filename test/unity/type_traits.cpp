#include <estd/type_traits.h>

#include "unit-test.h"

using namespace estd;

template <class T>
struct Specializable
{

};

// +++ EXPERIMENTAL
struct DerivedFrom : Specializable<int> {};

// specializing is_base_of is undefined, and if we try it causes a
// recursion
template <class Base, class Derived>
struct has_base : is_base_of<Base, Derived> {};

template <class T, class Derived>
struct has_base<Specializable<T>, Derived> :
    is_base_of<Specializable<T>, Derived> {};
// --- EXPERIMENTAL

static void test_type_traits_1()
{
    constexpr bool v = is_base_of<test::Dummy, test::ChildOfDummy>::value;
    TEST_ASSERT_TRUE(v);
}


static void test_is_same()
{
    bool v = is_same<int, const char*>::value;

    TEST_ASSERT_FALSE(v);
}


static void test_is_constructible()
{
    // FIX: On AVR promicro, this fails - it considers
    // one constructible from another.  Furthermore, this MIGHT
    // be an artifact of the '-fpermissive' flag applied to Arduino
    // on AVR.

    bool v = is_constructible<int, const char*>::value;

    TEST_ASSERT_FALSE(v);

    v = is_constructible<const char*, int>::value;

    TEST_ASSERT_FALSE(v);
}


static void test_is_convertible()
{
    bool v = is_convertible<int, long>::value; 
	TEST_ASSERT_TRUE(v);

    v = is_convertible<int, const char*>::value;

    TEST_ASSERT_FALSE(v);

    v = is_convertible<char[32], const char*>::value;

    TEST_ASSERT_TRUE(v);
}


static void test_is_assignable()
{
    bool v = is_copy_assignable<const char*>::value;

    TEST_ASSERT_TRUE(v);
}

static void test_is_trivially_copyable()
{
    bool v = is_trivially_copyable<const char*>::value;

    TEST_ASSERT_TRUE(v);

    // DEBT: Arguably TestStruct could be trivially copyable, I'd have
    // to review the spec more.  Debt continues in that is_trivially_copyable
    // itself only works with primitives
    //v = is_trivially_copyable<TestStruct>::value;

    //TEST_ASSERT_FALSE(v);
}


#ifdef ESP_IDF_TESTING
TEST_CASE("compile time type_traits", "[type_traits]")
#else
void test_type_traits()
#endif
{
    RUN_TEST(test_type_traits_1);
    RUN_TEST(test_is_same);
    RUN_TEST(test_is_constructible);
    RUN_TEST(test_is_convertible);
    RUN_TEST(test_is_assignable);
    RUN_TEST(test_is_trivially_copyable);
}
