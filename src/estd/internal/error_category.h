#pragma once

#include "errc.h"
#include "../string_view.h"
#include "locale.h"

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


template <class ErrorCodeEnum, class Tag>
struct error_traits
{
    using value_type = ErrorCodeEnum;
    using category = Tag;

    template <class Locale = classic_locale_type>
    static const char* to_string(value_type);

    template <class ErrorTraits2>
    static int map_to(value_type v);
};

// On Linux, this usually maps POSIX error codes.  Not fleshed out
using system_error_traits = error_traits<int, system_category_tag>;

// generic_category is more or less 1:1 with errc
using generic_error_traits = error_traits<errc, generic_category_tag>;

// Crude POSIX comparisons acknowledging that errno codes and errc match in these
// environments
#if __unix__
template <>
template <>
constexpr int error_traits<errc, generic_category_tag>::map_to<system_error_traits>(value_type v)
{
    return int(v);
}

template <>
template <>
constexpr int error_traits<int, system_category_tag>::map_to<generic_error_traits>(value_type v)
{
    return int(v);
}
#endif

template <class ErrorTraits>
class error_condition;

template <class ErrorTraits>
class error_code;

template <class ErrorTraits>
class error_category
{
protected:
    template <class ErrorTraits2>
    static constexpr error_condition<ErrorTraits2> map_to(
        in_place_type_t<ErrorTraits2>, int val)
    {
        return { int(traits::template map_to<ErrorTraits2>(static_cast<value_type>(val))) };
    }

    static constexpr error_condition<ErrorTraits> map_to(
        in_place_type_t<ErrorTraits>, int val)
    {
        return { val };
    }

public:
    using traits = ErrorTraits;
    using value_type = typename traits::value_type;

    template <class Locale = classic_locale_type>
    static constexpr string_view message(int condition)
    {
        return traits::template to_string<Locale>(static_cast<value_type>(condition));
    }

    // 27JUL26 MB DEBT: This still just doesn't line up with std flavor's philosophy,
    // though we aren't far now (a subclass may do the trick... ?)
    template <class ErrorTraits2 = generic_error_traits>
    static constexpr error_condition<ErrorTraits2> default_error_condition(int val)
    {
        return map_to(in_place_type_t<ErrorTraits2>{}, val);
    }
};

// EXPERIMENTAL
using generic_category = error_category<generic_error_traits>;
using system_category = error_category<system_error_traits>;

}

}

#if FEATURE_STD_SYSTEM_ERROR_ALIAS
namespace std {
#else
namespace estd {

template<>
struct is_error_condition_enum<errc> : true_type {};
#endif
}
