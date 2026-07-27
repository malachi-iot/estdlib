#pragma once

#include "errc.h"
#include "../string_view.h"


namespace estd {

#if FEATURE_STD_SYSTEM_ERROR_ALIAS
using std::is_error_code_enum;
using std::is_error_condition_enum;
#else
template <class>
struct is_error_code_enum;

template <class>
struct is_error_condition_enum;
#endif

namespace internal {

template <class ErrorCodeEnum>
struct error_traits
{
    using value_type = ErrorCodeEnum;

    static const char* to_condition_string(value_type);
    static const char* to_code_string(value_type);
};

using system_error_traits = error_traits<errc>;

template <class ErrorTraits>
class error_category
{
public:
    using traits = ErrorTraits;

    static constexpr string_view message(int condition)
    {
        using value_type = typename traits::value_type;

        return traits::to_condition_string(static_cast<value_type>(condition));
    }
};

template <class ErrorTraits>
class error_base
{
protected:
    int ec_;

    constexpr error_base(int ec) : ec_{ec}  {}

public:
    using category_type = error_category<ErrorTraits>;

    static constexpr category_type category() { return {}; }

    constexpr string_view message() const
    {
        return category().message(ec_);
    }
};

// "holds a platform-independent value identifying an error condition"
template <class ErrorTraits>
class error_condition : public error_base<ErrorTraits>
{

};

// "represents a platform-dependent error code value"
template <class ErrorTraits>
class error_code : public error_base<ErrorTraits>
{
    using base_type = error_base<ErrorTraits>;

public:
    using traits = ErrorTraits;
    using value_type = typename traits::value_type;

    constexpr error_code(value_type ec) : base_type{static_cast<int>(ec)}   {}
};

}

constexpr internal::error_code<internal::system_error_traits> make_error_code(errc e) noexcept
{
    return { e };
}

}

#if FEATURE_STD_SYSTEM_ERROR_ALIAS
namespace std {
#else
namespace estd {

template<>
struct is_error_code_enum<errc> : true_type {};
#endif

}
