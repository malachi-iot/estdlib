#pragma once

#include "internal/platform.h"
#include "internal/type_traits.h"

#include "internal/raw/cstddef.h"

// mainly to fill in gaps where pre-C++03 is used
namespace estd {

#ifdef FEATURE_CPP_VARIADIC
#include "internal/is_function.h"
#endif



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

// Duplicating code because __has_extension is not available in GCC
#ifdef __llvm__
// http://releases.llvm.org/3.5.1/tools/clang/docs/LanguageExtensions.html
#if __has_extension(is_empty)
template<typename _Tp>
struct is_empty
        : public integral_constant<bool, __is_empty(_Tp)>
{};
#endif
#elif defined(__GNUC__)
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __GNUC_PREREQ(4,3)
// https://stackoverflow.com/questions/35531309/how-is-stdis-emptyt-implemented-in-vs2015-or-any-compiler
// https://gcc.gnu.org/onlinedocs/gcc-4.5.4/gcc/Type-Traits.html
// https://www.boost.org/doc/libs/1_65_1/boost/type_traits/intrinsics.hpp
// /usr/include/c++/4.8/type_traits:516
template<typename _Tp>
struct is_empty
        : public integral_constant<bool, __is_empty(_Tp)>
{};
#endif  // GCC exp/4.3
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

#include "internal/promoted_type.h"