#pragma once

#include "../system_error.h"
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

// due to FEATURE_CPP_ENUM_CLASS not available everywhere
struct encodings
{
    enum values
    {
        ASCII,
        ISO8859_1,
        UTF8,
        UTF16
    };
};

template <encodings::values encoding>
struct char_base_traits_base;

struct char_base_traits_char_base
{
    typedef int8_t int_type;
    typedef char char_type;

    inline static int_type eol() { return -1; }
};

struct char_base_traits_wchar_base
{
    typedef int16_t int_type;
    typedef wchar_t char_type;

    inline static int_type eol() { return -1; }
};


template <>
struct char_base_traits_base<encodings::ASCII> : char_base_traits_char_base {};

template <>
struct char_base_traits_base<encodings::UTF8> : char_base_traits_char_base {};

template <>
struct char_base_traits_base<encodings::UTF16> : char_base_traits_wchar_base {};

}

}
