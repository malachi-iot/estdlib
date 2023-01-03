/*
 * @file
 */
#pragma once

#include <stdlib.h>
#include "internal/array.h"
#include "internal/legacy/array.h"

namespace estd {


template <class T, size_t N>
inline array<typename estd::remove_cv<T>::type, N> to_array(T (&a)[N])
{
    array<typename estd::remove_cv<T>::type, N> array;

    estd::copy_n(a, N, array.begin());

    return array;
}


}
