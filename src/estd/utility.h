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
#include "internal/utility/sequence.h"
#include "internal/utility.h"
#if FEATURE_ESTD_CPP03_TUPLE == 0
#include "internal/fwd/tuple.h"
#include "internal/tuple/make.h"
#endif

#include "cstddef.h"

namespace estd {

struct piecewise_construct_t {};

template<class T1, class T2>
struct pair
{
    T1 first;
    T2 second;

    typedef T1 first_type;
    typedef T2 second_type;

    pair() = default;
    pair(const pair&) = default;
    pair(pair&&) = default;

    constexpr pair(const T1& first, const T2& second) :
        first(first), second(second) {}

    template <class U1, class U2>
    constexpr pair(U1&& first, U2&& second) : first(first), second(second) {}

#if FEATURE_ESTD_CPP03_TUPLE == 0
    template <class ...Args1, class ...Args2>
    constexpr pair(piecewise_construct_t,
        tuple<Args1...> first_args,
        tuple<Args2...> second_args) :
        // DEBT: Feels like we can avoid an implied && move here
        first{make_from_tuple<T1>(std::move(first_args))},
        second{make_from_tuple<T2>(std::move(second_args))}
    {
    }
#endif

    pair& operator=(const pair& other) = default;
};

template <class T1, class T2>
ESTD_CPP_CONSTEXPR(11) pair<T1, T2> make_pair(T1 t, T2 u)
{
    return pair<T1, T2> { t, u };
}


template <class T1, class T2>
ESTD_CPP_CONSTEXPR(11) bool operator==(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
{
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

template <class T1, class T2>
ESTD_CPP_CONSTEXPR(11) bool operator!=(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
{
    return lhs.first != rhs.first || lhs.second != rhs.second;
}

template <class T1, class T2>
ESTD_CPP_CONSTEXPR(11) bool operator<(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
{
    return
        lhs.first < rhs.first ? true :
        lhs.first > rhs.first ? false :
        lhs.second < rhs.second;
}

// NOT TESTED
template <class T1, class T2>
ESTD_CPP_CONSTEXPR(11) bool operator>(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
{
    return
        lhs.first > rhs.first ? true :
        lhs.first < rhs.first ? false :
        lhs.second > rhs.second;
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
