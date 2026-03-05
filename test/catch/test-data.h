#pragma once

#include <estd/array.h>
#include <estd/new.h>

#include <estd/cstdint.h>
#include <estd/utility.h>

#include <estd/internal/rtto.h>

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
    bool moved_from_ = false;       ///< This object was the source of a move
    bool destroyed_ = false;        ///< Set when destroyed, somewhat UB to read this so watch out
    bool initialized_ = true;

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
        move_from.moved_from_ = true;
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
        destroyed_ = true;
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
            strcmp(value2, compare_to.value2) < 0;
    }
};

struct ChildOfDummy : Dummy {};

struct VirtualRttoDummy : Dummy, estd::internal::rtto_base::virtual_base
{
    using this_type = VirtualRttoDummy;

    template <class ...Args>
    constexpr VirtualRttoDummy(Args&&...args) : Dummy(std::forward<Args>(args)...)    {}

#if FEATURE_ESTD_RTTO_GET_METADATA
    int get_metadata(const metadata** out) const override
    {
        *out = get<VirtualRttoDummy>();
        return 0;
    }
#endif

    int copy_to(void* dest, int sz) const override
    {
        new (dest) this_type(*this);
        return 0;
    }

    int move_to(void* dest, int sz) override
    {
        new (dest) this_type(std::move(*this));
        return 0;
    }
};

struct Functor : Dummy
{
    using base_type = Dummy;

    int call_count_ = 0;

    Functor() = default;
    ESTD_CPP_FORWARDING_CTOR(Functor)
    ESTD_CPP_DEFAULT_RULE_OF_5(Functor)

    void operator()(int v = 0)
    {
        ++call_count_;
        val1 += v;
    }
};


struct NonCopyable
{
    int val;

    // FIX: We are mostly-copyable with a default constructor
    NonCopyable() = default;

    // FIX: technically we are mostly-copyable with this explicit
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

struct NoDefaultConstructor
{
    int val;

    NoDefaultConstructor() = delete;

    explicit NoDefaultConstructor(int val) : val{val}   {}
};

template <unsigned N>
class Templated
{
public:
    int val1;

    static constexpr unsigned index = N;
};


template <class Base>
struct Forwarder : Base
{
    template <class ...Args>
    Forwarder(Args&&...args) : Base(std::forward<Args>(args)...) {}
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

