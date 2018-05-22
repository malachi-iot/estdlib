#pragma once

#include "platform.h"

// TODO: Utilize std version of this, if available

namespace estd {

//#if __cplusplus < 201200L
template<class T>
struct less
{
    CONSTEXPR bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs < rhs;
    }
};

template <class T>
struct greater
{
    CONSTEXPR bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs > rhs;
    }
};

/*
#else
template< class T = void >
struct less;
#endif
*/

}
