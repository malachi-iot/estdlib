#pragma once

#include "../raw/type_traits.h"

namespace estd {

namespace internal {

template <class _Tp> struct __libcpp_is_void       : public false_type {};
template <>          struct __libcpp_is_void<void> : public true_type {};

}

template <class _Tp> struct is_void
    : public internal::__libcpp_is_void<typename remove_cv<_Tp>::type> {};


}