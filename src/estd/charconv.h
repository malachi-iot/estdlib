/*
 * @file
 */
#pragma once

#include "system_error.h"
#include "cctype.h"
#include "type_traits.h"
#include "internal/charconv.hpp"
#include "algorithm.h"

namespace estd {

namespace internal {

// DEBT: Put this somewhere else.  I got rid of it since estd::locale seemed like
// a happy home for it, but now I've changed my mind (estd::locale brings in a lot
// of dependencies)
typedef locale<locale_code::C, encodings::ASCII> classic_locale_type;

// Temporarily used as we migrate away from char_traits_base
typedef experimental::cbase<char, 10, internal::classic_locale_type> cbase_C_10_type;
typedef experimental::cbase<char, 16, internal::classic_locale_type> cbase_C_16_type;
typedef experimental::cbase<char, 36, internal::classic_locale_type> cbase_C_36_type;

}

template <class TInt>
estd::from_chars_result from_chars(const char* first,
                                                const char* last,
                                                TInt& value,
                                                const int base = 10)
{
    if(base > 10)
        return internal::from_chars_integer<internal::cbase_C_36_type>(first, last, value, base);
    else
        return internal::from_chars_integer<internal::cbase_C_10_type>(first, last, value, base);
}

// TODO: Needs bounds check on to_chars

/// Deviates from regular to_chars in that 'ptr' refers to start rather than one past end,
/// and 'last' is the static non-deviating end
/// \tparam TInt
/// \param first
/// \param last
/// \param value
/// \param base
/// \return
template <class TInt>
inline to_chars_result to_chars_opt(char* first, char* last, TInt value, const int base = 10)
{
    if(base > 10)
        return internal::to_chars_integer_opt<internal::cbase_C_36_type>(first, last, value, base);
    else
        return internal::to_chars_integer_opt<internal::cbase_C_10_type>(first, last, value, base);
}

template <class TInt>
to_chars_result to_chars(char* first, char* last, TInt value, const int base = 10)
{
    to_chars_result opt_result = to_chars_opt(first, last, value, base);

    opt_result.ptr = estd::copy(opt_result.ptr, last + 1, first);

    return opt_result;
}

template <class TInt>
to_chars_result to_chars_exp(char* first, char* last, TInt value, const int base = 10)
{
    if(base > 10)
        return internal::to_chars_integer<internal::cbase_C_36_type>(first, last, value, base);
    else
        return internal::to_chars_integer<internal::cbase_C_10_type>(first, last, value, base);
}


}
