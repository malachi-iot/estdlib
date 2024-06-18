#pragma once

// DEBT: move this to file named chars_result

#include "../../system_error.h"
#include "../deduce_fixed_size.h"
#include "../text/encodings.h"

namespace estd {

namespace detail {

template <typename CharIt>
struct from_chars_result
{
    CharIt ptr;
    estd::errc ec;

    constexpr from_chars_result(CharIt ptr, estd::errc ec) :
        ptr(ptr), ec(ec) {}

    from_chars_result() = default;
};

template <class CharIt>
struct to_chars_result
{
    CharIt ptr;
    estd::errc ec;

    constexpr to_chars_result(CharIt ptr, estd::errc ec) :
        ptr(ptr), ec(ec) {}

    to_chars_result() = default;
};

}

/*
struct to_chars_result
{
    char* ptr;
    estd::errc ec;

#ifndef __cpp_initializer_lists
    to_chars_result(char* ptr, estd::errc ec) :
        ptr(ptr), ec(ec) {}
#endif
};
 */

typedef detail::to_chars_result<char*> to_chars_result;
typedef detail::from_chars_result<const char*> from_chars_result;


}
