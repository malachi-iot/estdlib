#pragma once

// can't include this here as it would cause a circular loop
// type_traits.h -> internal/common_type.h -> utility.h and
// given the specific way type_traits includes it #pragma once
// doesn't help us
//#include "type_traits.h"
#include "cstddef.h"

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

#ifdef FEATURE_CPP_VARIADIC
// adapted from https://gist.github.com/ntessore/dc17769676fb3c6daa1f
template<typename T, T... Ints>
struct integer_sequence
{
    typedef T value_type;
    static constexpr std::size_t size() { return sizeof...(Ints); }
};

template<std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

template<typename T, std::size_t N, T... Is>
struct make_integer_sequence : make_integer_sequence<T, N-1, N-1, Is...> {};

template<typename T, T... Is>
struct make_integer_sequence<T, 0, Is...> : integer_sequence<T, Is...> {};

template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template<typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

#endif

}

#if defined(FEATURE_STD_UTILITY)
// Utilize stock-standard std version of this if it's available
#include <utility>
#elif defined(FEATURE_CPP_MOVESEMANTIC) && defined(FEATURE_CPP_DECLTYPE)
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


// more or less copy/pasted from GNU reference

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

namespace estd {

template<class T, class U = T>
T exchange(T& obj, U&& new_value)
{
    T old_value = std::move(obj);
    obj = std::forward<U>(new_value);
    return old_value;
}

}
