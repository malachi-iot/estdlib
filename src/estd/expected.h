#pragma once

#include "internal/expected.h"

namespace estd {

template <class E>
class unexpected : public internal::unexpected<const E>
{
    typedef internal::unexpected<const E> base_type;

public:
#if __cpp_variadic_templates
    template <class Err = E>
    constexpr explicit unexpected(Err&& e) : base_type(std::move(e)) {}
#else
    ESTD_CPP_CONSTEXPR_RET unexpected(const E& e) : base_type(e) {}
#endif
};


template <class T, class E>
class expected : public internal::expected<T, E>
{
    typedef internal::expected<T, E> base_type;

    const bool has_value_;

public:
    typedef unexpected<E> unexpected_type;
    typedef typename base_type::nonvoid_value_type nonvoid_value_type;

    ESTD_CPP_CONSTEXPR_RET expected() :
        has_value_(true)
    {}

    // DEBT: Doesn't do U -> T conversion as per spec
#if __cpp_constexpr
    constexpr explicit
#endif
    expected(nonvoid_value_type&& v) :
        base_type(std::forward<nonvoid_value_type>(v)),
        has_value_(true)
    {}


#if __cpp_variadic_templates
    template <class... TArgs>
    constexpr explicit expected(in_place_t, TArgs&&...args) :
        base_type(in_place_t{}, std::forward<TArgs>(args)...),
        has_value_(true)
    {}

    template <class... TArgs>
    constexpr explicit expected(unexpect_t, TArgs&&...args) :
        base_type(unexpect_t{}, std::forward<TArgs>(args)...),
        has_value_(false)
    {}
#else
    template <class TE1>
    ESTD_CPP_CONSTEXPR_RET expected(unexpect_t, const TE1& e) :
        base_type(unexpect_t{}, e),
        has_value_(false)
    {}
#endif

    CONSTEXPR_EXPLICIT(true) expected(const unexpected_type& u) :
        base_type(unexpect_t{}, u.error()),
        has_value_(false)
    {}

    ESTD_CPP_CONSTEXPR_RET bool has_value() const { return has_value_; }

    const nonvoid_value_type& operator*() const
    {
        return base_type::value();
    }

    const T* operator->() const
    {
        return & base_type::value();
    }

#if __cpp_constexpr
    constexpr explicit
#endif
    operator bool() const { return has_value_; }

#if __cpp_rvalue_references
    template <class U>
    ESTD_CPP_CONSTEXPR_RET T value_or(U&& default_value) const&
    {
        return has_value_ ? base_type::value() : default_value;
    }
#endif
};


}
