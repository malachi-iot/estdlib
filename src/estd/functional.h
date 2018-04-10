#pragma once

// TODO: Utilize std version of this, if available

namespace estd {

#if __cplusplus < 201200L
template<class T>
struct less;
#else
template< class T = void >
struct less;
#endif

}
