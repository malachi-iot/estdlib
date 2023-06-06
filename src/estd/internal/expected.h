#pragma once

#include "platform.h"
#include "type_traits.h"
#include "utility.h"
#include "variant.h"

#include "raw/expected.h"

// c++20 gives us conditional explicits - before that, we generally
// have to leave explicit out.  However, one may force explicit ctors
// by enabling this flag.
#ifndef FEATURE_ESTD_STRICT_EXPECTED
#define FEATURE_ESTD_STRICT_EXPECTED 0
#endif

// DEBT: At first we made an effort to be c++03 compliant, but now things like
// 'are_trivial' and 'variant_storage' are very much c++11 onward.  So, either
// make THEM also c++03 compliant, or remove the sprinkling of __cpp_variadic_templates
// (and friends) checking

namespace estd { namespace internal {

struct expected_tag {};
struct unexpected_tag {};

#if __cplusplus >= 201103L
template <class T>
using expected_ctor_6 = bool_constant<
    is_void<T>::value == false &&
    is_same<remove_cvref_t<T>, in_place_t>::value == false &&
    // DEBT: is_constructible
    is_base_of<expected_tag, remove_cvref_t<T>>::value == false &&
    is_base_of<unexpected_tag, remove_cvref_t<T>>::value == false
    >;
#endif


// Doesn't need to play with uninitialized storage
// since it's always required that E is initialized somehow
// when using this directly as 'unexpected'
template <class E>
class unexpected : public unexpected_tag
{
private:
    const E error_;

protected:
    typedef E error_type;

#if __cpp_variadic_templates
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) NOEXCEPT = default;

    template <class Err = E>
    constexpr explicit unexpected(Err&& e) : error_(std::forward<Err>(e)) {}

    template <class ...TArgs>
    constexpr explicit unexpected(in_place_t, TArgs&&...args) :
        error_(std::forward<TArgs>(args)...)
    {}
#else
    template <class TE1>
    ESTD_CPP_CONSTEXPR_RET unexpected(const TE1& e) : error_(e) {}
#endif

public:
    E& error() { return error_; }
    ESTD_CPP_CONSTEXPR_RET const E& error() const { return error_; }
};


// DEBT: We can't really use estd::layer1::optional here as E and T
// overlap, but perhaps we can use a high-bit filter in controlled
// circumstances

// DEBT: Deviates from spec when T=void, it is treated as 'monostate'.
// If this is an issue, we can specialize this internal::expected.  However,
// even in that case we may need monostate if E is not default constructible
template <class T, class E>
class expected : public expected_tag
{
public:
    typedef T value_type;
    typedef E error_type;

    // Silently promote void to monostate so that it registers as 'trivial'
    // and plays nice with variant_storage.  Deviates from std approach.
    typedef typename conditional<is_void<T>::value, monostate, T>::type nonvoid_value_type;

private:
    enum Positions
    {
        VALUE = 0,
        ERROR = 1
    };

    // DEBT: Consider putting a variant directly here.  Not 100% sure
    // yet whether it's fully compatible (probably though)
    variant_storage<nonvoid_value_type, E> storage;

protected:
    template <class U, class G>
    ESTD_CPP_CONSTEXPR_RET expected(const expected<U, G>& copy_from, bool has_value) :
        storage(copy_from.storage, has_value ? VALUE : ERROR)
    {
    }

    ESTD_CPP_CONSTEXPR_RET expected() :
        storage(in_place_index_t<0>()) {}

#if __cpp_variadic_templates
    template <class... TArgs>
    constexpr explicit expected(in_place_t, TArgs&&...args) :
        storage(in_place_index_t<VALUE>{}, std::forward<TArgs>(args)...)
    {}

    template <class... TArgs>
    constexpr explicit expected(unexpect_t, TArgs&&...args) :
        storage(in_place_index_t<ERROR>{}, std::forward<TArgs>(args)...)
    {}
#else
    template <class T1>
    ESTD_CPP_CONSTEXPR_RET expected(in_place_t, const T1& v) :
        storage(in_place_index_t<VALUE>(), v)
    {}

    template <class TE1>
    ESTD_CPP_CONSTEXPR_RET expected(unexpect_t, const TE1& e) :
        storage(in_place_index_t<ERROR>(), e)
    {}
#endif

#if __cpp_constexpr && __cpp_rvalue_references
    constexpr explicit expected(nonvoid_value_type&& v) :
        storage(in_place_index_t<VALUE>{}, std::forward<nonvoid_value_type>(v))
#else
    expected(const nonvoid_value_type& v) :
        storage(in_place_index_t<VALUE>(), v)
#endif
    {}

    void destroy_value()
    {
        storage.template destroy<VALUE>();
    }

    void destroy_error()
    {
        storage.template destroy<ERROR>();
    }

public:
    nonvoid_value_type& value() { return get<VALUE>(storage); }
    ESTD_CPP_CONSTEXPR_RET const nonvoid_value_type& value() const { return get<VALUE>(storage); }

    E& error() { return get<ERROR>(storage); }
    ESTD_CPP_CONSTEXPR_RET const E& error() const { return get<ERROR>(storage); }
};

#if __cpp_concepts
template <class T>
concept expected_ctor_6_ = expected_ctor_6<T>::value;

#endif


}}
