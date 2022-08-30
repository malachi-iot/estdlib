#pragma once

#include "../platform.h"

namespace estd {
#ifdef FEATURE_CPP_VARIADIC

// lifted from https://en.cppreference.com/w/cpp/types/common_type

// primary template (used for zero types)
template <class ...T>
struct common_type
{
};

#else

// NOTE: C++98 mode currently only implemented for chrono specialization

template <class T1 = void, class T2 = void, class T3 = void, class T4 = void>
struct common_type;


#endif
}