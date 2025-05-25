#pragma once

#include "../feature/cpp.h"
#include "../macro/c++/const.h"

namespace estd {

namespace internal {

template <class T>
struct optional_base;

// EXPERIMENTAL
// I don't love that it's encouraged to have '0' as default null value integer, so
// playing with this.
template <class T, class enabled = void>
struct optional_default_value;

}

template <class T, class Base = internal::optional_base<T> >
class optional;

struct nullopt_t
{
    explicit ESTD_CPP_CONSTEVAL nullopt_t(int) {}

#if !defined(FEATURE_CPP_INLINE_VARIABLES) && !defined(FEATURE_CPP_INLINE_STATIC)
    /// \brief in the case where we can't easily make a global 'nullopt',
    /// make a provision for more easily creating a nullopt_t on the fly
    ///
    constexpr nullopt_t() {}
#endif
};


}
