#pragma once

#include "platform.h"
#include "fwd/utility.h"
#include "raw/utility.h"
#include "legacy/utility.h"
#include "raw/type_traits.h"

#if FEATURE_STD_UTILITY
// Utilize stock-standard std version of this if it's available
#include <utility>
#else
#include "utility/declval.h"
#include "utility/forward.h"
#endif

namespace estd { namespace internal {

// https://stackoverflow.com/questions/42175294/how-get-the-class-object-type-from-pointer-to-method
// TODO: Fuse with functional's invoke_result
template<class T>
struct MethodInfo;

#ifdef FEATURE_CPP_VARIADIC
template<class C, class R, class... A>
struct MethodInfo<R(C::*)(A...)> //method pointer
{
    typedef C ClassType;
    typedef R ReturnType;
    //typedef estd::tuple<A...> ArgsTuple;
};

template<class C, class R, class... A>
struct MethodInfo<R(C::*)(A...) const> : MethodInfo<R(C::*)(A...)> {}; //const method pointer
#else
template<class C, class R, class P1>
struct MethodInfo<R(C::*)(P1)> //method pointer
{
    typedef C ClassType;
    typedef R ReturnType;
    //typedef estd::tuple<A...> ArgsTuple;
};
#endif



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

#if defined(FEATURE_CPP_DECLTYPE) && defined(FEATURE_CPP_ALIASTEMPLATE)
    //typedef estd::internal::MethodInfo<decltype(&T::method_name)> method_info;
    //template <typename F>
    //typedef
#endif

};

template <typename>
struct has_typedef { typedef void type; };


} }


