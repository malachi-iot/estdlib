#pragma once

#include "internal/platform.h"
#include "internal/is_empty.h"
#include "internal/type_traits.h"
#include "internal/is_base_of.h"

#include "internal/raw/cstddef.h"

// mainly to fill in gaps where pre-C++03 is used
namespace estd {

#ifdef FEATURE_CPP_VARIADIC
#include "internal/is_function.h"
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

template< class T >
struct is_floating_point
     : integral_constant<
         bool,
         is_same<float, typename remove_cv<T>::type>::value  ||
         is_same<double, typename remove_cv<T>::type>::value  ||
         is_same<long double, typename remove_cv<T>::type>::value
     > {};




namespace detail {
template< class T, bool is_function_type = false >
struct add_pointer :
    type_identity<typename estd::remove_reference<T>::type*> {};

template< class T >
struct add_pointer<T, true> : type_identity<T> {};

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
#ifdef __cpp_variadic_templates
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

#ifdef __cpp_alias_templates
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

}

#include "internal/llvm_type_traits.h"

#ifdef FEATURE_CPP_VARIADIC
#include "internal/invoke_result.h"
#endif

#if defined(FEATURE_CPP_ALIASTEMPLATE) && defined(FEATURE_CPP_DECLTYPE)
#include "internal/common_type.h"
#endif

#include "internal/promoted_type.h"
