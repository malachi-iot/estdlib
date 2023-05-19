#pragma once

#include "../../internal/platform.h"
#include "../../internal/raw/type_traits.h"

namespace estd {

#define FEATURE_ESTD_IS_EMPTY 1
template<typename _Tp>
struct is_empty
    : public integral_constant<bool, __is_empty(_Tp)>
{};

}
