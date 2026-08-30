#pragma once

// DEBT: move this to file named chars_result

#include "../errc.h"

#include "fwd.h"

namespace estd {

namespace detail {

template <typename CharIt>
struct from_chars_result
{
    CharIt ptr;
    estd::errc ec;

    constexpr from_chars_result(CharIt ptr, estd::errc ec) :
        ptr{ptr}, ec{ec} {}

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


}
