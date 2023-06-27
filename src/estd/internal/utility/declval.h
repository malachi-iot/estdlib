#pragma once

#include "../platform.h"

#if !FEATURE_STD_UTILITY && __cpp_rvalue_references && defined(FEATURE_CPP_DECLTYPE)
namespace std {
// more or less copy/pasted from GNU reference
// DEBT: __declval seems like a GNU extension, so put guards around the following 

 /**
   *  @brief  Utility to simplify expressions used in unevaluated operands
   *  @ingroup utilities
   */

template<typename _Tp, typename _Up = _Tp&&>
    _Up
    __declval(int);

template<typename _Tp>
    _Tp
    __declval(long);

template<typename _Tp>
    auto declval() noexcept -> decltype(__declval<_Tp>(0));
}

#endif