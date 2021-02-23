#pragma once

#include "deduce_fixed_size.h"

namespace estd {

struct from_chars_result
{
    const char* ptr;
    estd::errc ec;

#ifndef __cpp_initializer_lists
    from_chars_result(const char* ptr, estd::errc ec) :
        ptr(ptr), ec(ec) {}
#endif
};

struct to_chars_result
{
    char* ptr;
    estd::errc ec;

#ifndef __cpp_initializer_lists
    to_chars_result(char* ptr, estd::errc ec) :
        ptr(ptr), ec(ec) {}
#endif
};


namespace internal {
/// @brief Represents char-to-base-n conversion traits
/// @tparam b numeric base indicator
/// @tparam TEnable for internal use
template<unsigned short b, class TEnable = estd::internal::Range<true> >
struct char_base_traits;

struct char_base_traits_base
{
    typedef int8_t int_type;
    typedef char char_type;

    inline static int_type eol() { return -1; }
};

}

}
