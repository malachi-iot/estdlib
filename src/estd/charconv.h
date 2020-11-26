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

#ifndef FEATURE_CPP_INITIALIZER_LIST
    from_chars_result(const char* ptr, estd::errc ec) :
        ptr(ptr), ec(ec) {}
#endif
};

struct to_chars_result {
    char* ptr;
    estd::errc  ec;

#ifndef __cpp_initializer_lists
    from_chars_result(const char* ptr, estd::errc ec) :
        ptr(ptr), ec(ec) {}
#endif
};

namespace legacy {
from_chars_result from_chars(const char* first, const char* last,
                             long& value, int base = 10);
}

namespace internal {

#ifdef __has_builtin
// adapted from GNUC
template<typename _Tp>
typename estd::enable_if<estd::is_signed<_Tp>::value, bool>::type
raise_and_add(_Tp& __val, const unsigned short __base, unsigned char __c)
{
    if (__builtin_mul_overflow(__val, __base, &__val)
        || __builtin_add_overflow(__val, __c, &__val))
        return false;
    return true;
}


template<typename _Tp>
typename estd::enable_if<!estd::is_signed<_Tp>::value, bool>::type
raise_and_add(_Tp& __val, const unsigned short __base, unsigned char __c)
{
    if (__builtin_umul_overflow(__val, __base, &__val)
        || __builtin_uadd_overflow(__val, __c, &__val))
        return false;
    return true;
}
#else
// DEBT: Only to get things to compile.  Overflow goes unnoticed
template <typename T>
bool raise_and_add(T& val, const unsigned short base, unsigned char c)
{
    val *= base;
    val += c;
    return true;
}
#endif

/// @brief Represents char-to-base-n conversion traits
/// @tparam b numeric base indicator
/// @tparam TEnable for internal use
template<unsigned short b, class TEnable = estd::internal::Range<true> >
struct char_base_traits;

struct char_base_traits_base
{
    typedef int8_t int_type;

    inline static int_type eol() { return -1; }
};

/// (Maybe) Requires ASCII
template<unsigned short b>
struct char_base_traits<b, estd::internal::Range<b <= 10> > :
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
        if (is_in_base(c, _base)) return from_char(c);

        return eol();
    }
};

/// (Maybe) Requires ASCII
template<unsigned short b>
struct char_base_traits<b, estd::internal::Range<(b > 10 && b <= 36)> > :
        char_base_traits_base
{
    static inline bool isupper(char c, const unsigned short _base = b)
    {
        return 'A' <= c && c <= ('A' + (_base - 11));
    }

    static inline bool islower(char c, const unsigned short _base = b)
    {
        return 'a' <= c && c <= ('a' + (_base - 11));
    }

    static inline unsigned base() { return b; }

    static inline bool is_in_base(char c, const unsigned short _base = b)
    {
        return estd::isdigit(c) ||
               isupper(c, _base) ||
               islower(c, _base);
    }

    static inline int_type from_char_with_test(char c, const unsigned short _base = b)
    {
        if (estd::isdigit(c)) return c - '0';

        if (isupper(c, _base)) return c - 'A' + 10;

        if (islower(c, _base)) return c - 'a' + 10;

        return eol();
    }

    static inline int_type from_char(char c)
    {
        if (c <= '9')
            return c - '0';
        else if (c <= 'Z')
            return c - 'A' + 10;
        else
            return c - 'a' + 10;
    }
};


template<class TCharBaseTraits, class T>
estd::from_chars_result from_chars_integer(const char* first, const char* last,
                                           T& value,
                                           const unsigned short base = TCharBaseTraits::base())
{
    typedef TCharBaseTraits traits;
    typedef typename traits::int_type int_type;
#ifdef __cpp_static_assert
    // DEBT: Expand this to allow any numeric type, we'll have to make specialized
    // versions of raise_and_add to account for that
    static_assert(estd::is_integral<T>::value, "T must be integral type");
#endif

    const char* current = first;
    bool negate;
    // "If no characters match the pattern or if the value obtained by parsing
    // the matched characters is not representable in the type of value, value is unmodified,"
    T local_value = 0;

    while (estd::isspace(*current))
        current++;

    if (estd::is_signed<T>::value)
    {
        negate = *current == '-';

        if (negate) current++;
    }

    while (current != last)
    {
        const int_type digit =
                traits::from_char_with_test(*current, base);
        if (digit != traits::eol())
        {
            bool success = raise_and_add(local_value, base, digit);
            if (!success)
            {
                // skip to either end or next spot which isn't a number
                while (current++ != last && traits::is_in_base(*current)) {}

#ifdef FEATURE_CPP_INITIALIZER_LIST
                return from_chars_result{current, estd::errc::result_out_of_range};
#else
                return from_chars_result(current, estd::errc::result_out_of_range);
#endif
            }
        }
        else
        {
            value = local_value;
#ifdef FEATURE_CPP_INITIALIZER_LIST
            return from_chars_result{current,
                 current==first ? estd::errc::invalid_argument : estd::errc(0)};
#else
            return from_chars_result(current,
                 current==first ? estd::errc::invalid_argument : estd::errc(0));
#endif
        }
        current++;
    }

    // prepend with constexpr so we can optimize out non-signed flavors
    if (estd::is_signed<T>::value && negate)
        local_value = -local_value;

    value = local_value;
#ifdef FEATURE_CPP_INITIALIZER_LIST
    return from_chars_result{last,estd::errc(0)};
#else
    return from_chars_result(last,estd::errc(0));
#endif
}

}

template <class TInt>
estd::from_chars_result from_chars(const char* first,
                                                const char* last,
                                                TInt& value,
                                                const int base = 10)
{
    if(base > 10)
        return internal::from_chars_integer<internal::char_base_traits<36> >(first, last, value, base);
    else
        return internal::from_chars_integer<internal::char_base_traits<10> >(first, last, value, base);
}

// TODO: Not done yet, need to handle > base 10 properly
template <class TInt>
to_chars_result to_chars(char* first, char* last, TInt value, const int base = 10)
{
    while(first != last)
    {
        *first = '0' + value % base;
        value /= base;

        first++;

        if(value == 0)
        {
#ifdef __cpp_initializer_lists
            return to_chars_result{first, estd::errc(0)};
#else
            return to_chars_result(first, estd::errc(0));
#endif
        }
    }

#ifdef __cpp_initializer_lists
    return to_chars_result{first, estd::errc::value_too_large};
#else
#endif
}

}