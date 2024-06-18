#pragma once

#include "../locale/fwd.h"

namespace estd { namespace internal {

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