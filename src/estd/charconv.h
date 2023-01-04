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

// Temporarily used as we migrate away from char_traits_base
typedef cbase<char, 10, internal::classic_locale_type> cbase_C_10_type;
typedef cbase<char, 36, internal::classic_locale_type> cbase_C_36_type;

template<unsigned b, class T>
inline estd::from_chars_result from_chars_integer(const char* first, const char* last,
    T& value,
    const unsigned short base = b)
{
    return internal::from_chars_integer<
        cbase<char, b, internal::classic_locale_type> >(first, last, value, base);
}

}

template <class TInt>
inline estd::from_chars_result from_chars(const char* first,
                                                const char* last,
                                                TInt& value,
                                                const int base = 10)
{
    if(base > 10)
        return internal::from_chars_integer<36>(first, last, value, base);
    else
        return internal::from_chars_integer<10>(first, last, value, base);
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
#ifdef FEATURE_CPP_DEFAULT_TARGS
template <class TInt, class enabled = typename enable_if<numeric_limits<TInt>::is_integer>::type>
#else
template <class TInt>
#endif
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
