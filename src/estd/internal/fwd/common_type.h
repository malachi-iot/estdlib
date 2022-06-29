#pragma once

namespace estd {
#ifdef FEATURE_CPP_VARIADIC

// lifted from https://en.cppreference.com/w/cpp/types/common_type

// primary template (used for zero types)
template <class ...T>
struct common_type
{
};

#endif
}