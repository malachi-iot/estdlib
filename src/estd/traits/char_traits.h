#pragma once

#include "../internal/platform.h"

#include "../internal/fpos.h"
//#include "../iosfwd.h"

// FEATURE_STD_STRING: Scenarios where system has no std::char_traits
// FEATURE_ESTD_CHARTRAITS: System has std::char_traits, but we prefer ours
#if !defined(FEATURE_STD_STRING) || FEATURE_ESTD_CHARTRAITS

#include <stdint.h>

namespace estd {

template <class TChar>
struct char_traits;

template<>
struct char_traits<char>
{
    typedef char char_type;

    typedef int16_t int_type;

    // DEBT: Use fpos instead
#if ESTD_MCU_ATMEL_AVR
    typedef int pos_type;
#else
    typedef streampos pos_type;
#endif
    typedef int off_type;

    static ESTD_CPP_CONSTEXPR_RET char_type to_char_type(int_type ch) { return ch; }
    static ESTD_CPP_CONSTEXPR_RET int_type to_int_type(const char ch) { return ch; }
    static ESTD_CPP_CONSTEXPR_RET int_type eof() { return -1; }
    static ESTD_CPP_CONSTEXPR_RET bool eq(char c1, char c2) { return c1 == c2; }
    static ESTD_CPP_CONSTEXPR_RET bool not_eof(int_type v) { return v != -1; }

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

    static
#if __cpp_constexpr > 201304L
        constexpr
#endif
        int compare(const char_type* s1, const char_type* s2, size_t count)
    {
        for(;count != 0; --count, ++s1, ++s2)
        {
            if(*s1 < *s2) return -1;
            if(*s1 > *s2) return 1;
        }

        return 0;
    }

#ifdef FEATURE_IOS_EXPERIMENTAL_TRAIT_NODATA
    // Non-standard timeout/data unavailable return value, since eof() suggests no more data EVER
    // we want a different error code
    static CONSTEXPR int_type nodata() { return -2; }
#endif
};

// DEBT: std spec doesn't indicate we can do this - may have to "deconst" all
// our char_traits usages
template<>
struct char_traits<const char> : char_traits<char> {};


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
#elif defined(FEATURE_STD_STRING) && FEATURE_ESTD_CHARTRAITS == 0
using std::char_traits;
#endif

}
