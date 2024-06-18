/**
 *
 * References:
 *
 * 1. https://en.cppreference.com/w/cpp/utility/from_chars
 * 2. https://en.cppreference.com/w/cpp/string/byte/strtol
 */
#pragma once

#include "locale/ctype.h"
#include "locale.h"
#include "cstdlib.h"

#include "charconv/base_provider.h"
#include "charconv/from_chars.hpp"
#include "charconv/to_chars.hpp"

//#include "locale/iterated/num_get.h"    // Very minimal dependencies

namespace estd { namespace internal {

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
/// \remarks
/// NOTE: Not using default template arg to maintain c++03 compatibility
/// DEBT: Strongly consider disallowing negative hex and oct renderings
/// DEBT: Move this out to charconv/to_chars.hpp
template <class Cbase, class Int, class CharIt, int base_>
inline detail::to_chars_result<CharIt> to_chars_integer_opt(
        CharIt first,
        CharIt last, Int value, base_provider<base_> base,
        Cbase cbase)
{
    typedef Cbase cbase_type;
    typedef detail::to_chars_result<CharIt> result_type;
    typedef estd::numeric_limits<Int> numeric_limits;
    const bool negative = numeric_limits::is_signed && value < 0;

#if __cpp_static_assert
    typedef typename iterator_traits<CharIt>::value_type char_type;
    static_assert(is_same<typename cbase_type::char_type, char_type>::value, "CharIt must align with cbase");
#endif

    if(numeric_limits::is_signed && negative) value *= -1;

    while(first != --last)
    {
// EXPERIMENTAL, not sure if this helps at all
#if FEATURE_ESTD_TO_CHARS_LIBC_DIV
        using d = div<Int>;
        const typename d::result v = d::eval(value, base());
        *last = cbase_type::to_char(v.rem);
        value = v.quot;
#else
        // DEBT: Pretty sure there's a div and mod combined operation, use that
        // i.e. https://riscv.org/wp-content/uploads/2017/05/riscv-spec-v2.2.pdf page 36 section 6.2
        *last = cbase.to_char(value % base());
        value /= base();
#endif

        if(value == 0)
        {
            if(numeric_limits::is_signed && negative)
                *--last = '-';

#ifdef __cpp_initializer_lists
            return result_type{last, estd::errc(0)};
#else
            return result_type(last, estd::errc(0));
#endif
        }
    }

#ifdef __cpp_initializer_lists
    return result_type{last, estd::errc::value_too_large};
#else
    return result_type(last, estd::errc::value_too_large);
#endif
}

// This one operates exactly according to spec, but slightly slower than above.
// Since simple algorithm likes to do things in reverse order, we reverse the whole
// string at the end
// NOTE: If opt is an underlying call to regular estd::to_chars, that one does
// a memcpy, so speed would be comparable
template <unsigned b, class TInt>
to_chars_result to_chars_integer_reverse(char* first, char* last, TInt value, const int base)
{
    typedef cbase<char, b, classic_locale_type> cbase_type;
    typedef typename cbase_type::char_type char_type;

    if(estd::numeric_limits<TInt>::is_signed && value < 0)
    {
        *first++ = '-';
        value *= -1;
    }

    char_type* current = first;

    while(current != last)
    {
        // DEBT: instance method in case its runtime casing
        *current = cbase_type().to_char(value % base);
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
