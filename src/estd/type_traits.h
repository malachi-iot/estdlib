#pragma once

#include "internal/platform.h"

// mainly to fill in gaps where pre-C++03 is used
// FIX: Need to bring in is_function also
namespace estd {

template<class T, T v>
struct integral_constant {
    static CONSTEXPR T value = v;
    typedef T value_type;
    typedef integral_constant type; // using injected-class-name
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    operator value_type() const { return value; }
    //constexpr value_type operator()() const noexcept { return value; } //since c++14
};


typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

// lifted from http://en.cppreference.com/w/cpp/types/conditional
template<bool B, class T, class F>
struct conditional { typedef T type; };

template<class T, class F>
struct conditional<false, T, F> { typedef F type; };

// lifted from http://en.cppreference.com/w/cpp/types/is_same
template<class T, class U>
struct is_same : false_type {};

template<class T>
struct is_same<T, T> : true_type {};

// lifted from http://en.cppreference.com/w/cpp/types/enable_if
template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };

#ifdef FEATURE_CPP_ALIASTEMPLATE
template< bool B, class T = void >
using enable_if_t = typename enable_if<B,T>::type;
#else
// UNTESTED
template< bool B, class T = void >
class enable_if_t : public enable_if<B, T>::type {};
#endif


template<class T> struct is_const          : false_type {};
template<class T> struct is_const<const T> : true_type {};


template< class T > struct remove_const          { typedef T type; };
template< class T > struct remove_const<const T> { typedef T type; };

template< class T > struct remove_volatile             { typedef T type; };
template< class T > struct remove_volatile<volatile T> { typedef T type; };


template< class T > struct remove_reference      { typedef T type; };
template< class T > struct remove_reference<T&>  { typedef T type; };
#ifdef FEATURE_CPP_MOVESEMANTIC
template< class T > struct remove_reference<T&&> { typedef T type; };
#endif

template< class T >
struct remove_cv {
    typedef typename estd::remove_volatile<typename remove_const<T>::type>::type type;
};


template< class T >
struct add_cv { typedef const volatile T type; };

template< class T> struct add_const { typedef const T type; };

template< class T> struct add_volatile { typedef volatile T type; };

#if defined(FEATURE_CPP_ALIASTEMPLATE)
namespace detail {
template< class T, bool is_function_type = false >
struct add_pointer {
    using type = typename std::remove_reference<T>::type*;
};
 
template< class T >
struct add_pointer<T, true> {
    using type = T;
};

#if defined(FEATURE_CPP_VARIADIC)
template< class T, class... Args >
struct add_pointer<T(Args...), true> {
    using type = T(*)(Args...);
};
 
template< class T, class... Args >
struct add_pointer<T(Args..., ...), true> {
    using type = T(*)(Args..., ...);
};
#endif
 
} // namespace detail
 
template< class T >
struct add_pointer : detail::add_pointer<T, std::is_function<T>::value> {};
#endif

template<class T>
struct is_array : false_type {};

template<class T>
struct is_array<T[]> : true_type {};

template<class T, std::size_t N>
struct is_array<T[N]> : true_type {};

template<class T>
struct remove_extent { typedef T type; };
 
template<class T>
struct remove_extent<T[]> { typedef T type; };
 
template<class T, std::size_t N>
struct remove_extent<T[N]> { typedef T type; };

#ifdef FEATURE_CPP_ENUM_CLASS
// Obviously a simplistic implementation, but it's a start
enum class endian
{
#ifdef _WIN32
    little = 0,
    big    = 1,
    native = little
#else
    little = __ORDER_LITTLE_ENDIAN__,
    big    = __ORDER_BIG_ENDIAN__,
    native = __BYTE_ORDER__
#endif
};
#endif

#ifdef FEATURE_CPP_ALIASTEMPLATEX
template< class T >
using add_const_t    = typename add_const<T>::type;
#endif

// FIX: Have to do this because we didn't bring in our own is_function
// however this cascade of non-constexpr may make it moot
#if __cplusplus >= 201103L
template< class T >
struct decay {
private:
    typedef typename estd::remove_reference<T>::type U;
public:
    typedef typename estd::conditional<
        estd::is_array<U>::value,
        typename estd::remove_extent<U>::type*,
        typename estd::conditional<
            std::is_function<U>::value,
            typename estd::add_pointer<U>::type,
            typename estd::remove_cv<U>::type
        >::type
    >::type type;
};
#endif

#ifdef FEATURE_CPP_ALIASTEMPLATE
template< class... >
using void_t = void;

template< class T >
using decay_t = typename decay<T>::type;
#else
#endif

}

// Normally we try to avoid direct substituting std namespace, but in circumstances
// where std namespace is completely absent,
// oft-used conventions need to be spliced in
#if !defined(FEATURE_STD_UTILITY)
namespace std {

// more or less copy/pasted from GNU reference

 /**
   *  @brief  Utility to simplify expressions used in unevaluated operands
   *  @ingroup utilities
   */

template<typename _Tp, typename _Up = _Tp&&>
    _Up
    __declval(int);

template<typename _Tp>
    _Tp
    __declval(long);

template<typename _Tp>
    auto declval() noexcept -> decltype(__declval<_Tp>(0));

}
#endif

#if defined(FEATURE_CPP_ALIASTEMPLATE) && defined(FEATURE_CPP_DECLTYPE)
#include "internal/common_type.h"
#endif
