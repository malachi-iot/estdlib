#pragma once

#include "features.h"

#if FEATURE_ESTD_TYPE_TRAITS_ALIASED
#include <type_traits>

namespace estd {

template <typename T>
using underlying_type = std::underlying_type<T>;

template <typename T>
using is_trivial = std::is_trivial<T>;

#define FEATURE_ESTD_UNDERLYING_TYPE 1
#define FEATURE_ESTD_IS_TRIVIAL 1

}
#endif
