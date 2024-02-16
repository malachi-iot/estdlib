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
#include "locale.h"
#include "cstdlib.h"

#include "charconv/from_chars.hpp"
#include "charconv/to_chars.hpp"

//#include "locale/iterated/num_get.h"    // Very minimal dependencies

namespace estd { namespace internal {

template<class Cbase, class T, class CharIt>
detail::from_chars_result<CharIt> from_chars_integer(CharIt first, CharIt last,
                                           T& value,
                                           const unsigned short base = Cbase::base())
{
    typedef Cbase cbase_type;
    typedef typename cbase_type::optional_type optional_type;
    typedef detail::from_chars_result<CharIt> result_type;

#ifdef __cpp_static_assert
    // DEBT: Expand this to allow any numeric type, we'll have to make specialized
    // versions of raise_and_add to account for that
    static_assert(estd::is_integral<T>::value, "T must be integral type");
#endif

    CharIt current = first;
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
        const optional_type digit = cbase_type::from_char(*current, base);
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
                return result_type{current, estd::errc::result_out_of_range};
#else
                return result_type(current, estd::errc::result_out_of_range);
#endif
            }
        }
        else if(current == first)
        {
            return result_type
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
            return result_type{current, estd::errc(0)};
#else
            return result_type(current, estd::errc(0));
#endif
        }
        ++current;
    }

    // prepend with constexpr so we can optimize out non-signed flavors
    if (estd::is_signed<T>::value && negate)
        local_value = -local_value;

    value = local_value;
#ifdef FEATURE_CPP_INITIALIZER_LIST
    return result_type{last,estd::errc(0)};
#else
    return result_type(last,estd::errc(0));
#endif
}

// DEBT: to_chars bounds checking here is quite weak

template <int base_ = -1>
struct base_provider
{
    static CONSTEXPR unsigned base = (unsigned) base_;

    ESTD_CPP_CONSTEXPR_RET unsigned operator()() const { return base; }
};

template <>
struct base_provider<-1>
{
    typedef unsigned type;

    const type base;

    ESTD_CPP_CONSTEXPR_RET EXPLICIT base_provider(type base) : base{base} {}
    ESTD_CPP_CONSTEXPR_RET EXPLICIT base_provider(int base) : base{(type)base} {}

    ESTD_CPP_CONSTEXPR_RET unsigned operator()() const { return base; }
};


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
        CharIt last, Int value, base_provider<base_> base)
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
        *last = cbase_type::to_char(value % base());
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

// This one operates exactly according to spec, but slightly slower than above
template <unsigned b, class TInt>
to_chars_result to_chars_integer(char* first, char* last, TInt value, const int base)
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
