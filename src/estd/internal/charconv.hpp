#pragma once

#include "charconv.h"
#include "../algorithm.h"

namespace estd { namespace internal {

// AVR compiler appears to have incomplete implementation of __has_builtin
#if defined(__has_builtin) && !defined(__AVR__)
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

/// (Maybe) Requires ASCII
template<unsigned short b>
struct char_base_traits<b, estd::internal::Range<b <= 10> > :
        char_base_traits_base<encodings::UTF8>
{
    static inline CONSTEXPR unsigned base() { return b; }

    // adapted from GNUC
    static inline CONSTEXPR bool is_in_base(char_type c, const int _base = b)
    {
        return '0' <= c && c <= ('0' + (_base - 1));
    }

    static inline CONSTEXPR int_type from_char(char_type c)
    {
        return c - '0';
    }

    static inline CONSTEXPR char_type to_char(int_type v)
    {
        return '0' + v;
    }

    static inline CONSTEXPR int_type from_char_with_test(char_type c, const int _base = b)
    {
        return is_in_base(c, _base) ?
            from_char(c) :
            eol();
    }
};

/// (Maybe) Requires ASCII
template<unsigned short b>
struct char_base_traits<b, estd::internal::Range<(b > 10 && b <= 36)> > :
        char_base_traits_base<encodings::UTF8>
{
    static inline CONSTEXPR bool isupper(char c, const unsigned short _base = b)
    {
        return 'A' <= c && c <= ('A' + (_base - 11));
    }

    static inline CONSTEXPR bool islower(char c, const unsigned short _base = b)
    {
        return 'a' <= c && c <= ('a' + (_base - 11));
    }

    static inline CONSTEXPR unsigned base() { return b; }

    static inline CONSTEXPR bool is_in_base(char_type c, const unsigned short _base = b)
    {
        return estd::isdigit(c) ||
               isupper(c, _base) ||
               islower(c, _base);
    }

    static inline int_type from_char_with_test(char_type c, const unsigned short _base = b)
    {
        if (estd::isdigit(c)) return c - '0';

        if (isupper(c, _base)) return c - 'A' + 10;

        if (islower(c, _base)) return c - 'a' + 10;

        return eol();
    }

    static inline int_type from_char(char_type c)
    {
        if (c <= '9')
            return c - '0';
        else if (c <= 'Z')
            return c - 'A' + 10;
        else
            return c - 'a' + 10;
    }

    static inline CONSTEXPR char_type to_char(int_type v)
    {
        return v <= 10 ?
            ('0' + v) :
            'a' + (v - 10);
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

// DEBT: to_chars bounds checking here is quite weak

/// Performs to_chars slightly differently than stock.  returned 'ptr' is beginning
/// of converted integer rather than end, since the algorithm likes to do it that way
/// \tparam TCharBaseTraits
/// \tparam TInt
/// \param first
/// \param last
/// \param value
/// \param base
/// \return
template <class TCharBaseTraits, class TInt>
to_chars_result to_chars_integer_opt(char* first, char* last, TInt value, const int base)
{
    typedef TCharBaseTraits traits;
    typedef estd::numeric_limits<TInt> numeric_limits;
    const bool negative = numeric_limits::is_signed && value < 0;

    if(negative) value *= -1;

    while(first != last)
    {
        *last = traits::to_char(value % base);
        value /= base;

        if(value == 0)
        {
            if(numeric_limits::is_signed && negative)
                *--last = '-';

#ifdef __cpp_initializer_lists
            return to_chars_result{last, estd::errc(0)};
#else
            return to_chars_result(last, estd::errc(0));
#endif
        }

        last--;
    }

#ifdef __cpp_initializer_lists
    return to_chars_result{last, estd::errc::value_too_large};
#else
    return to_chars_result(last, estd::errc::value_too_large);
#endif
}

// This one operates exactly according to spec, but slightly slower than above
template <class TCharBaseTraits, class TInt>
to_chars_result to_chars_integer(char* first, char* last, TInt value, const int base)
{
    typedef TCharBaseTraits traits;
    typedef typename traits::char_type char_type;

    if(estd::numeric_limits<TInt>::is_signed && value < 0)
    {
        *first++ = '-';
        value *= -1;
    }

    char_type* current = first;

    while(current != last)
    {
        *current = traits::to_char(value % base);
        value /= base;

        current++;

        if(value == 0)
        {
            reverse(first, current);
#ifdef __cpp_initializer_lists
            return to_chars_result{current, estd::errc(0)};
#else
            return to_chars_result(current, estd::errc(0));
#endif
        }
    }

#ifdef __cpp_initializer_lists
    return to_chars_result{current, estd::errc::value_too_large};
#else
    return to_chars_result(current, estd::errc::value_too_large);
#endif
}


}}
