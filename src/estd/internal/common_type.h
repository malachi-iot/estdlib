#pragma once

#include "platform.h"

// mainly just for tooltips, normally you don't include internal/common_type directly
#include "../type_traits.h"
#include "../utility.h"



namespace estd {
// NOTE: This is rife with C++11 features, so this is a general guard
// TODO: would really like to enable a basic < C++11 capability since chrono
// depends on this
#ifdef FEATURE_CPP_VARIADIC

// lifted from https://en.cppreference.com/w/cpp/types/common_type

// primary template (used for zero types)
template <class ...T> struct common_type {};

//////// one type
template <class T>
struct common_type<T> {
    using type = estd::decay_t<T>;
};

//////// two types

namespace internal
{
// default implementation for two types
template<class T1, class T2>
using cond_t = decltype(false ? std::declval<T1>() : std::declval<T2>());

template<class T1, class T2, class=void>
struct common_type_2_default { };

template<class T1, class T2>
struct common_type_2_default<T1, T2, estd::void_t<cond_t<T1, T2>>> {
    using type = estd::decay_t<cond_t<T1, T2>>;
};

// dispatcher to decay the type before applying specializations
template<class T1, class T2, class D1 = estd::decay_t<T1>, class D2=estd::decay_t<T2>>
struct common_type_2_impl : common_type<D1, D2> {};

template<class D1, class D2>
struct common_type_2_impl<D1, D2, D1, D2> : common_type_2_default<D1, D2> {};
}

template <class T1, class T2>
struct common_type<T1, T2> : internal::common_type_2_impl<T1, T2> { };

//////// 3+ types

#ifdef FEATURE_CPP_VARIADIC
template<class AlwaysVoid, class T1, class T2, class...R>
struct common_type_multi_impl { };

template< class T1, class T2, class...R>
struct common_type_multi_impl<estd::void_t<typename common_type<T1, T2>::type>, T1, T2, R...>
    : common_type<typename common_type<T1, T2>::type, R...>  { };


template <class T1, class T2, class... R>
struct common_type<T1, T2, R...>
    : common_type_multi_impl<void, T1, T2, R...> { };
#endif

#ifdef FEATURE_CPP_ALIASTEMPLATE
template< class... T >
using common_type_t = typename common_type<T...>::type;
#endif

#else  // FEATURE_CPP_VARIADIC

// C++98 mode currently only implemented for chrono specialization

template <class T1 = void, class T2 = void, class T3 = void, class T4 = void>
struct common_type;


#endif // FEATURE_CPP_VARIADIC
}

