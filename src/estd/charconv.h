/*
 * @file
 */
#pragma once

#include "system_error.h"
#include "cctype.h"
#include "type_traits.h"
#include "internal/deduce_fixed_size.h"

namespace estd {

struct from_chars_result {
    const char* ptr;
    estd::errc  ec;

#if __cplusplus < 201103L
    from_chars_result(const char* ptr, estd::errc ec) :
        ptr(ptr), ec(ec) {}
#endif
};

from_chars_result from_chars(const char* first, const char* last,
                             long& value, int base = 10);

namespace experimental {

// lifted from GNUC
template<typename _Tp>
bool
raise_and_add(_Tp& __val, const unsigned __base, unsigned char __c)
{
    if (__builtin_mul_overflow(__val, __base, &__val)
        || __builtin_add_overflow(__val, __c, &__val))
        return false;
    return true;
}


/// @brief Represents char-to-base-n conversion traits
/// @tparam b numeric base indicator
/// @tparam TEnable for internal use
template <unsigned b, class TEnable = estd::internal::Range<true>>
struct char_base_traits;

struct char_base_traits_base
{
    typedef int8_t int_type;

    inline static int_type eol() { return -1; }
};

/// (Maybe) Requires ASCII
template <unsigned b>
struct char_base_traits<b, estd::internal::Range<b <= 10>> :
        char_base_traits_base
{
    static inline unsigned base() { return b; }

    // adapted from GNUC
    static inline bool is_in_base(char c, const int _base = b)
    {
        return '0' <= c && c <= ('0' + (_base - 1));
    }

    static inline int_type from_char(char c)
    {
        return c - '0';
    }

    static inline int_type from_char_with_test(char c, const int _base = b)
    {
        if(is_in_base(c, _base)) return from_char(c);

        return eol();
    }
};

/// (Maybe) Requires ASCII
template <unsigned b>
struct char_base_traits<b, estd::internal::Range<(b > 10 && b <= 26)>> :
        char_base_traits_base
{
    static inline bool isupper(char c, const int _base = b)
    {
        return 'A' <= c && c <= ('A' + (_base - 11));
    }

    static inline bool islower(char c, const int _base = b)
    {
        return 'a' <= c && c <= ('a' + (_base - 11));
    }

    static inline unsigned base() { return b; }
    static inline bool is_in_base(char c, const int _base = b)
    {
        return estd::isdigit(c) ||
               isupper(c, _base) ||
               islower(c, _base);
    }

    static inline int_type from_char_with_test(char c, const int _base = b)
    {
        if(estd::isdigit(c)) return c - '0';

        if(isupper(c, _base)) return c - 'A' + 10;

        if(islower(c, _base)) return c - 'a' + 10;

        return eol();
    }

    static inline int_type from_char(char c)
    {
        if(c <= '9')
            return c - '0';
        else if (c <= 'Z')
            return c - 'A' + 10;
        else
            return c - 'a' + 10;
    }
};



template <class TCharBaseTraits, class T>
estd::from_chars_result from_chars_integer(const char* first, const char* last,
                                           T& value,
                                           const int base = TCharBaseTraits::base())
{
    typedef TCharBaseTraits traits;
#ifdef __cpp_static_assert
    static_assert(estd::is_integral<T>::value, "implementation bug");
#endif

    const char* current = first;
    bool negate;

    estd::from_chars_result result { last, estd::errc(0) };

    while(estd::isspace(*current))
        current++;

    if(estd::is_signed<T>::value)
    {
        if(negate = (*current == '-'))
            current++;
    }

    while(current != last)
    {
        const typename traits::int_type digit =
                traits::from_char_with_test(*current, base);
        if(digit != traits::eol())
        {
            bool success = raise_and_add(value, base, digit);
            if (!success)
            {
                // skip to either end or next spot which isn't a number
                while(current++ != last && traits::is_in_base(*current)) {}

                result.ptr = current;
                result.ec = estd::errc::result_out_of_range;
                return result;
            }
        }
        else
        {
            result.ptr = current;
            return result;
        }
        current++;
    }

    // prepend with constexpr so we can optimize out non-signed flavors
    if(estd::is_signed<T>::value && negate)
        value = -value;

    return result;
}

template <class TInt>
estd::from_chars_result from_chars(const char* first,
                                                const char* last,
                                                TInt& value,
                                                const int base = 10)
{
    if(base > 10)
        return from_chars_integer<char_base_traits<36>>(first, last, value, base);
    else
        return from_chars_integer<char_base_traits<10>>(first, last, value, base);
}

}

}