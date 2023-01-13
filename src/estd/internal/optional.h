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

template <class T, class enabler = void>
struct optional_value_provider;


// intrinsic variety, no need to go crazy with raw_instance_provider
template <class T>
struct optional_value_provider<T, typename estd::enable_if<estd::is_integral<T>::value>::type >
{
    typedef T value_type;

protected:
    T value_;

    void value(const T& value)
    {
        value_ = value;
    }

    void value(T&& value)
    {
        value_ = std::move(value);
    }

    ESTD_CPP_DEFAULT_CTOR(optional_value_provider)
    optional_value_provider(T value) : value_{value} {}

public:
    value_type& value() { return value_; }
    ESTD_CPP_CONSTEXPR_RET const value_type& value() const { return value_; }

    value_type* operator->() { return &value_; }
    const value_type* operator->() const { return &value_; }
};

template <class T>
struct optional_value_provider<T, typename estd::enable_if<!estd::is_integral<T>::value>::type > :
    experimental::raw_instance_provider<T>
{
    typedef typename experimental::raw_instance_provider<T> provider_type;
    typedef typename provider_type::value_type value_type;
    typedef value_type& return_type;
    typedef const value_type& const_return_type;

    optional_value_provider(bool initialized = false) : optional_base_base(initialized) {}

    //typename aligned_storage<sizeof(T), alignof (T)>::type storage;
    // TODO: will need attention on the alignment front

    value_type* operator->() { return &provider_type::value(); }
    const value_type* operator->() const { return &provider_type::value(); }
};

template <class T>
struct optional_base : optional_value_provider<T>,
   optional_base_base
{
    typedef optional_value_provider<T> base_type;

    ESTD_CPP_DEFAULT_CTOR(optional_base)
    optional_base(const T& copy_from) :
        base_type(copy_from),
        optional_base_base(true)
    {}
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

    ESTD_CPP_CONSTEXPR_RET optional_bitwise(const T& copy_from) :
        value_(copy_from), has_value_{true}
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

}

namespace layer1 { namespace internal {

template <class T, T null_value_>
class optional_base : public estd::internal::optional_value_provider<T>
{
    typedef estd::internal::optional_value_provider<T> base_type;

protected:
//public:
    //optional_base(T& value) : _value(value) {}
    // should always bool == true here
    optional_base(bool) {}

    optional_base() : base_type(null_value_) {}

public:
    typedef T value_type;

    ESTD_CPP_CONSTEXPR_RET bool has_value() const { return base_type::value_ != null_value_; }
    void has_value(bool) {}
    void reset() { base_type::value_ = null_value_; }

    static ESTD_CPP_CONSTEXPR_RET value_type null_value() { return null_value_; }
};

}}

}