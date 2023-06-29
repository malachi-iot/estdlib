#pragma once

#include <new>

#include <estd/cstdint.h>
#include <estd/utility.h>

#include "macro/push.h"

#include "test/nontrivial.h"

namespace estd { namespace test {

struct Dummy
{
    int val1;
    const char* value2;
    const bool moved_ = false;
    const bool copied_ = false;

    // because underlying struct is an array for layer1::queue, darnit
    Dummy() = default;

    Dummy(int val1, const char* val2) :
        val1(val1), value2(val2)
        {}

    Dummy(Dummy&& move_from) NOEXCEPT :
        val1(move_from.val1),
        value2(move_from.value2),
        moved_{true}
    {

    }

    Dummy(const Dummy& copy_from) :
        val1(copy_from.val1),
        value2(copy_from.value2),
        copied_{true}
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

    Dummy& operator=(Dummy&& move_from)
    {
        new (this) Dummy(std::move(move_from));
        return *this;
    }
};

struct ChildOfDummy : Dummy {};


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

#include "macro/pop.h"

