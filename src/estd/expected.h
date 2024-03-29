#pragma once

#include "internal/expected.h"

#if __cpp_exceptions
#include <exception>
#endif

namespace estd {

#if __cpp_exceptions
template <class E>
class bad_expected_access;

template <>
class bad_expected_access<void> : public std::exception {};

template <class E>
class bad_expected_access : public bad_expected_access<void>
{
    const E error_;

public:
    bad_expected_access(const E& e) : error_(e) {}

    const E& error() const { return error_; }
};
#endif

template <class E>
class unexpected : public internal::unexpected<const E>
{
    typedef internal::unexpected<const E> base_type;

public:
#if __cpp_rvalue_references
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) noexcept = default;

    // DEBT: needs more filters to be not same as unexpected, in_place_t, etc
    template <class Err = E, class =
        enable_if_t<
            is_same<remove_cvref<Err>, unexpected>::value == false>
        >
    constexpr explicit unexpected(Err&& e) : base_type(std::forward<Err>(e)) {}

    // TODO: Need in_place_t ctor
#else
    ESTD_CPP_CONSTEXPR_RET unexpected(const E& e) : base_type(e) {}
#endif
};

#if __cpp_deduction_guides
template <class E>
unexpected(E) -> unexpected<E>;
#endif


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

    void assert_has_value()
    {
        if(!has_value_)
#if __cpp_exceptions
            throw bad_expected_access<E>(base_type::error());
#else
            abort();
#endif
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
        : base_type(copy_from, copy_from.has_value()),
        has_value_(copy_from.has_value())
    {}
#endif

    ESTD_CPP_CONSTEXPR_RET expected(const expected& copy_from) :
        base_type(copy_from, copy_from.has_value()),
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

    // DEBT: Not tested
    ESTD_CPP_CONSTEXPR_RET expected(expected&& move_from) NOEXCEPT :
        base_type(std::move(move_from), move_from.has_value()),
        has_value_(move_from.has_value())
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

    nonvoid_value_type& value() ESTD_CPP_REFQ
    {
        assert_has_value();

        return base_type::value();
    }

    const nonvoid_value_type& value() const ESTD_CPP_REFQ
    {
        assert_has_value();

        return base_type::value();
    }

    ESTD_CPP_CONSTEXPR_RET bool has_value() const { return has_value_; }

    nonvoid_value_type& operator*() ESTD_CPP_REFQ
    {
        return base_type::value();
    }

    const nonvoid_value_type& operator*() const ESTD_CPP_REFQ
    {
        return base_type::value();
    }

#if __cpp_rvalue_references
    nonvoid_value_type&& operator*() &&
    {
        return base_type::value();
    }

    const nonvoid_value_type&& operator*() const&&
    {
        return base_type::value();
    }
#endif

    const T* operator->() const
    {
        return &base_type::value();
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
