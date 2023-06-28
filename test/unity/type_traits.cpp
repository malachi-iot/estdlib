#include <estd/type_traits.h>

#include "unit-test.h"
#include "../catch/test-data.h"

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

#ifdef ESP_IDF_TESTING
TEST_CASE("compile time type_traits", "[type_traits]")
#else
void test_type_traits()
#endif
{
    RUN_TEST(test_type_traits_1);
}