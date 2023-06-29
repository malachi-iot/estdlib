#pragma once

#include "../platform.h"

#if !FEATURE_STD_UTILITY && __cpp_rvalue_references
// Normally we try to avoid direct substituting std namespace, but in circumstances
// where std namespace is completely absent,
// oft-used conventions need to be spliced in

namespace std {

// more or less copy/pasted from GNU reference

/**
 *  @brief  Forward an lvalue.
 *  @return The parameter cast to the specified type.
 *
 *  This function is used to implement "perfect forwarding".
 */
template<typename _Tp>
constexpr _Tp&&
forward(typename estd::remove_reference<_Tp>::type& __t) noexcept
{ return static_cast<_Tp&&>(__t); }


/**
   *  @brief  Forward an rvalue.
   *  @return The parameter cast to the specified type.
   *
   *  This function is used to implement "perfect forwarding".
   */
template<typename _Tp>
constexpr _Tp&&
forward(typename estd::remove_reference<_Tp>::type&& __t) noexcept
{
    static_assert(!estd::is_lvalue_reference<_Tp>::value, "template argument"
        " substituting _Tp is an lvalue reference type");
    return static_cast<_Tp&&>(__t);
}

/**
   *  @brief  Convert a value to an rvalue.
   *  @param  __t  A thing of arbitrary type.
   *  @return The parameter cast to an rvalue-reference to allow moving it.
  */
  template<typename _Tp>
    constexpr typename estd::remove_reference<_Tp>::type&&
    move(_Tp&& __t) noexcept
    { return static_cast<typename estd::remove_reference<_Tp>::type&&>(__t); }

}
#endif