#include <catch.hpp>

#include "estd/array.h"
//#include "estd/exp/buffer.h"
#include "mem.h"

struct TestA {};

struct Test
{
    int a;
    float b;

    //std::string s;
    TestA& t;

    constexpr Test(int a, float b, TestA& t) :
        a(a), b(b), t(t) {}
};

TestA t;

template <class TBase>
struct provider_test : TBase
{
    typedef TBase value_provider;

    void do_require(int value)
    {
        int v = value_provider::value();

        REQUIRE(v == value);
    }

    provider_test() {}

    provider_test(int v) : value_provider (v) {}
};

int global_provider_test_value = 6;


TEST_CASE("experimental tests")
{
    SECTION("A")
    {
        //estd::experimental::layer0
        constexpr Test test[] =
        {
            { 1,2, t },
            { 2, 3, t}
        };
        //constexpr Test test1(1, 2, t);
    }
    SECTION("accessor")
    {
        _allocator<int> a;
        int* val = a.allocate(1);

        *val = 5;

        estd::experimental::stateful_locking_accessor<_allocator<int>> acc(a, val);

        int& val2 = acc;

        REQUIRE(val2 == 5);

        a.deallocate(val, 1);
    }
    SECTION("bitness size_t deducer")
    {
        SECTION("8-bit")
        {
            typedef estd::internal::deduce_fixed_size_t<10> deducer_t;
            REQUIRE(sizeof(deducer_t::size_type) == 1);
        }
        {
            typedef estd::internal::deduce_fixed_size_t<100> deducer_t;
            REQUIRE(sizeof(deducer_t::size_type) == 1);
        }
        SECTION("16-bit")
        {
            typedef estd::internal::deduce_fixed_size_t<1000> deducer_t;
            REQUIRE(sizeof(deducer_t::size_type) == 2);
        }
    }
    SECTION("providers")
    {
        using namespace estd::experimental;

        SECTION("temporary")
        {
            provider_test<temporary_provider<int> > pt;

            REQUIRE(pt.value() == 0);

            pt.do_require(0);
        }
        SECTION("instanced")
        {
            provider_test<instance_provider<int> > pt(5);

            REQUIRE(pt.value() == 5);

            pt.do_require(5);
        }
        SECTION("global")
        {
            provider_test<global_provider<int&, global_provider_test_value> > pt;

            REQUIRE(pt.value() == 6);

            pt.do_require(6);
        }
        SECTION("global")
        {
            provider_test<literal_provider<int, 7> > pt;

            REQUIRE(pt.value() == 7);

            pt.do_require(7);
        }
        SECTION("pointer from value")
        {
            provider_test<pointer_from_instance_provider<int> > pt(5);

            REQUIRE(*pt.value() == 5);
        }
    }
}
