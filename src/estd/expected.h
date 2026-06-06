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
    using base_type = internal::unexpected<const E>;

public:
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) noexcept = default;

    // DEBT: needs more filters to be not same as unexpected, in_place_t, etc
    template <class Err = E, class =
        enable_if_t<
            is_same<remove_cvref<Err>, unexpected>::value == false>
        >
    constexpr explicit unexpected(Err&& e) : base_type(std::forward<Err>(e)) {}

    // TODO: Need in_place_t ctor
};

#if __cpp_deduction_guides
template <class E>
unexpected(E) -> unexpected<E>;
#endif


template <class T, class E>
class expected : public internal::expected<T, E>
{
    using base_type = internal::expected<T, E>;

    // DEBT: Consider refactor to use 'Positions' enum, just for more consistency
    bool has_value_;

    void destroy()
    {
        if(has_value_)
            base_type::destroy_value();
        else
            base_type::destroy_error();
    }

    ESTD_CPP_CONSTEXPR(14) void assert_has_value() const
    {
        if(!has_value_)
#if __cpp_exceptions
            throw bad_expected_access<E>(base_type::error());
#else
            abort();
#endif
    }

    template <class U>
    ESTD_CPP_CONSTEXPR(14) void assign_value(U&& u)
    {
        base_type::assign_value(has_value_, std::forward<U>(u));
        has_value_ = true;
    }

    template <class G>
    ESTD_CPP_CONSTEXPR(14) void assign_error(G&& e)
    {
        base_type::assign_error(!has_value_, std::forward<G>(e));
        has_value_ = false;
    }

public:
    using unexpected_type = unexpected<E>;
    using typename base_type::nonvoid_value_type;
    using typename base_type::error_type;

    constexpr expected() :
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

    constexpr expected(const expected& copy_from) :
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

    // When U is convertible, converting constructor is implicit.  Aligns with constructor #6 from
    // https://en.cppreference.com/w/cpp/utility/expected/expected.html
    template <class U = remove_cv_t<T>,
        enable_if_t<
            internal::expected_ctor_6<U>::value &&
                is_convertible<U, T>::value,
            int> = 0>
    constexpr expected(U&& v) :
        base_type(in_place_t{}, std::forward<U>(v)),
        has_value_{true}
    {}

    // When U is not convertible, converting constructor is explicit.  Aligns with constructor #6 from
    // https://en.cppreference.com/w/cpp/utility/expected/expected.html.  This expects underlying T
    // has a converting constructor
    template <class U = remove_cv_t<T>,
        enable_if_t<
            internal::expected_ctor_6<U>::value &&
                !is_convertible<U, T>::value,
            int> = 0>
    constexpr explicit expected(U&& v) :
        base_type(in_place_t{}, std::forward<U>(v)),
        has_value_{true}
    {}

    // DEBT: Not tested
    constexpr expected(expected&& move_from) noexcept :
        base_type(std::move(move_from), move_from.has_value()),
        has_value_(move_from.has_value())
    {}

    template <class... Args>
    constexpr explicit expected(in_place_t, Args&&...args) :
        base_type(in_place_t{}, std::forward<Args>(args)...),
        has_value_{true}
    {}

    template <class... Args>
    constexpr explicit expected(unexpect_t, Args&&...args) :
        base_type(unexpect_t{}, std::forward<Args>(args)...),
        has_value_(false)
    {}

    template <class G>
#if __cpp_conditional_explicit || FEATURE_ESTD_STRICT_EXPECTED
    CONSTEXPR_EXPLICIT((!is_convertible<const G&, E>::value)) expected(
        const unexpected<G>& u) :
#else
    constexpr expected(const unexpected<G>& u) :
#endif
        base_type(unexpect_t(), u.error()),
        has_value_(false)
    {}

    ESTD_CPP_CONSTEXPR(14) expected& operator=(const expected& copy_from)
    {
        if(copy_from.has_value())
            assign_value(copy_from.value());
        else
            assign_error(copy_from.error());
        return *this;
    }

    ESTD_CPP_CONSTEXPR(14) expected& operator=(expected&& move_from) noexcept
    {
        if(move_from.has_value())
            assign_value(std::move(move_from.value()));
        else
            assign_error(std::move(move_from.error()));
        return *this;
    }

    template <class U, class enabled = enable_if_t<
        is_same<expected, remove_cvref_t<U>>::value == false &&
        internal::is_unexpected<remove_cvref_t<U>>::value == false &&
        is_constructible<T, U>::value
        >>
    ESTD_CPP_CONSTEXPR(14) expected& operator=(U&& v)
    {
        assign_value(std::forward<U>(v));
        return *this;
    }

    template <class G, class GF = const G&, class enabled = enable_if_t<
        internal::is_variant_assignable<E, GF>::value
        >>
    ESTD_CPP_CONSTEXPR(14) expected& operator=(const unexpected<G>& copy_from)
    {
        assign_error(copy_from.error());
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
        has_value_ = true;
        return base_type::value();
    }

    nonvoid_value_type& value() ESTD_CPP_REFQ
    {
        assert_has_value();

        return base_type::value();
    }

    constexpr const nonvoid_value_type& value() const ESTD_CPP_REFQ
    {
        assert_has_value();

        return base_type::value();
    }

    constexpr bool has_value() const { return has_value_; }

    nonvoid_value_type& operator*() ESTD_CPP_REFQ
    {
        return base_type::value();
    }

    constexpr const nonvoid_value_type& operator*() const ESTD_CPP_REFQ
    {
        return base_type::value();
    }

    nonvoid_value_type&& operator*() &&
    {
        return base_type::value();
    }

    const nonvoid_value_type&& operator*() const&&
    {
        return base_type::value();
    }

    const T* operator->() const
    {
        return &base_type::value();
    }

    constexpr explicit operator bool() const { return has_value_; }

    template <class U>
    constexpr T value_or(U&& default_value) const&
    {
        return has_value_ ? base_type::value() : default_value;
    }
};



}
