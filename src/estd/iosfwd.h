#pragma once

namespace estd {

#if FEATURE_ESTD_CHARTRAITS
template <class TChar>
class char_traits;
#endif

template <class TState>
class fpos;

// TODO: use specific 16/32/64 bit versions depending on architecture
typedef int streampos;
typedef int streamoff;
typedef int streamsize;

}