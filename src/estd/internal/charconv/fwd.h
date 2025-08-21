#pragma once

#include "../type_traits.h"
#include "../../limits.h"
#include "features.h"

namespace estd {

namespace detail {

template <typename CharIt>
struct from_chars_result;

template <class CharIt>
struct to_chars_result;

// This is of the "opt" variety, where last character truly is at 'last'
template <unsigned b, class Int, class CharIt>
constexpr enable_if_t<estd::numeric_limits<Int>::is_integer, to_chars_result<CharIt> >
to_chars(CharIt first, CharIt last, Int value);

}

// DEBT: This only works because someone else pulled in result.h - fix that
using from_chars_result = detail::from_chars_result<const char*>;

template <class Int, bool sto_mode = false>
ESTD_CPP_CONSTEXPR(14) enable_if_t<numeric_limits<Int>::is_integer, from_chars_result>
    from_chars(const char* first,
        const char* last,
        Int& value,
        int base = 10);

}