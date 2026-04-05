#pragma once

#if FEATURE_ESTD_TYPE_TRAITS_ALIASED
#include <type_traits>

namespace estd {

using std::is_trivially_destructible;

}

#endif

namespace estd {

#if (UNIT_TESTING || FEATURE_ESTD_TYPE_TRAITS_ALIASED == false)

namespace detail {

#if __GNUC__

// See https://rocm.docs.amd.com/projects/llvm-project/en/docs-6.2.1/LLVM/clang/html/LanguageExtensions.html
// Copy/pasting straight from GCC type_traits
template <class T>
using is_trivially_destructible =
// __has_trivial_destructor works in all cases, but spews warnings
#if __clang__
    bool_constant<__is_trivially_destructible(T)>;
#else
    std::__and_<std::__is_destructible_safe<T>, bool_constant<__has_trivial_destructor(T)>>;
#endif

#endif

}
#endif

#if FEATURE_ESTD_TYPE_TRAITS_ALIASED == false
using detail::is_trivially_destructible;
#endif

}

