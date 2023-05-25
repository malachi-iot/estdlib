/*
 *  @file
 *
 * Portion of type traits with no dependencies (except internal/platform.h)
 * TODO: Document why this is needed so that we can structure raw vs internal properly.
 * Until we do that, leave this in a half-refactored state so that we don't cause dependency
 * issues accidentally
 */

#pragma once

#include "platform.h"
#include "raw/type_traits.h"
#include "../port/type_traits.h"
#include "is_convertible.h"

#if FEATURE_STD_TYPE_TRAITS
#include <type_traits>

// DEBT: Copy/paste inline version of this from LLVM, GNU, etc.
#if __cpp_alias_templates
template <class T, class ...TArgs>
using is_constructible = std::is_constructible<T, TArgs...>;
#endif

#endif

// Semi-hack to get arduino/Print.h to auto include more smoothly
// Only a little kludgey
#ifndef ESTD_INTERNAL_TYPE_TRAITS_ONLINE
#define ESTD_INTERNAL_TYPE_TRAITS_ONLINE
#endif

#include "../port/support_platform.h"
