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

#ifdef FEATURE_CPP_DEFAULT_TARGS
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
#endif

namespace experimental {

template <>
struct string_convert_traits<char, int16_t>
{
    static CONSTEXPR uint8_t max_size() { return 5; }

    template <class TImpl>
    static void to_string(internal::allocated_array<TImpl>& s, int16_t value)
    {
        internal::toString(s.lock(), value);
        s.unlock();
    }
};


}

}
