#pragma once

#include "../feature/cpp.h"
#include "../feature/std.h"
#include "../macro/c++/const.h"

// DEBT: Combine or at least match convention to FEATURE_ESTD_STD_ALIAS
#ifndef FEATURE_ESTD_STD_NULLOPT_ALIAS
#define FEATURE_ESTD_STD_NULLOPT_ALIAS FEATURE_STD_OPTIONAL
#endif

#if FEATURE_ESTD_STD_NULLOPT_ALIAS
#include <optional>
#endif

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

#if FEATURE_ESTD_STD_NULLOPT_ALIAS
using std::nullopt_t;
using std::nullopt;
#else
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

// DEBT: Pretty sure this won't work with c++03.  If it does, document that
#ifdef FEATURE_CPP_INLINE_VARIABLES
inline
#elif defined(FEATURE_CPP_INLINE_STATIC)
static
#endif
constexpr nullopt_t nullopt{0};

#endif


}
