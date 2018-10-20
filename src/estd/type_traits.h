#pragma once

#include "internal/platform.h"

#include "cstddef.h"

// mainly to fill in gaps where pre-C++03 is used
namespace estd {

template <class T> struct remove_cv;
template <class T> struct is_integral;
template <class T> struct is_floating_point;
template <class T> struct is_pointer;
template <class T> struct is_reference;
template <class T> struct is_member_pointer;
template <class T> struct is_union;
template <class T> struct is_class;


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

#ifdef FEATURE_CPP_VARIADIC
#include "internal/is_function.h"
#endif

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

template< bool B, class T, class F >
using conditional_t = typename conditional<B,T,F>::type;
#else
// UNTESTED
template< bool B, class T = void >
class enable_if_t : public enable_if<B, T>::type {};
#endif

#ifdef FEATURE_CPP_ALIGN
template<std::size_t Len, std::size_t Align /* default alignment not implemented */>
struct aligned_storage {
    struct type {
        alignas(Align) byte data[Len];
    };
};
#endif


namespace internal {

template <class T> char is_class_test(int T::*);
struct two { char c[2]; };
template <class T> two is_class_test(...);

template <class _Tp> struct __libcpp_union : public false_type {};

}

template <class _Tp> struct is_union
    : public internal::__libcpp_union<typename remove_cv<_Tp>::type> {};

template <class T>
struct is_class : integral_constant<bool, sizeof(internal::is_class_test<T>(0))==1
                                            && !is_union<T>::value> {};


template<class T> struct is_const          : false_type {};
template<class T> struct is_const<const T> : true_type {};

template <class T> struct is_reference      : false_type {};
template <class T> struct is_reference<T&>  : true_type {};
#ifdef FEATURE_CPP_MOVESEMANTIC
template <class T> struct is_reference<T&&> : true_type {};
#endif

template<class T> struct is_lvalue_reference     : false_type {};
template<class T> struct is_lvalue_reference<T&> : true_type {};

#ifdef FEATURE_CPP_MOVESEMANTIC
template <class T> struct is_rvalue_reference      : false_type {};
template <class T> struct is_rvalue_reference<T&&> : true_type {};
#endif

template< class T >
struct is_arithmetic : integral_constant<bool,
                                    is_integral<T>::value ||
                                    is_floating_point<T>::value> {};

namespace internal {

template< class T > struct ___is_pointer     : false_type {};
template< class T > struct ___is_pointer<T*> : true_type {};

template<typename T,bool = is_arithmetic<T>::value>
struct is_signed : integral_constant<bool, T(-1) < T(0)> {};

template<typename T>
struct is_signed<T,false> : false_type {};

}

template<typename T>
struct is_signed : internal::is_signed<T>::type {};

template< class T > struct is_pointer : internal::___is_pointer<typename remove_cv<T>::type> {};

template<class T> struct is_volatile             : false_type {};
template<class T> struct is_volatile<volatile T> : true_type {};

template< class T >
struct is_floating_point
     : integral_constant<
         bool,
         is_same<float, typename remove_cv<T>::type>::value  ||
         is_same<double, typename remove_cv<T>::type>::value  ||
         is_same<long double, typename remove_cv<T>::type>::value
     > {};


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
    using type = typename estd::remove_reference<T>::type*;
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
struct add_pointer : detail::add_pointer<T, estd::is_function<T>::value> {};

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

#ifdef FEATURE_CPP_ALIASTEMPLATE
template< class T >
using add_const_t    = typename add_const<T>::type;
#endif

// because is_function requires variadic
#ifdef FEATURE_CPP_VARIADIC
template< class T >
struct decay {
private:
    typedef typename estd::remove_reference<T>::type U;
public:
    typedef typename estd::conditional<
        estd::is_array<U>::value,
        typename estd::remove_extent<U>::type*,
        typename estd::conditional<
            estd::is_function<U>::value,
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
#endif

template< class T >
T* addressof(T& arg)
{
    return reinterpret_cast<T*>(
               &const_cast<char&>(
                  reinterpret_cast<const volatile char&>(arg)));
}

#ifdef __llvm__
// http://releases.llvm.org/3.5.1/tools/clang/docs/LanguageExtensions.html
#if __has_extension(is_empty)
template<typename _Tp>
struct is_empty
        : public integral_constant<bool, __is_empty(_Tp)>
{};
#endif
#elif defined(__GNUC__) && defined(__GXX_EXPERIMENTAL_CXX0X__)
// https://stackoverflow.com/questions/35531309/how-is-stdis-emptyt-implemented-in-vs2015-or-any-compiler
#if __GNUC_PREREQ(4,8)
// /usr/include/c++/4.8/type_traits:516
template<typename _Tp>
struct is_empty
        : public integral_constant<bool, __is_empty(_Tp)>
{};
#endif
#elif 0 // LLVM
#endif

}

#include "internal/llvm_type_traits.h"

#ifdef FEATURE_CPP_VARIADIC
#include "internal/is_base_of.h"
#include "internal/invoke_result.h"
#endif

#if defined(FEATURE_CPP_ALIASTEMPLATE) && defined(FEATURE_CPP_DECLTYPE)
#include "internal/common_type.h"
#endif
