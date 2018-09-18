/**
 *  @file
 */
#pragma once

#include "utility.h"
#include "memory.h"

namespace estd {

namespace internal {

struct optional_base_base
{
    bool m_initialized;

    optional_base_base(bool initialized = false) :
        m_initialized(initialized) {}

    bool has_value() const { return m_initialized; }

protected:
    void has_value(bool initialized) { m_initialized = initialized; }
};

template <class T>
struct optional_base : optional_base_base
{
    optional_base(bool initialized = false) : optional_base_base(initialized) {}

    //typename aligned_storage<sizeof(T), alignof (T)>::type storage;
    // TODO: will need attention on the alignment front
    experimental::raw_instance_provider<T> provider;
};


// experimental: use this when we don't need an external flag, but instead some
// specific value of T represents a NULL/not present.  To really work well this
// needs specialization on T, which might not be easy since typedef's fall back
// down to int, short, long, etc.
template <class T, T null_value>
struct optional_reserved_base
{
    //typename aligned_storage<sizeof(T), alignof (T)>::type storage;
    // TODO: will need attention on the alignment front
    experimental::raw_instance_provider<T> provider;

    bool has_value() const { return provider.value() == null_value; }

protected:
    // FIX: this is a noop, but that is kind of clumsy in cases where
    // it's passed in a false - except that currently never happens,
    // therefore there's potential to clean this up
    void has_value(bool) {}
};

}

// with some guidance from https://www.bfilipek.com/2018/05/using-optional.html#intro
template <class T, class TBase = internal::optional_base<T> >
class optional : public TBase
{
    typedef TBase base_type;

public:
    typedef T value_type;

    value_type& value() { return base_type::provider.value(); }
    const value_type& value() const { return base_type::provider.value(); }

    optional() {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    template< class U = T >
    optional& operator=( U&& v )
    {
        new (&value()) value_type(std::move(v));
        base_type::has_value(true);
        return *this;
    }

    optional(value_type&& v) : base_type(true)
    {
        new (&value()) value_type(std::move(v));
    }

    template< class U >
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
    constexpr
#endif
    T value_or( U&& default_value ) &&
    {
        return base_type::has_value() ? value() : std::move(default_value);
    }
#else
    optional(value_type& v) : base_type(true)
    {
        new (&value()) value_type(v);
    }
#endif

#ifdef FEATURE_CPP_VARIADIC
    template< class... TArgs >
    T& emplace( TArgs&&... args )
    {
        T& v = value();
        new (&v) T(std::forward<TArgs>(args)...);
        return v;
    }
#endif

    operator bool() const { return base_type::has_value(); }
};

}
