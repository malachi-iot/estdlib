/**
 *
 * References:
 *
 * 1. https://en.cppreference.com/w/cpp/utility/from_chars
 */
#pragma once

#include "charconv.h"
#include "../algorithm.h"
#include "cctype.h"

#include "locale/ctype.h"

namespace estd { namespace internal {

// AVR compiler appears to have incomplete implementation of __has_builtin
// DEBT: According to https://stackoverflow.com/questions/3944505/detecting-signed-overflow-in-c-c
// contents of the result (&__val in this case) are undefined on overflow.  That might be an
// issue down the line if we're depending on what we've gathered so far.
#if defined(__has_builtin) && !defined(__AVR__)
// adapted from GNUC
template<typename _Tp>
typename estd::enable_if<estd::is_signed<_Tp>::value, bool>::type
inline raise_and_add(_Tp& __val, const unsigned short __base, unsigned char __c)
{
    if (__builtin_mul_overflow(__val, __base, &__val)
        || __builtin_add_overflow(__val, __c, &__val))
        return false;
    return true;
}


template<typename _Tp>
typename estd::enable_if<!estd::is_signed<_Tp>::value, bool>::type
inline raise_and_add(_Tp& __val, const unsigned short __base, unsigned char __c)
{
    if (__builtin_umul_overflow(__val, __base, &__val)
        || __builtin_uadd_overflow(__val, __c, &__val))
        return false;
    return true;
}
#else
// DEBT: Only to get things to compile.  Overflow goes unnoticed
template <typename T>
inline bool raise_and_add(T& val, const unsigned short base, unsigned char c)
{
    val *= base;
    val += c;
    return true;
}
#endif


template<class TCbase, class T>
estd::from_chars_result from_chars_integer(const char* first, const char* last,
                                           T& value,
                                           const unsigned short base = TCbase::base())
{
    typedef TCbase cbase_type;
    typedef typename cbase_type::int_type int_type;
#ifdef __cpp_static_assert
    // DEBT: Expand this to allow any numeric type, we'll have to make specialized
    // versions of raise_and_add to account for that
    static_assert(estd::is_integral<T>::value, "T must be integral type");
#endif

    const char* current = first;
    bool negate;
    // "If no characters match the pattern or if the value obtained by parsing
    // the matched characters is not representable in the type of value, value is unmodified," [1]
    T local_value = 0;

    /*
     * "leading whitespace is not ignored"
    while (estd::internal::ascii_isspace(*current))
        current++; */

    if (estd::is_signed<T>::value)
    {
        negate = *current == '-';

        if (negate) ++current;
    }

    while (current != last)
    {
        // DEBT: Use has_value() and friends for clarity here
        const int_type digit =
                *cbase_type::from_char(*current, base);
        if (digit != cbase_type::eol())
        {
            bool success = raise_and_add(local_value, base, digit);

            // If we didn't succeed, that means we overflowed
            if (!success)
            {
                // skip to either end or next spot which isn't a number
                // "ptr points at the first character not matching the pattern." [1]
                while (current++ != last && cbase_type::is_in_base(*current)) {}

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
template <class TCbase, class TInt>
to_chars_result to_chars_integer_opt(char* first, char* last, TInt value, const int base)
{
    typedef TCbase cbase_type;
    typedef estd::numeric_limits<TInt> numeric_limits;
    const bool negative = numeric_limits::is_signed && value < 0;

    if(negative) value *= -1;

    while(first != last)
    {
        *last = cbase_type::to_char(value % base);
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
template <class TCbase, class TInt>
to_chars_result to_chars_integer(char* first, char* last, TInt value, const int base)
{
    typedef TCbase cbase_type;
    typedef typename cbase_type::char_type char_type;

    if(estd::numeric_limits<TInt>::is_signed && value < 0)
    {
        *first++ = '-';
        value *= -1;
    }

    char_type* current = first;

    while(current != last)
    {
        *current = cbase_type::to_char(value % base);
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
