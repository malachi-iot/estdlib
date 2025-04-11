#pragma once

#include "../../internal/platform.h"

#ifdef FEATURE_POSIX_IOS

#include <ostream>

#include "../../string.h"

namespace estd
{
// DEBT: Enable this for all compiler scenarios.  For some reason we did have
// to do some filtering.  I don't remember why

// Somehow clang has slightly different expectations during catch << resolution
#if defined(__clang__) || defined(__MINGW32__)
template <class Char, class Impl>
std::ostream& operator <<(
    std::ostream& os,
    const estd::detail::basic_string<Impl>& value)
{
    // DEBT: This only works for null terminated
    const char* s = value.clock();
    operator <<(os, s);
    value.cunlock();
    return os;
}
#endif

#if defined(FEATURE_STD_OSTREAM) || defined(FEATURE_ESTD_IOSTREAM_NATIVE) && !defined(__ADSPBLACKFIN__)

//A bit finicky so that we can remove const (via Traits::char_type)
template <class Char, class Traits, class Impl>
inline std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& os,
    const estd::detail::basic_string<Impl>& str)
{
    // TODO: Do query for null terminated vs non null terminated so that
    // this might be more efficient
    os.write(str.clock(), str.size());

    str.cunlock();

    return os;
}

/*
template <class CharT, class Traits, class Allocator>
inline std::ostream&
    operator<<(std::ostream& os,
               const estd::basic_string<const CharT, typename Traits::char_traits, Allocator, Traits>& str)
{
    // TODO: Do query for null terminated vs non null terminated so that
    // this might be more efficient
    os.write(str.clock(), str.size());

    str.cunlock();

    return os;
} */
#endif


}

#endif
