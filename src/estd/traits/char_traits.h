#pragma once

#include "../internal/platform.h"

// Only for scenarios where system has no std::char_traits to lean on
#ifndef FEATURE_STD_STRING

namespace std {

template<class CharT> class char_traits;

template<>
struct char_traits<char>
{
    typedef char char_type;

#ifdef __MBED__
    typedef int16_t int_type;
#else
    typedef int int_type;
#endif

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

    static CONSTEXPR size_t length(const char_type* s)
    {
        return strlen(s);
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
