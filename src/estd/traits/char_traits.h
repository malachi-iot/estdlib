#pragma once

#include "../internal/platform.h"

#include "../internal/fpos.h"
#include "../iosfwd.h"

#include <stdint.h>

// FEATURE_STD_STRING: Scenarios where system has no std::char_traits
// FEATURE_ESTD_CHARTRAITS: System has std::char_traits, but we prefer ours
#if !defined(FEATURE_STD_STRING) || FEATURE_ESTD_CHARTRAITS

namespace estd {

template<>
struct char_traits<char>
{
    typedef char char_type;

    typedef int16_t int_type;

    // DEBT: Use fpos instead
    typedef streampos pos_type;
    typedef int off_type;

    static CONSTEXPR char_type to_char_type(int_type ch) { return ch; }
    static CONSTEXPR int_type to_int_type(const char ch) { return ch; }
    static CONSTEXPR int_type eof() { return -1; }
    static CONSTEXPR bool eq(char c1, char c2) { return c1 == c2; }
    static const char_type* find(const char_type* p, size_t count, const char_type& ch)
    {
        while(count--)
        {
            if(*p == ch) return p;
            p++;
        }

        return NULLPTR;
    }

    // DEBT: Almost certainly there are some platform-specific
    // optimizations available for this.  We may prefer to reach
    // out to standard strlen
    static
#if __cpp_constexpr > 201304L
        constexpr
#endif
        size_t length(const char_type* s)
    {
        const char_type* i = s;

        for(; *i != 0; ++i);

        return i - s;
    }

#ifdef FEATURE_IOS_EXPERIMENTAL_TRAIT_NODATA
    // Non-standard timeout/data unavailable return value, since eof() suggests no more data EVER
    // we want a different error code
    static CONSTEXPR int_type nodata() { return -2; }
#endif
};

}
#else
#include <string>
#endif

namespace estd {

#if __ADSPBLACKFIN__
// Naughty ADI didn't make theirs compatible
// They omitted the template argument, so recreate it here
template <class TChar>
struct char_traits;

template <>
struct char_traits<char> : std::char_traits {};
#elif FEATURE_ESTD_CHARTRAITS == 0
using std::char_traits;
#endif

}
