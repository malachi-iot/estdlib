#pragma once

#include "features.h"

#if FEATURE_ESTD_TYPE_TRAITS_ALIASED
#include <type_traits>

namespace estd {

template <typename T>
using underlying_type = std::underlying_type<T>;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

template <typename T>
using is_trivial
#if __cplusplus >= 201402L
// 05MAR26 Keep deprecated commented as we slowly phase out our own usage of is_trivial
// as per https://github.com/malachi-iot/estdlib/issues/171 (turning it on creates
// hundreds of warnings right now)
//[[deprecated]]
#endif
= std::is_trivial<T>;

#pragma GCC diagnostic pop

#define FEATURE_ESTD_UNDERLYING_TYPE 1
#define FEATURE_ESTD_IS_TRIVIAL 1

}
#endif
