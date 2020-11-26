/*
 * @file
 */
#pragma once

#include "system_error.h"
#include "cctype.h"
#include "type_traits.h"
#include "internal/charconv.hpp"

namespace estd {

namespace legacy {
from_chars_result from_chars(const char* first, const char* last,
                             long& value, int base = 10);
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
    if(base > 10)
        return internal::to_chars_integer<internal::char_base_traits<36> >(first, last, value, base);
    else
        return internal::to_chars_integer<internal::char_base_traits<10> >(first, last, value, base);
}

}