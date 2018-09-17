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

    operator bool() const { return has_value(); }

protected:
    void has_value(bool initialized) { m_initialized = initialized; }
};

template <class T>
struct optional_base : optional_base_base
{
    optional_base(bool initialized = false) : optional_base_base(initialized) {}
};

}

// with some guidance from https://www.bfilipek.com/2018/05/using-optional.html#intro
template <class T, class TBase = internal::optional_base<T> >
class optional : public TBase
{
    typedef TBase base_type;
    //typename aligned_storage<sizeof(T), alignof (T)>::type storage;
    // TODO: will need attention on the alignment front
    experimental::raw_instance_provider<T> provider;

public:
    typedef T value_type;

    value_type& value() { return provider.value(); }
    const value_type& value() const { return provider.value(); }

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
    constexpr T value_or( U&& default_value ) &&
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
};

}
