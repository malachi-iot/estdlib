#pragma once

#include "platform.h"

namespace estd { namespace internal {

struct has_member_base
{
    // For the compile time comparison.
    typedef const char yes[1];
    typedef yes no[2];

    // helpers to suppress warnings on non-return type
    // pretty sure this only appears on rather old (pre c++11) compilers who aren't sure that it compile-time
    // resolves to not matter.
#ifdef FEATURE_CPP_INLINE_STATIC
    static CONSTEXPR yes yes_value = { 0 };
    // SAMD compiler fails on this one, so can't use it
    static CONSTEXPR no no_value = { { 0 }, { 1 } };
#else
    static CONSTEXPR yes yes_value;
    static CONSTEXPR no no_value;
#endif

    // This helper struct permits us to check that serialize is truly a method.
    // The second argument must be of the type of the first.
    // For instance reallyHas<int, 10> would be substituted by reallyHas<int, int 10> and works!
    // reallyHas<int, &C::serialize> would be substituted by reallyHas<int, int &C::serialize> and fail!
    // Note: It only works with integral constants and pointers (so function pointers work).
    // In our case we check that &C::serialize has the same signature as the first argument!
    // reallyHas<std::string (C::*)(), &C::serialize> should be substituted by
    // reallyHas<std::string (C::*)(), std::string (C::*)() &C::serialize> and work!
    template <typename U, U u> struct reallyHas;
};

} }

// TODO: Replace regular ESTD_HAS_METHOD_EXPERIMENTAL
#define ESTD_FN_HAS_METHOD(ret_type, method_name, ...) \
template <class T> struct has_##method_name##_method : has_member_base \
{ \
    template <typename C> static CONSTEXPR yes& test(reallyHas<ret_type (C::*)(__VA_ARGS__), &C::method_name>* /*unused*/) \
    { return yes_value; }  \
\
    template <typename> static CONSTEXPR no& test(...) { return no_value; } \
\
    static CONSTEXPR bool value = sizeof(test<T>(0)) == sizeof(yes); \
};
