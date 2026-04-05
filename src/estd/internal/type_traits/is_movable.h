#pragma once

#include "features.h"
#include "../raw/type_traits.h"

#if FEATURE_ESTD_TYPE_TRAITS_ALIASED
#include <type_traits>

namespace estd {

using std::is_trivially_move_constructible;
using std::is_trivially_move_assignable;

}

#endif

#if (UNIT_TESTING || FEATURE_ESTD_TYPE_TRAITS_ALIASED == false)

#include "constructible.h"

namespace estd {

namespace detail {

template <class T>
using is_trivially_move_constructible = is_trivially_constructible<T, add_rvalue_reference_t<T>>;

}

#if FEATURE_ESTD_TYPE_TRAITS_ALIASED == false
using detail::is_tris_trivially_constructible;
#endif

}
#endif
