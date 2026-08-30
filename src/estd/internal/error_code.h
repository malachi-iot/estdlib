#pragma once

#include "error_category.h"

namespace estd {

namespace internal {

template <class ErrorTraits>
class error_base
{
protected:
    int ec_;

    constexpr error_base(int ec) : ec_{ec}  {}

public:
    using category_type = error_category<ErrorTraits>;

    static constexpr category_type category() { return {}; }

    template <class Locale = classic_locale_type>
    constexpr string_view message() const
    {
        return category().template message<Locale>(ec_);
    }

    constexpr int value() const noexcept { return ec_; }
};

// "holds a platform-independent value identifying an error condition"
template <class ErrorTraits>
class error_condition : public error_base<ErrorTraits>
{
    using base_type = error_base<ErrorTraits>;

public:
    constexpr error_condition(int ec) : base_type{ec}   {}
};

// "represents a platform-dependent error code value"
template <class ErrorTraits>
class error_code : public error_base<ErrorTraits>
{
    using base_type = error_base<ErrorTraits>;
    using typename base_type::category_type;

public:
    using traits = ErrorTraits;
    using value_type = typename traits::value_type;

    constexpr error_code(value_type ec) : base_type{static_cast<int>(ec)}   {}

    template <class ErrorTraits2 = generic_error_traits>
    constexpr error_condition<ErrorTraits2> default_error_condition() const
    {
        return base_type::category().template default_error_condition<ErrorTraits2>(base_type::ec_);
    }
};

}

constexpr internal::error_code<internal::generic_error_traits> make_error_code(errc e) noexcept
{
    return { e };
}

constexpr internal::error_condition<internal::generic_error_traits> make_error_condition(errc e) noexcept
{
    return { int(e) };
}

}

