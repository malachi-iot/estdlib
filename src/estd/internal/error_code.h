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

struct system_category_tag {};  // platform-specific, more or less
struct generic_category_tag {}; // portable, more or less

template <class ErrorCodeEnum>
struct error_traits
{
    using value_type = ErrorCodeEnum;

    template <class CategoryTag>
    static const char* to_string(value_type);

    template <class CategoryTag>
    static int map_to(value_type v);
};

template <>
template <>
constexpr int error_traits<errc>::map_to<system_category_tag>(value_type v)
{
    return int(v);
}

template <>
template <>
constexpr int error_traits<errc>::map_to<generic_category_tag>(value_type v)
{
    return int(v);
}

using system_error_traits = error_traits<errc>;

template <class ErrorTraits, class CategoryTag = generic_category_tag>
class error_condition;

template <class ErrorTraits, class CategoryTag = system_category_tag>
class error_code;

template <class ErrorTraits, class CategoryTag>
class error_category
{
public:
    using traits = ErrorTraits;
    using value_type = typename traits::value_type;

    static constexpr string_view message(int condition)
    {
        return traits::template to_string<CategoryTag>(static_cast<value_type>(condition));
    }

    // 27JUL26 MB FIX: This still just doesn't line up with std flavor's philosophy
    template <class CategoryTag2 = generic_category_tag>
    static constexpr error_condition<ErrorTraits, CategoryTag2> default_error_condition(int val)
    {
        return { int(traits::template map_to<CategoryTag2>(static_cast<value_type>(val))) };
    }
};

template <class ErrorTraits, class CategoryTag>
class error_base
{
protected:
    int ec_;

    constexpr error_base(int ec) : ec_{ec}  {}

public:
    using category_type = error_category<ErrorTraits, CategoryTag>;

    static constexpr category_type category() { return {}; }

    constexpr string_view message() const
    {
        return category().message(ec_);
    }

    constexpr int value() const noexcept { return ec_; }
};

// "holds a platform-independent value identifying an error condition"
template <class ErrorTraits, class CategoryTag>
class error_condition : public error_base<ErrorTraits, CategoryTag>
{
    using base_type = error_base<ErrorTraits, CategoryTag>;

public:
    constexpr error_condition(int ec) : base_type{ec}   {}
};

// "represents a platform-dependent error code value"
template <class ErrorTraits, class CategoryTag>
class error_code : public error_base<ErrorTraits, CategoryTag>
{
    using base_type = error_base<ErrorTraits, CategoryTag>;
    using typename base_type::category_type;

public:
    using traits = ErrorTraits;
    using value_type = typename traits::value_type;

    constexpr error_code(value_type ec) : base_type{static_cast<int>(ec)}   {}

    template <class CategoryTag2 = generic_category_tag>
    constexpr error_condition<ErrorTraits, CategoryTag2> default_error_condition() const
    {
        return base_type::category().template default_error_condition<CategoryTag2>(base_type::ec_);
    }
};

}

constexpr internal::error_code<internal::system_error_traits> make_error_code(errc e) noexcept
{
    return { e };
}

constexpr internal::error_condition<internal::system_error_traits> make_error_condition(errc e) noexcept
{
    return { int(e) };
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
