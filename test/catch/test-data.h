#pragma once

#include <estd/utility.h>
#include <new>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

namespace estd { namespace test {

struct Dummy
{
    int val1;
    const char* value2;

    // because underlying struct is an array for layer1::queue, darnit
    Dummy() {}

    Dummy(int val1, const char* val2) :
        val1(val1), value2(val2)
        {}

    Dummy(Dummy&& move_from) :
        val1(std::move(move_from.val1)),
        value2(std::move(move_from.value2))
    {

    }

    explicit Dummy(const Dummy& copy_from) :
        val1(copy_from.val1),
        value2(copy_from.value2)
    {

    }

    ~Dummy()
    {
        const char* val3 = value2;
    }

    // this partially undoes our explicit copy constructor
    Dummy& operator =(const Dummy& copy_from)
    {
        new (this) Dummy(copy_from);
        return *this;
    }
};


struct NonCopyable
{
    int val;

    NonCopyable() {}

    // technically we are somewhat-copyable with this explicit
    // constructor
    explicit NonCopyable(const NonCopyable& copy_from) : val(copy_from.val) {}

    NonCopyable(NonCopyable&& move_from) :
        val(std::move(move_from.val))
    {}
};


struct DefaultConstructor
{
    static constexpr int default_value() { return 0x77; }
    int val;

    DefaultConstructor() : val(default_value()) {}
};


struct EmptyClass {};




static uint8_t octet_data[] { 1, 2, 3, 4, 5, 6, 7, 8, 9 };


static const char* str_simple = "hi2u";
static const char* str_hello = "hello world";
static const char* str_uint1 = "123456";
static const unsigned uint1 = 123456;

struct TestA {};

struct TestB
{
    int counter = 0;

    int add(int val)
    {
        return counter += val;
    }
};

template <class TBase>
struct provider_test : TBase
{
    typedef TBase value_provider;
    typedef typename value_provider::value_type value_type;

    template <class T>
    void do_require(const T& value)
    {
        const value_type& v = value_provider::value();

        REQUIRE(v == value);
    }

    provider_test() {}

    provider_test(int v) : value_provider (v) {}
};


}}

#pragma GCC diagnostic pop

#if __GNUC__
#ifndef __has_warning
// GNUC is permissive of disabling warnings even if it doesn't know about them, I think
#define __has_warning(x)    (true) // FAKE has warning
#endif
#endif
