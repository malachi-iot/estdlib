#pragma once

#if FEATURE_ESTD_TYPE_TRAITS_ALIASED
#include <type_traits>

namespace estd {

template <class T>
using is_trivially_copyable = std::is_trivially_copyable<T>;

}
#else
namespace estd {

template<typename T, typename = void>
struct is_trivially_copyable : false_type {};

// DEBT: Very crude, only works with primitive/trivial types
template<typename T>
struct is_trivially_copyable<T, typename enable_if<is_trivial<T>::value>::type> :
    true_type {};

}
#endif