#pragma once

namespace estd {

#if FEATURE_ESTD_CHARTRAITS
template <class TChar>
class char_traits;
#endif

template <class TState>
class fpos;

typedef int streampos;

}