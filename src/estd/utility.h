#pragma once

#include "type_traits.h"

// TODO: Utilize stock-standard std version of this if it's available

namespace estd {

template<
    class T1,
    class T2
> struct pair
{
    T1 first;
    T2 second;

    typedef T1 first_type;
    typedef T2 second_type;

    //pair(T1& first, T2& second) : first(first), second(second) {}
};

/*
template<class T>
typename estd::add_rvalue_reference<T>::type declval() */

}

// Normally we try to avoid direct substituting std namespace, but in circumstances
// where std namespace is completely absent,
// oft-used conventions need to be spliced in
#if !defined(FEATURE_STD_UTILITY)
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

}


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
    static_assert(!std::is_lvalue_reference<_Tp>::value, "template argument"
        " substituting _Tp is an lvalue reference type");
    return static_cast<_Tp&&>(__t);
}

/**
   *  @brief  Convert a value to an rvalue.
   *  @param  __t  A thing of arbitrary type.
   *  @return The parameter cast to an rvalue-reference to allow moving it.
  */
  template<typename _Tp>
    constexpr typename std::remove_reference<_Tp>::type&&
    move(_Tp&& __t) noexcept
    { return static_cast<typename std::remove_reference<_Tp>::type&&>(__t); }

#endif
