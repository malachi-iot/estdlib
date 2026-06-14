#pragma once

#include "core.h"

namespace estd { namespace experimental {

// Stealing idea from GCC.
// https://github.com/gcc-mirror/gcc/blob/bd4b33d265d44bfb3222c7bb962818179242bf4b/libstdc%2B%2B-v3/include/std/variant
// Interesting, they too track trivial - but for different functional reasons probably
template <bool trivial, class ...Types>
union variadic_union;

template <class First, class ...Rest>
union variadic_union<false, First, Rest...>
{
};

template <bool active, class T>
struct variant_storage_leaf;

template <class T>
struct variant_storage_leaf<true, T>
{
    T value;
};

template <class T>
struct variant_storage_leaf<false, T>
{
};

template <size_t I_active, size_t I, class... Ts>
struct variant_storage {};

template<size_t I_active, size_t I, class Head, class... Tail>
struct variant_storage<I_active, I, Head, Tail...>
    : variant_storage_leaf<I == I_active, Head>
    , variant_storage<I_active, I+1, Tail...>
{};

}}

namespace estd { namespace internal {

// Catches both trivial and non-trivial so that our debug friendly specializations below
// have a fallback when things get too large
template <bool trivial, class ...T>
union variant_union
{
    estd::byte raw[sizeof(typename largest_type<T...>::type)];
};

template <>
union variant_union<true>
{

};

// variant_union exists to make debugging friendlier.  variant_storage *always* targets 'raw'.
// As one might guess, Debugger has a much easier time inspecting t1, t2 etc. than raw
// https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c183-dont-use-a-union-for-type-punning
// is not violated, though debugger may produce garbage results.
template <class T1>
union variant_union<true, T1>
{
    T1 t1;
    byte raw[0];
};


template <class T1, class T2>
union variant_union<true, T1, T2>
{
    T1 t1;
    T2 t2;
    byte raw[0];
};

template <class T1, class T2, class T3>
union variant_union<true, T1, T2, T3>
{
    T1 t1;
    T2 t2;
    T3 t3;
    byte raw[0];
};

template <class T1, class T2, class T3, class T4>
union variant_union<true, T1, T2, T3, T4>
{
    T1 t1;
    T2 t2;
    T3 t3;
    T4 t4;
    byte raw[0];
};

template <class T1, class T2, class T3, class T4, class T5>
union variant_union<true, T1, T2, T3, T4, T5>
{
    T1 t1;
    T2 t2;
    T3 t3;
    T4 t4;
    T5 t5;
    byte raw[0];
};

template <class T1, class T2, class T3, class T4, class T5, class T6>
union variant_union<true, T1, T2, T3, T4, T5, T6>
{
    T1 t1;
    T2 t2;
    T3 t3;
    T4 t4;
    T5 t5;
    T5 t6;
    byte raw[0];
};

}}
