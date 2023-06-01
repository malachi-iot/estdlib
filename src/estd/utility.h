#pragma once

// can't include this here as it would cause a circular loop
// type_traits.h -> internal/common_type.h -> utility.h and
// given the specific way type_traits includes it #pragma once
// doesn't help us
//#include "type_traits.h"
// internal type_traits has no dependencies, so that's safe
#include "internal/fwd/tuple-shared.h"
#include "internal/type_traits.h"
#include "internal/variadic/integer_sequence.h"

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

#if __cplusplus >= 201103L
    constexpr pair() : first(), second() {}

    pair(const T1& first, const T2& second) : first(first), second(second) {}

    template <class U1, class U2>
    constexpr pair(U1&& first, U2&& second) : first(first), second(second) {}
#else
    pair() : first(), second() {}
#endif
};

template <class T1, class T2>
#ifdef FEATURE_CPP_CONSTEXPR
constexpr
#endif
pair<T1, T2> make_pair(T1 t, T2 u)
{
#ifdef FEATURE_CPP_INITIALIZER_LIST
    return pair<T1, T2> { t, u };
#else
    return pair<T1, T2>(t, u);
#endif
}

template <class T1, class T2>
struct tuple_size<pair<T1, T2> > : integral_constant<std::size_t, 2> {};

namespace internal {

template <std::size_t index, class T1, class T2>
struct GetImplPair;

// Semi-experimental
#ifdef FEATURE_CPP_CONSTEXPR
#define ESTD_CPP_CONSTEXPR_FUNCTION constexpr
#else
#define ESTD_CPP_CONSTEXPR_FUNCTION inline
#endif

template <class T1, class T2>
struct GetImplPair<0, T1, T2>
{
    static ESTD_CPP_CONSTEXPR_FUNCTION T1& value(pair<T1, T2>& p) { return p.first; }
    static ESTD_CPP_CONSTEXPR_FUNCTION const T1& value(const pair<T1, T2>& p) { return p.first; }

    typedef T1 type;
};

template <class T1, class T2>
struct GetImplPair<1, T1, T2>
{
    static ESTD_CPP_CONSTEXPR_FUNCTION T2& value(pair<T1, T2>& p) { return p.second; }
    static ESTD_CPP_CONSTEXPR_FUNCTION const T2& value(const pair<T1, T2>& p) { return p.second; }

    typedef T2 type;
};


};


template <std::size_t I, class T1, class T2>
struct tuple_element<I, pair<T1, T2> > :
    internal::GetImplPair<I, T1, T2>
{
};

template <std::size_t index, class T1, class T2>
ESTD_CPP_CONSTEXPR_FUNCTION
typename tuple_element<index, pair<T1, T2> >::type& get(pair<T1, T2>& p)
{
    return internal::GetImplPair<index, T1, T2>::value(p);
}

template <std::size_t index, class T1, class T2>
const ESTD_CPP_CONSTEXPR_FUNCTION
typename tuple_element<index, pair<T1, T2> >::type& get(const pair<T1, T2>& p)
{
    return internal::GetImplPair<index, T1, T2>::value(p);
}

/*
template<class T>
typename estd::add_rvalue_reference<T>::type declval() */

#ifdef FEATURE_CPP_VARIADIC
// adapted from https://gist.github.com/ntessore/dc17769676fb3c6daa1f
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

// DEBT: Try to move this 'std' portion to an internal or other reduced-dependency
// area
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

namespace estd {

#ifdef FEATURE_CPP_MOVESEMANTIC
template<class T, class U = T>
T exchange(T& obj, U&& new_value)
{
    T old_value = std::move(obj);
    obj = std::forward<U>(new_value);
    return old_value;
}
#endif

template <class T>
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
constexpr
#endif
void swap(T& a, T& b)
{
    // NOTE: perhaps we should use move/forward here...?  so that
    // objects with underlying self-referencing pointers can be managed?
    const T temp = a;

    a = b;
    b = temp;
}

namespace internal {

template <class T>
inline void xor_swap(T& a, T& b)
{
    a ^= b;
    b ^= a;
    a ^= b;
}

}

template<>
inline void swap(char& a, char& b) { internal::xor_swap(a, b); }

template<>
inline void swap(short& a, short& b) { internal::xor_swap(a, b); }

template<>
inline void swap(int& a, int& b) { internal::xor_swap(a, b); }

template<>
inline void swap(long& a, long& b) { internal::xor_swap(a, b); }

template<>
inline void swap(unsigned char& a, unsigned char& b) { internal::xor_swap(a, b); }

template<>
inline void swap(unsigned short& a, unsigned short& b) { internal::xor_swap(a, b); }

template<>
inline void swap(unsigned int& a, unsigned int& b) { internal::xor_swap(a, b); }

template<>
inline void swap(unsigned long& a, unsigned long& b) { internal::xor_swap(a, b); }

}
