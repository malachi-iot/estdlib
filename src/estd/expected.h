#pragma once

#include "internal/expected.h"

namespace estd {

template <class E>
class unexpected : public internal::unexpected<const E>
{
    typedef internal::unexpected<const E> base_type;

public:
#if __cpp_rvalue_references
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) NOEXCEPT = default;

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

#if __cpp_concepts
    template <class U, class G>
    constexpr explicit expected(const expected<U, G>& copy_from)
        requires(
        ((is_void<T>::value && is_void<U>::value) ||
                (is_constructible_v<T, add_lvalue_reference_t<const U> >)) &&
                is_constructible_v<E, const G&>
        )

#else
    ESTD_CPP_CONSTEXPR_RET EXPLICIT expected(const expected& copy_from)
#endif
        : base_type(copy_from, copy_from.has_value()),
        has_value_(copy_from.has_value())
    {

    }

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

#if __cpp_rvalue_references
    template <class U, class enabled = enable_if_t<internal::expected_ctor_6<U>::value> >
    constexpr expected(U&& v) :
        base_type(in_place_t{}, std::forward<U>(v)),
        has_value_{true}
    {}
#endif


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
    template <class T1>
    ESTD_CPP_CONSTEXPR_RET expected(in_place_t, const T1& v) :
        base_type(in_place_t(), v),
        has_value_(true)
    {}

    template <class TE1>
    ESTD_CPP_CONSTEXPR_RET expected(unexpect_t, const TE1& e) :
        base_type(unexpect_t(), e),
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
        base_type(unexpect_t(), u.error()),
        has_value_(false)
    {}


#if __cpp_rvalue_references
    template <class U, class enabled = enable_if_t<
        is_same<expected, remove_cvref_t<U>>::value == false &&
        internal::is_unexpected<remove_cvref_t<U>>::value == false &&
        is_constructible<T, U>::value
        >>
    expected& operator=(U&& v)
    {
        base_type::assign_value(has_value_, std::forward<U>(v));
        if(!has_value_) has_value_ = true;
        return *this;
    }
#else
    expected& operator=(const nonvoid_value_type& v)
    {
        base_type::assign_value(has_value_, v);
        if(!has_value_) has_value_ = true;
        return *this;
    }
#endif

#if __cplusplus >= 201103L
    template <class G, class GF = const G&, class enabled = enable_if_t<
        internal::is_variant_assignable<E, GF>::value
        >>
#else
    template <class G>
#endif
    expected& operator=(const unexpected<G>& copy_from)
    {
        base_type::assign_error(!has_value_, copy_from.error());
        if(has_value_) has_value_ = false;
        return *this;
    }

    // DEBT: Spec doesn't mention whether a move where neither a value or error is present
    // but I have a feeling we'll need to address that
    ~expected()
    {
        destroy();
    }

#if __cpp_variadic_templates
    template <class ...TArgs>
    nonvoid_value_type& emplace(TArgs&&...args)
    {
        destroy();
        new (&base_type::value()) T(std::forward<TArgs>(args)...);
        has_value_ = true;
        return base_type::value();
    }
#endif

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
