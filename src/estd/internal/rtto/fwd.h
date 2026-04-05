#pragma once

#include "../type_traits.h"

namespace estd { namespace internal {

template <class T, class IsDefaultConstructible = is_trivially_default_constructible<T>>
struct rtto_traits;

template <class T, class Traits = rtto_traits<T>>
struct rtto;

}}
