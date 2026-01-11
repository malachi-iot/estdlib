#pragma once

//#include "../cctype.h"
#include "../locale/ctype.h"
#include "../locale/num_get.h"
#include "../raise_and_add.h"

#include "features.h"
#include "result.h"
#include "estd/system_error.h"

namespace estd { namespace internal {

// We need Cbase and runtime base, Cbase indicates whether we do alphanum or just num,
// while 'base' itself is actual calculated base
// DEBT: Either remove constexpr indication here, or rework to return a pair.  Also, raise_and_add
// would need a separate constexpr flavor.  Has non-trivial impact on non-constexpr optimization
// DEBT: Look into whether a constexpr/'base' helper is a worthwhile optimization
template<class Cbase, bool sto_mode = false, class T, class CharIt>
ESTD_CPP_CONSTEXPR(14) detail::from_chars_result<CharIt> from_chars_integer(CharIt first, CharIt last,
    T& value,
    unsigned short base)
{
    using cbase_type = Cbase;
    using char_type = typename cbase_type::char_type;
    using optional_type =  typename cbase_type::optional_type;
    using locale_type = typename cbase_type::locale_type;
    // DEBT: Grabbing char_type from cbase OK, but not perfect (probably want to deduce it
    // from locale/encoding instead)
    using ctype_type = estd::ctype<char_type, locale_type>;
    using result_type = detail::from_chars_result<CharIt>;

    // DEBT: Expand this to allow any numeric type, we'll have to make specialized
    // versions of raise_and_add to account for that
    static_assert(is_integral<T>::value, "T must be integral type");

    CharIt current = first;
    bool negate{};          // <= c++17 needs this initialized for constexpr
    // "If no characters match the pattern or if the value obtained by parsing
    // the matched characters is not representable in the type of value, value is unmodified," [1]
    T local_value = 0;

    // For sto mode (feeds stoi, stol, etc) spec calls for octal leading '0' parsing when base = 0.
    // A fallback to decimal is also expected.  Specifically:
    // "pattern identical to the one used by std::strtol" [1]
    // "If the value of base is 0, the numeric base is auto-detected: if the prefix is 0, [...]
    //  otherwise the base is decimal" https://en.cppreference.com/w/cpp/string/basic_string/stol.html

    // "leading whitespace is not ignored" [1]
    // Only eat whitespace in sto mode
    while(sto_mode && ctype_type::isspace(*current))
        ++current;

    // NOTE: A little odd, https://en.cppreference.com/w/cpp/string/byte/strtoul.html
    // indicates '-' is OK, but how could that be?
    if(estd::is_signed<T>::value)
    {
        negate = *current == '-';

        if (negate) ++current;
    }

    if(sto_mode && *current == '+')    ++current;

    if(sto_mode && *current == '0')
    {
        char_type c = *++current;

        if((base == 0 || base == 16) && (c == 'x' || c == 'X'))
        {
            base = 16;
            ++current;
        }
#if FEATURE_ESTD_FROM_CHARS_OCTAL
        else if(base == 0)
            base = 8;
#endif
    }

    while(current < last)
    {
        const optional_type digit = cbase_type::from_char(*current, base);

        //printf("PHASE 1: current=%c, local_value=%d base=%u digit.has_value=%u\n",
        //    *current, local_value, base, digit.has_value());

        if(digit.has_value())
        {
            bool success = raise_and_add(local_value, base, digit.value());

            // If we didn't succeed, that means we overflowed
            if(!success)
            {
                // skip to either end or next spot which isn't a number
                // "ptr points at the first character not matching the pattern." [1]
                while (++current != last && cbase_type::is_in_base(*current)) {}

                return result_type{current, estd::errc::result_out_of_range};
            }
        }
        // If first character isn't a digit, error out immediately
        else if(current == first)
        {
            return result_type{current, estd::errc::invalid_argument};
        }
        else
        {
            // If some character other than first character isn't a digit,
            // abort but succeed
            last = current;
        }
        ++current;
    }

    //printf("PHASE 2: current=%c, local_value=%d\n", *current, local_value);

    value = estd::is_signed<T>::value && negate ? -local_value : local_value;

    return result_type{last,estd::errc(0)};
}

// DEBT: Still needs 'std::chars_format' parameter
template <class CharIt, class Float>
ESTD_CPP_CONSTEXPR(14) detail::from_chars_result<CharIt> from_chars_float(
    CharIt first, CharIt last, Float& value)
{
    using char_type = typename iterator_traits<CharIt>::value_type;
    using type = num_get<char_type, CharIt, classic_locale_type>;
    ios_base::iostate iostate{};
    CharIt out = type::get(first, last, {}, iostate, value);

    // DEBT: Time for errc to become a true blue enum class I think
    return { out, iostate == 0 ? estd::errc{} : errc::invalid_argument };
}

}}
