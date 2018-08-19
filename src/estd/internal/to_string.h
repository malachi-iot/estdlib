#pragma once

// string.h includes US, so this should never actually include anything,
// just using it for tooltip help
#include "../string.h"
#include "string_convert.h"

// not doing #include <stdio.h> because all its putc/putchar macros get things
// confused
#if defined(__GNU_LIBRARY__)
extern "C" int sprintf ( char * str, const char * format, ... ) __THROWNL;
#else
extern "C" int sprintf ( char * str, const char * format, ... );
#endif

namespace estd {

// non standard but non intrusive overload in case you've already got the string
// you'd like to populate
template <class T, class TStrImpl, size_t N = internal::maxStringLength<T>() >
inline void to_string(estd::internal::allocated_array<TStrImpl>& s, const T& value)
{
    internal::toString(s.lock(), value);
    s.unlock();
}

// NOTE: Counting on return value optimization to eliminate the copy of 's'
template <class T, size_t N = internal::maxStringLength<T>() >
inline layer1::string<N> to_string(const T& value)
{
    layer1::string<N> s;
    to_string(s, value);
    return s;
}


}
