#pragma once

#include <estd/new.h>

#include <estd/cstdint.h>
#include <estd/utility.h>

#include "macro/push.h"

#include "test/nontrivial.h"

namespace estd { namespace test {

struct Dummy
{
    int val1;
    const char* value2;
    int* inc_on_destruct = nullptr;
    const bool moved_ = false;      ///< This object was made from a move
    const bool copied_ = false;     ///< This object was made via a copy

    // because underlying struct is an array for layer1::queue, darnit
    Dummy() = default;

    Dummy(int val1, const char* val2, int* inc_on_destruct = nullptr) :
        val1(val1), value2(val2), inc_on_destruct(inc_on_destruct)
        {}

    Dummy(Dummy&& move_from) NOEXCEPT :
        val1(move_from.val1),
        value2(move_from.value2),
        inc_on_destruct(move_from.inc_on_destruct),
        moved_{true}
    {
        move_from.inc_on_destruct = nullptr;
    }

    Dummy(const Dummy& copy_from) :
        val1(copy_from.val1),
        value2(copy_from.value2),
        inc_on_destruct(copy_from.inc_on_destruct),
        copied_{true}
    {

    }

    ~Dummy()
    {
        const char* val3 = value2;
        if(inc_on_destruct)
            ++*inc_on_destruct;
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

    bool operator==(const Dummy& compare_to) const
    {
        return val1 == compare_to.val1 &&
            value2 == compare_to.value2;
    }

    bool operator<(const Dummy& compare_to) const
    {
        return val1 < compare_to.val1 ? true :
            val1 > compare_to.val1 ? false :
            value2 < compare_to.value2;
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

template <unsigned N>
class Templated
{
public:
    int val1;

    static constexpr unsigned index = N;
};


struct EmptyClass {};




static uint8_t octet_data[] { 1, 2, 3, 4, 5, 6, 7, 8, 9 };


static const char* str_simple = "hi2u";
static const char* str_hello = "hello world";
static const char* str_uint1 = "123456";
static const uint32_t uint1 = 123456;

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


// Mainly used in 'functional' area
struct ContextTest
{
    int val = 0;

    int add(int v) { return val += v; }

    // NOTE: Cannot name 'add' since estd::experimental::context_function is unable to resolve
    // overloads
    void add2() { val += 7; }

    static int add3(int v, ContextTest* c)
    {
        return c->add(v);
    }
};



#include "macro/pop.h"

