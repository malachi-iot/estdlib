#include <catch2/catch_all.hpp>

#include "estd/array.h"
//#include "estd/exp/buffer.h"
#include "mem.h"
#include "test-data.h"
#include <estd/string.h>
#include <estd/memory.h>
#include <estd/functional.h>
#include "estd/streambuf.h"
#include <estd/charconv.h>
#include <estd/cctype.h>
//#include <estd/locale.h>
#include "estd/exp/unique_handle.h"

using namespace estd::test;

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

int global_provider_test_value = 6;

estd::layer1::string<128> provider_string;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// Windows already defines this
#ifndef _STATIC_ASSERT
// trying this
// https://stackoverflow.com/questions/2831934/how-to-use-if-inside-define-in-the-c-preprocessor
#define _STATIC_ASSERT(expr) STATIC_ASSERT_##expr
#define STATIC_ASSERT(expr) _STATIC_ASSERT(expr)
#define STATIC_ASSERT_true
#define STATIC_ASSERT_false #error Assert Failed

// UNTESTED, UNFINISHED
#if !defined(__cpp_static_assert)
#define static_assert(expr, message) STATIC_ASSERT(expr)
#endif
#endif



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
            {
                typedef estd::internal::deduce_fixed_size_t<4> deducer_t;
                REQUIRE(sizeof(deducer_t::size_type) == 1);
            }
            {
                typedef estd::internal::deduce_fixed_size_t<10> deducer1_t;
                REQUIRE(sizeof(deducer1_t::size_type) == 1);
            }
            {
                typedef estd::internal::deduce_fixed_size_t<100> deducer_t;
                REQUIRE(sizeof(deducer_t::size_type) == 1);
            }
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

            provider_test<global_provider<
                    estd::layer1::string<128>&,
                            provider_string> > pt2;

            estd::layer1::string<64> f = "hi2u";

            provider_string = "hi2u";

            REQUIRE(pt2.value() == f);
            REQUIRE(pt2.value() == "hi2u");

            pt2.do_require(f);
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
    SECTION("instance wrapper")
    {
        using namespace estd;
        using namespace estd::experimental;

        SECTION("simplistic")
        {
            instance_wrapper<int> a;

            a.construct(5);

            REQUIRE(a == 5);
        }
        SECTION("Dummy class")
        {
            instance_wrapper<test::Dummy> a;

            a.construct(7, "hi2u");

            REQUIRE(a.value().val1 == 7);
        }
        SECTION("non copyable")
        {
            instance_wrapper<test::NonCopyable> a;

            a.construct();
        }
    }
    SECTION("streambuf")
    {
        SECTION("streambuf-traits")
        {
            char buf[] = "Hello";
            estd::span<char> span = buf;
            typedef char char_type;
            typedef estd::internal::streambuf<
                    estd::internal::impl::in_span_streambuf<char_type >> streambuf_type;
            typedef estd::experimental::streambuf_traits<streambuf_type> streambuf_traits;

            streambuf_type sb(span);

            estd::span<char> same_span = streambuf_traits::gdata(sb);

            REQUIRE(span.size() == same_span.size());
            REQUIRE(same_span[0] == buf[0]);
        }
    }
    SECTION("function_traits")
    {
        using namespace estd::experimental;

        SECTION("member function pointer")
        {
            TestB v;

            typedef function_ptr_traits<decltype(&TestB::add), &TestB::add> fn_traits;

            REQUIRE(estd::is_same<fn_traits::this_type, TestB>::value);
            REQUIRE(estd::is_same<fn_traits::arg<0>, int>::value);
        }
    }
#ifdef STATIC_ASSERT
    SECTION("STATIC_ASSERT")
    {
        STATIC_ASSERT(true);
        //STATIC_ASSERT(false); // does indeed halt compilation, clunky though
    }
#endif
}

#pragma GCC diagnostic pop
