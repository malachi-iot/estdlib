#pragma once

#include "../raw/type_traits.h"
// DEBT: Piecemeal out dependencies to reduce overall dependency footprint.
// required: void_t, true_type, is_class, remove_cv, integral_constant
#include "../../utility.h"

namespace estd {

// Shameless lifted from various places including
// https://en.cppreference.com/w/cpp/types/is_base_of

// DEBT: Piecemeal out dependencies rather than a c++11 check
//#if __cpp_decltype
#if __cplusplus >= 201103L

namespace internal {
    template <typename Base> estd::true_type is_base_of_test_func(const volatile Base*);
    template <typename Base> estd::false_type is_base_of_test_func(const volatile void*);

#if LEGACY
    template <typename Base, typename Derived>
    using pre_is_base_of = decltype(is_base_of_test_func<Base>(std::declval<Derived*>()));

    // with <experimental/type_traits>:
    // template <typename Base, typename Derived>
    // using pre_is_base_of2 = std::experimental::detected_or_t<std::true_type, pre_is_base_of, Base, Derived>;
    template <typename Base, typename Derived, typename = void>
    struct pre_is_base_of2 : public estd::true_type { };
    // note std::void_t is a C++17 feature
    template <typename Base, typename Derived>
    struct pre_is_base_of2<Base, Derived, estd::void_t<pre_is_base_of<Base, Derived>>> :
        public pre_is_base_of<Base, Derived> { };
#else
    template<typename B, typename D>
    auto test_is_base_of(int) -> decltype(is_base_of_test_func<B>(static_cast<D*>(nullptr)));
    template<typename, typename>
    auto test_is_base_of(...) -> estd::true_type; // private or ambiguous base
#endif
}

template <typename Base, typename Derived>
struct is_base_of :
#if LEGACY
    public estd::conditional_t<
        estd::is_class<Base>::value && estd::is_class<Derived>::value,
        internal::pre_is_base_of2<Base, Derived>,
        estd::false_type
#else
    bool_constant<is_class<Base>::value && is_class<Derived>::value &&
    decltype(internal::test_is_base_of<Base, Derived>(0))::value
#endif
    > { };

#if __cplusplus >= 201703L
template <typename Base, typename Derived>
inline constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;
#endif

#else

// Shamelessly copied from
// https://code.woboq.org/gcc/libstdc++-v3/include/tr1/type_traits.html 

namespace internal {
template<typename _Base, typename _Derived>
struct __is_base_of_helper
{
    typedef typename remove_cv<_Base>::type    _NoCv_Base;
    typedef typename remove_cv<_Derived>::type _NoCv_Derived;
    static const bool __value = (is_same<_Base, _Derived>::value
                                || (__is_base_of(_Base, _Derived)
                                    && !is_same<_NoCv_Base,
                                                _NoCv_Derived>::value));
};
}

template<typename _Base, typename _Derived>
struct is_base_of
: public integral_constant<bool,
                            internal::__is_base_of_helper<_Base, _Derived>::__value>
{ };

#endif

}
