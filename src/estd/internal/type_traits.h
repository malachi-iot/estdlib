/*
 *  @file
 *
 * Portion of type traits with fewer dependencies (except internal/platform.h)
 */

#pragma once

#include "platform.h"
#include "feature/type_traits.h"
#include "raw/type_traits.h"
#include "../port/type_traits.h"
#include "type_traits/is_assignable.h"
#include "type_traits/is_base_of.h"
#include "type_traits/is_copyable.h"
#include "type_traits/constructible.h"
#include "type_traits/is_destructible.h"
#include "type_traits/is_empty.h"
#include "type_traits/is_function.h"
#include "type_traits/is_convertible.h"
#include "type_traits/is_member.h"
#include "type_traits/is_movable.h"
#include "type_traits/make_unsigned.h"
#include "type_traits/is_trivial.h"


#include "../port/support_platform.h"

namespace estd {

// DEBT: Location for this not ideal.  Needs to come after port/type_traits and
// type_traits/is_trivial
#if FEATURE_ESTD_UNDERLYING_TYPE
template <class T>
using underlying_type_t = typename underlying_type<T>::type;
#endif

}
