#pragma once

#include "internal/expected.h"

namespace estd {

template <class E>
class unexpected : public internal::unexpected<const E>
{
    typedef internal::unexpected<const E> base_type;

public:
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) NOEXCEPT = default;

#if __cpp_rvalue_references
    template <class Err = E>
    constexpr explicit unexpected(Err&& e) : base_type(std::forward<Err>(e)) {}
#else
    ESTD_CPP_CONSTEXPR_RET unexpected(const E& e) : base_type(e) {}
#endif
};


template <class T, class E>
class expected : public internal::expected<T, E>
{
    typedef internal::expected<T, E> base_type;

    bool has_value_;

    void destroy()
    {
        if(has_value_)
            base_type::destroy_value();
        else
            base_type::destroy_error();
    }

public:
    typedef unexpected<E> unexpected_type;
    typedef typename base_type::nonvoid_value_type nonvoid_value_type;
    typedef typename base_type::error_type error_type;

    ESTD_CPP_CONSTEXPR_RET expected() :
        has_value_(true)
    {}

#if UNUSED
    // DEBT: Doesn't do U -> T conversion as per spec
#if __cpp_constexpr
    constexpr explicit
#endif
    expected(nonvoid_value_type&& v) :
        base_type(std::forward<nonvoid_value_type>(v)),
        has_value_(true)
    {}
#endif
    template <class U, class enabled = enable_if_t<internal::expected_ctor_6<U>::value> >
    constexpr expected(U&& v) :
        base_type(in_place_t{}, std::forward<U>(v)),
        has_value_{true}
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

    template <class G>
#if __cpp_conditional_explicit || FEATURE_ESTD_STRICT_EXPECTED
    CONSTEXPR_EXPLICIT((!is_convertible<const G&, E>::value)) expected(
        const unexpected<G>& u) :
#else
    ESTD_CPP_CONSTEXPR_RET expected(const unexpected<G>& u) :
#endif
        base_type(unexpect_t{}, u.error()),
        has_value_(false)
    {}


    expected& operator=(nonvoid_value_type&& v)
    {
        if(has_value_)
            base_type::value() = std::forward<nonvoid_value_type>(v);
        else
        {
            base_type::destroy_error();
            new (&base_type::value()) nonvoid_value_type(std::forward<nonvoid_value_type>(v));
            has_value_ = true;
        }

        return *this;
    }

    expected& operator=(const unexpected_type& copy_from)
    {
        const error_type& e = copy_from.error();

        if(!has_value_)
            base_type::error() = e;
        else
        {
            base_type::destroy_value();
            new (&base_type::error()) error_type(e);
            has_value_ = false;
        }
        return *this;
    }

    // DEBT: Spec doesn't mention whether a move where neither a value or error is present
    // but I have a feeling we'll need to address that
    ~expected()
    {
        destroy();
    }

    template <class ...TArgs>
    nonvoid_value_type& emplace(TArgs&&...args)
    {
        destroy();
        new (&base_type::value()) T(std::forward<TArgs>(args)...);
        return base_type::value();
    }

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
