/**
 *
 * References:
 *
 * 1. https://en.cppreference.com/w/cpp/utility/from_chars
 * 2. https://en.cppreference.com/w/cpp/string/byte/strtol
 */
#pragma once

#include "charconv.h"
#include "raise_and_add.h"
#include "cctype.h"

#include "locale/ctype.h"
#include "locale/iterated/num_get.h"    // Very minimal dependencies

namespace estd { namespace internal {


template<class TCbase, class T>
estd::from_chars_result from_chars_integer(const char* first, const char* last,
                                           T& value,
                                           const unsigned short base = TCbase::base())
{
    typedef TCbase cbase_type;
    typedef typename cbase_type::optional_type optional_type;
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

    // DEBT: Spec calls for octal leading '0' parsing when base = 0.  A fallback to decimal
    // is also expected.  Specifically:
    // "pattern identical to the one used by std::strtol" [1]
    // "If the value of base is 0, the numeric base is auto-detected: if the prefix is 0, [...]
    //  otherwise the base is decimal" [2]

    /*
     * "leading whitespace is not ignored" [1]
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
        const optional_type digit = *cbase_type::from_char(*current, base);
        if (digit.has_value())
        {
            bool success = raise_and_add(local_value, base, digit.value());

            // If we didn't succeed, that means we overflowed
            if (!success)
            {
                // skip to either end or next spot which isn't a number
                // "ptr points at the first character not matching the pattern." [1]
                while (++current != last && cbase_type::is_in_base(*current)) {}

#ifdef FEATURE_CPP_INITIALIZER_LIST
                return from_chars_result{current, estd::errc::result_out_of_range};
#else
                return from_chars_result(current, estd::errc::result_out_of_range);
#endif
            }
        }
        else if(current == first)
        {
            return from_chars_result
#ifdef FEATURE_CPP_INITIALIZER_LIST
                {current, estd::errc::invalid_argument};
#else
                (current, estd::errc::invalid_argument);
#endif
        }
        else
        {
            value = local_value;
#ifdef FEATURE_CPP_INITIALIZER_LIST
            return from_chars_result{current, estd::errc(0)};
#else
            return from_chars_result(current, estd::errc(0));
#endif
        }
        ++current;
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
