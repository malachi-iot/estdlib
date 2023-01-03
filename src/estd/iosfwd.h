#pragma once

#include "internal/iosfwd.h"
#include "traits/char_traits.h"

namespace estd {

template <class TState>
class fpos;

// TODO: use specific 16/32/64 bit versions depending on architecture
typedef int streampos;
typedef int streamoff;
typedef int streamsize;

template<class TStreambuf>
class istreambuf_iterator;


}