#pragma once

// string.h includes US, so this should never actually include anything,
// just using it for tooltip help
//#include "../string.h"
#include "string_convert.h"
#include "../charconv.h"
#include "impl/allocated_array.h"

// not doing #include <stdio.h> because all its putc/putchar macros get things
// confused
#if defined(__GNU_LIBRARY__)
extern "C" int sprintf ( char * str, const char * format, ... ) __THROWNL;
#else
// Oddly, some compilers use C++ linkage for their sprintf.  Feels like a bug, in which
// case we probably want to check a version range
#if !(defined(__MINGW32__) || defined(__MINGW64__))
extern "C"
#endif
int sprintf ( char * str, const char * format, ... );
#endif

namespace estd {

// non standard but non intrusive overloads in case you've already got the string
// you'd like to populate
template <class T, class TStrImpl>
typename estd::enable_if<estd::numeric_limits<T>::is_integer, void>::type
to_string(estd::internal::allocated_array<TStrImpl>& s, const T& value)
{
    typedef typename TStrImpl::allocator_type::value_type char_type;

    char_type* raw = s.lock();

    to_chars_result result = to_chars(raw, raw + s.max_size(), value);

    *result.ptr = 0;

    s.unlock();
}


// DEBT: This works pretty well, I'm just getting nervous about TInt letting too many things flow
// in - so hiding it in internal, which due to ADL won't help a whole lot

// helper since we often convert a statically allocated string
// NOTE: this will behave slightly differently than a regular string, see to_chars_opt
template <unsigned N, typename Int>
inline typename estd::enable_if<estd::numeric_limits<Int>::is_integer, to_chars_result>::type
    to_string_opt(char (&buffer)[N], Int value, unsigned base)
{
    // -1 here because to_chars doesn't care about null termination, but we do
    to_chars_result result = to_chars_opt(buffer, buffer + N - 1, value, base);

    // DEBT: Check result for conversion failure

    // remember, opt flavor specifies 'ptr' as beginning and we must manually
    // null terminate the end (ala standard to_chars operation)
    buffer[N - 1] = 0;

    return result;
}

#ifdef FEATURE_CPP_DEFAULT_TARGS
// NOTE: Counting on return value optimization to eliminate the copy of 's'
// We do + 1 because remember maxStringLength does not account for NULL termination
// and string class itself specifies buffer size in raw form, meaning if NULL termination
// is desired, be sure to include than in your size - (and, awkwardly, if no NULL termination
// is desired, then you get that extra spot and size is tracked in additional variables)
// That design decision exists so that your "64" byte allocations don't actually end up being "65"
// and causing unexpected alignment issues
// TODO: Move this explanation out to wiki
template <class T, size_t N = numeric_limits<T>::template length<10>::value + 1>
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
    static void to_string(estd::internal::allocated_array<TImpl>& s, int16_t value)
    {
        estd::internal::toString(s.lock(), value);
        s.unlock();
    }
};


}

}
