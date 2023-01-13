#pragma once

#include "../new.h"

namespace estd { namespace internal {

struct optional_tag_base
{
    typedef void optional_tag;

    // DEBT: Are we really using this anymore?  This experiment wasn't a full success
    ESTD_FN_HAS_TYPEDEF_EXP(optional_tag)
};

struct optional_base_base
{
    bool m_initialized;

    optional_base_base(bool initialized = false) :
        m_initialized(initialized) {}

    bool has_value() const { return m_initialized; }
    void reset() { m_initialized = false; }

protected:
    void has_value(bool initialized) { m_initialized = initialized; }
};

template <class T>
struct optional_base : optional_base_base,
    experimental::raw_instance_provider<T>
{
    typedef typename experimental::raw_instance_provider<T> provider_type;
    typedef typename provider_type::value_type value_type;
    typedef value_type& return_type;
    typedef const value_type& const_return_type;

    optional_base(bool initialized = false) : optional_base_base(initialized) {}

    //typename aligned_storage<sizeof(T), alignof (T)>::type storage;
    // TODO: will need attention on the alignment front

    value_type* operator->() { return &provider_type::value(); }
    const value_type* operator->() const { return &provider_type::value(); }
};


template <class T>
class optional_bitwise
{
    struct
    {
        T value_ : 1;
        bool has_value_ : 1;
    };

protected:
    void value(T v) { value_ = v; }
    void has_value(bool initialized) { has_value_ = initialized; }

    ESTD_CPP_CONSTEXPR_RET optional_bitwise() :
        has_value_{false}
    {}

public:
    typedef T value_type;
    typedef value_type return_type;
    typedef value_type const_return_type;

    bool has_value() const { return has_value_; }
    void reset() { has_value_ = false; }

    // Deviates from spec here, since bitfield precludes returning a reference    
    ESTD_CPP_CONSTEXPR_RET value_type value() const { return value_; }
};

}}