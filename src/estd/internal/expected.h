#pragma once

#include "platform.h"
#include "type_traits.h"
#include "utility.h"
#include "variant.h"

#include "raw/expected.h"

namespace estd { namespace internal {

// Doesn't need to play with uninitialized storage
// since it's always required that E is initialized somehow
// when using this directly as 'unexpected'
template <class E>
class unexpected
{
private:
    const E error_;

protected:
    typedef E error_type;

    ESTD_CPP_DEFAULT_CTOR(unexpected)

#if __cpp_variadic_templates
    template <class Err = E>
    constexpr explicit unexpected(Err&& e) : error_(std::move(e)) {}

    template <class ...TArgs>
    constexpr explicit unexpected(in_place_t, TArgs&&...args) :
        error_(std::forward<TArgs>(args)...)
    {}
#else
    template <class TE1>
    ESTD_CPP_CONSTEXPR_RET unexpected(const TE1& e) : error_(e) {}
#endif

public:
    ESTD_CPP_CONSTEXPR_RET const E& error() const { return error_; }
};


template <class T, class E>
struct expected_is_trivial;

template <class T, class E>
using expected_is_trivial2 =
    are_trivial<conditional_t<is_void<T>::value, monostate, T>, E>;


template <class T, class E, bool trivial =
        //expected_is_trivial<T, E>::is_trivial>
        expected_is_trivial2<T, E>::value>
        //are_trivial<T, E>::value>
class expected;

template <class T, class E>
struct expected_is_trivial
{
    // DEBT: Do this all with an integral constant, just having a brain
    // fart how to do an or operation in that context
    static CONSTEXPR bool is_trivial =
#if FEATURE_ESTD_IS_TRIVIAL
        estd::is_trivial<T>::value & estd::is_trivial<E>::value;
#elif FEATURE_STD_TYPE_TRAITS
        std::is_trivial<T>::value & std::is_trivial<E>::value;
#else
        false;
#endif
};

template <class E>
struct expected_is_trivial<void, E>
{
    static CONSTEXPR bool is_trivial =
#if FEATURE_ESTD_IS_TRIVIAL
        estd::is_trivial<E>::value;
#elif FEATURE_STD_TYPE_TRAITS
        std::is_trivial<E>::value;
#else
        false;
#endif
};

// DEBT: We can't really use estd::layer1::optional here as E and T
// overlap, but perhaps we can use a high-bit filter in controlled
// circumstances


// Trivial flavor
template <class T, class E>
class expected<T, E, true>
{
public:
    typedef T value_type;
    typedef E error_type;

protected:
    typedef T nonvoid_value_type;

    variant_storage2<T, E> storage;

    //ESTD_CPP_CONSTEXPR_RET
    expected() :
        storage(in_place_index_t<0>{}, T{}) {}

#if __cpp_variadic_templates
    template <class... TArgs>
    constexpr explicit expected(in_place_t, TArgs&&...args) :
        storage(in_place_index_t<0>{}, std::forward<TArgs>(args)...)
    {}

    template <class... TArgs>
    //constexpr
    explicit expected(unexpect_t, TArgs&&...args) :
        storage(in_place_index_t<1>{}, std::forward<TArgs>(args)...)
    {}
#else
    template <class TE1>
    ESTD_CPP_CONSTEXPR_RET expected(unexpect_t, const TE1& e) : error_(e) {}
#endif

    // DEBT: Bring this back
    //ESTD_CPP_CONSTEXPR_RET
    expected(const value_type& v) :
        storage(in_place_index_t<0>{}, v)
    {}

public:
    T& value() { return get<0>(storage); }
    ESTD_CPP_CONSTEXPR_RET const T& value() const { return get<0>(storage); }

    E& error() { return get<1>(storage); }
    ESTD_CPP_CONSTEXPR_RET const E& error() const { return get<1>(storage); }

    const T& operator*() const { return value(); }

    expected& operator=(value_type&& v)
    {
        // FIX: Should we be doing std::move here?  And if not, document why not
        value() = v;
    }
};


// Non trivial flavor
template <class T, class E>
class expected<T, E, false>
{
    variant_storage<false, T, E> storage;

protected:
    typedef T nonvoid_value_type;

#if __cpp_variadic_templates
    template <class... TArgs>
    explicit expected(in_place_t, TArgs&&...args) :
        storage(in_place_index_t<0>{}, std::forward<TArgs>(args)...)
    {
    }

    template <class... TArgs>
    explicit expected(unexpect_t, TArgs&&...args) :
        storage(in_place_index_t<1>{}, std::forward<TArgs>(args)...)
    {
    }
#endif

    // DEBT: Figure out how to make this and others work on initializing list line
    // so that we can constexpr it
    explicit expected(const nonvoid_value_type& v) :
        storage(in_place_index_t<0>{}, v)
    {
    }

    explicit expected() :
        storage(in_place_index_t<0>{}, T{})
    {
    }

public:
    typedef T value_type;
    typedef E error_type;

    T& value() { return get<0>(storage); }
    const T& value() const { return get<0>(storage); }
    E& error() { return get<1>(storage); }

    const T& operator*() const { return value(); }
};


template <class E>
class expected<void, E, true>
{
    variant_storage<true, monostate, E> storage;

public:
    typedef void value_type;
    typedef estd::monostate nonvoid_value_type;
    typedef E error_type;

protected:
    ESTD_CPP_DEFAULT_CTOR(expected)

#if __cpp_variadic_templates
    template <class... TArgs>
    constexpr explicit expected(unexpect_t, TArgs&&...args) :
        storage(in_place_index_t<1>{}, std::forward<TArgs>(args)...)
    {}
#else
    template <class TE1>
    ESTD_CPP_CONSTEXPR_RET expected(unexpect_t, const TE1& e) : base_type(e) {}
#endif

public:
    static void value() { }

    E& error() { return get<1>(storage); }
    const E& error() const { return get<1>(storage); }
};

}}
