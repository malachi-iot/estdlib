/*
 * @file
 */
#pragma once

#include "cstdlib.h"
#include "internal/array.h"
#include "internal/legacy/array.h"

namespace estd {

template <class T, size_t N>
inline ESTD_CPP_CONSTEXPR(14) array<remove_cv_t<T>, N> to_array(T (&a)[N])
{
    array<remove_cv_t<T>, N> array;

    copy_n(a, N, array.begin());

    return array;
}


template <class T, size_t N>
inline ESTD_CPP_CONSTEXPR(14) array<remove_cv_t<T>, N> to_array(T (&&a)[N])
{
    array<remove_cv_t<T>, N> array;

    copy_n(a, N, array.begin());

    return array;
}


}
