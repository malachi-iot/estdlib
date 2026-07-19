#pragma once

#include "../internal/platform.h"

#include "../internal/fpos.h"

#if FEATURE_STD_STRING
#include <string>
#endif

// FEATURE_STD_STRING: Does system even have std::char_traits?
// FEATURE_ESTD_CHARTRAITS: System may have std::char_traits, but we prefer ours

#ifndef FEATURE_ESTD_CHARTRAITS
//#define FEATURE_ESTD_CHARTRAITS 1
#endif

#include <stdint.h>

#include "../internal/string/len.h"
#include "../internal/raw/iosfwd.h"

namespace estd {

namespace internal {

template<class Char, class Int>
struct char_traits_base
{
    using char_type = Char;

    using int_type = Int;

    // DEBT: Use fpos instead
#if ESTD_MCU_ATMEL_AVR
    typedef int pos_type;
#else
    using pos_type = streampos;
#endif
    using off_type = streamoff;

    static constexpr char_type to_char_type(int_type ch) { return ch; }
    static constexpr int_type to_int_type(char_type ch) { return ch; }
    static constexpr int_type eof() { return -1; }
    static constexpr bool eq(char_type c1, char_type c2) { return c1 == c2; }
    static constexpr bool not_eof(int_type v) { return v != -1; }

    static const char_type* find(const char_type* p, size_t count, const char_type& ch)
    {
        while(count--)
        {
            if(*p == ch) return p;
            ++p;
        }

        return nullptr;
    }

    // DEBT: Almost certainly there are some platform-specific
    // optimizations available for this.  We may prefer to reach
    // out to standard strlen
    static constexpr size_t length(const char_type* s)
    {
        return estd::strlen(s);
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

// Nearly char_traits_byte.  However, when Char is uint8_t, obviously we
// don't need to static_cast - so this is char specific
template<class Char>
struct char_traits_char : char_traits_base<Char, int16_t>
{
    constexpr static int16_t to_int_type(Char ch)
    {
        return static_cast<uint8_t>(ch);
    }
};

// 19JUL26 MB DEBT: clearly we need wchar, char8, etc.


}

// Always available regardless of FEATURE_ESTD_CHARTRAITS configuration
namespace detail {

template<class Char>
struct char_traits : internal::char_traits_base<Char, int> {};

template<>
struct char_traits<char> : internal::char_traits_char<char> {};

template<>
struct char_traits<const char> : internal::char_traits_char<const char> {};

}

#if __ADSPBLACKFIN__
// Naughty ADI didn't make theirs compatible
// They omitted the template argument, so recreate it here
template <class TChar>
struct char_traits;

template <>
struct char_traits<char> : std::char_traits {};
#elif FEATURE_ESTD_CHARTRAITS || !defined(FEATURE_STD_STRING)
using detail::char_traits;
#elif defined(FEATURE_STD_STRING)
using std::char_traits;
#else
#error "Couldn't resolve char_traits"
#endif

}
