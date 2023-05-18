#pragma once

#include "platform.h"
#include "type_traits.h"
#include "utility.h"
#include "variant.h"

#include "raw/expected.h"

// DEBT: At first we made an effort to be c++03 compliant, but now things like
// 'are_trivial' and 'variant_storage' are very much c++11 onward.  So, either
// make THEM also c++03 compliant, or remove the sprinkling of __cpp_variadic_templates
// (and friends) checking

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


// Silently promote void to monostate so that it registers as 'trivial'
template <class T, class E>
using expected_is_trivial =
    are_trivial<conditional_t<is_void<T>::value, monostate, T>, E>;


template <class T, class E, bool trivial = expected_is_trivial<T, E>::value>
class expected;

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
    typedef conditional_t<is_void<T>::value, monostate, T> nonvoid_value_type;

    variant_storage2<nonvoid_value_type, E> storage;

    ESTD_CPP_CONSTEXPR_RET expected() :
        storage(in_place_index_t<0>{}, nonvoid_value_type{}) {}

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

    ESTD_CPP_CONSTEXPR_RET expected(const nonvoid_value_type& v) :
        storage(in_place_index_t<0>{}, v)
    {}

public:
    // DEBT: Deviates from spec when void is involved, returns monostate
    nonvoid_value_type& value() { return get<0>(storage); }
    ESTD_CPP_CONSTEXPR_RET const nonvoid_value_type& value() const { return get<0>(storage); }

    E& error() { return get<1>(storage); }
    ESTD_CPP_CONSTEXPR_RET const E& error() const { return get<1>(storage); }

    const nonvoid_value_type& operator*() const { return value(); }

    expected& operator=(nonvoid_value_type&& v)
    {
        // FIX: Should we be doing std::move here?  And if not, document why not
        value() = v;
    }
};


// Non trivial flavor, mainly need this because we can't
// do constexprs so easily with non trivial one
template <class T, class E>
class expected<T, E, false>
{
    variant_storage<false, T, E> storage;

protected:
    typedef conditional_t<is_void<T>::value, monostate, T> nonvoid_value_type;

#if __cpp_variadic_templates
    template <class... TArgs>
    constexpr explicit expected(in_place_t, TArgs&&...args) :
        storage(in_place_index_t<0>{}, std::forward<TArgs>(args)...)
    {
    }

    template <class... TArgs>
    constexpr explicit expected(unexpect_t, TArgs&&...args) :
        storage(in_place_index_t<1>{}, std::forward<TArgs>(args)...)
    {
    }
#endif

    // DEBT: Figure out how to make this and others work on initializing list line
    // so that we can constexpr it
    constexpr explicit expected(const nonvoid_value_type& v) :
        storage(in_place_index_t<0>{}, v)
    {
    }

    constexpr explicit expected() :
        storage(in_place_index_t<0>{}, T{})
    {
    }

public:
    typedef T value_type;
    typedef E error_type;

    nonvoid_value_type& value() { return get<0>(storage); }
    const nonvoid_value_type& value() const { return get<0>(storage); }
    E& error() { return get<1>(storage); }
    const E& error() const { return get<1>(storage); }

    const nonvoid_value_type& operator*() const { return value(); }
};


}}
